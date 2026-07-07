#!/usr/bin/env python3
"""Locate effective Wide32 SRP bins for the N6 standard mode.

This script is intentionally separate from firmware defaults. It scores bins
with synthetic localization performance, high-SNR agreement, PHAT peak quality,
and long-baseline alias risk, then emits band-balanced B12/B16/B24 candidates.
"""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import random
from pathlib import Path

from acoustic_imaging_model import (
    AcousticScenario,
    MODE_WIDE32,
    PROFILE_BALANCED,
    AcousticSource,
    angular_error_deg,
    build_config,
    estimate_direction,
    frequency_pair_weight,
    load_mics,
    percentile,
    select_longest_pairs,
    select_mode_mics,
    with_bin_indices,
)
from evaluate_srp_profiles import build_wide32_scenarios


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_COORDS = ROOT / "docs" / "knowledge" / "microphone-array" / "array_32ch_coords.csv"


@dataclass(frozen=True)
class BinScore:
    bin_index: int
    frequency_hz: float
    p90_error_deg: float
    p90_high_snr_deg: float
    max_error_deg: float
    mean_quality: float
    alias_weight: float
    score: float


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--coords", type=Path, default=DEFAULT_COORDS)
    parser.add_argument("--seed", type=int, default=647)
    parser.add_argument("--random-trials", type=int, default=8)
    return parser.parse_args()


def local_rng(seed: int, bin_index: int, scenario_index: int) -> random.Random:
    return random.Random(seed + (bin_index * 4099) + (scenario_index * 1009))


def alias_weight_for_bin(pairs, frequency_hz: float) -> float:
    weights = [frequency_pair_weight(pair, frequency_hz, "wide32_general_srp") for pair in pairs]
    return sum(weights) / max(len(weights), 1)


def score_bin(bin_index: int, all_mics, scenarios: list[AcousticScenario], seed: int) -> BinScore:
    mics = select_mode_mics(all_mics, MODE_WIDE32)
    config = with_bin_indices(build_config(MODE_WIDE32, PROFILE_BALANCED), (bin_index,))
    pairs = select_longest_pairs(mics, config.pair_count)
    errors: list[float] = []
    high_snr_errors: list[float] = []
    qualities: list[float] = []
    frequency_hz = bin_index * config.sample_rate_hz / config.nfft

    for scenario_index, scenario in enumerate(scenarios):
        estimate = estimate_direction(
            config,
            mics,
            pairs,
            scenario,
            "wide32_general_srp",
            local_rng(seed, bin_index, scenario_index),
        )
        error = angular_error_deg(estimate, scenario.sources[0])
        errors.append(error)
        if scenario.snr_db >= 10.0:
            high_snr_errors.append(error)
        qualities.append(estimate.quality)

    alias = alias_weight_for_bin(pairs, frequency_hz)
    p90 = percentile(errors, 90.0)
    p90_high = percentile(high_snr_errors, 90.0)
    worst = max(errors) if errors else 0.0
    mean_quality = sum(qualities) / max(len(qualities), 1)

    # Lower is better. Alias weight protects coarse stability on long baselines;
    # quality rewards bins that form a clear SRP peak in the selected geometry.
    score = p90_high + (0.35 * p90) + (0.10 * worst) - (10.0 * mean_quality) - (3.0 * alias)
    return BinScore(
        bin_index=bin_index,
        frequency_hz=frequency_hz,
        p90_error_deg=p90,
        p90_high_snr_deg=p90_high,
        max_error_deg=worst,
        mean_quality=mean_quality,
        alias_weight=alias,
        score=score,
    )


def band_for_bin(bin_index: int) -> str:
    if bin_index <= 10:
        return "low"
    if bin_index <= 24:
        return "mid"
    return "high"


def select_band_balanced(scores: list[BinScore], quotas: dict[str, int]) -> tuple[int, ...]:
    selected: list[int] = []
    by_band: dict[str, list[BinScore]] = {"low": [], "mid": [], "high": []}

    for item in scores:
        by_band[band_for_bin(item.bin_index)].append(item)
    for band_scores in by_band.values():
        band_scores.sort(key=lambda item: item.score)

    for band in ("low", "mid", "high"):
        selected.extend(item.bin_index for item in by_band[band][: quotas[band]])

    return tuple(sorted(selected))


def print_scores(scores: list[BinScore]) -> None:
    print("=== Effective-bin single-bin scores ===")
    print("lower score is better; alias_weight is the average long-baseline frequency weight")
    for item in sorted(scores, key=lambda score: score.score)[:20]:
        print(
            f"bin={item.bin_index:>2} "
            f"freq={item.frequency_hz:>7.1f}Hz "
            f"score={item.score:>6.2f} "
            f"p90={item.p90_error_deg:>5.1f}deg "
            f"p90>=10dB={item.p90_high_snr_deg:>5.1f}deg "
            f"max={item.max_error_deg:>5.1f}deg "
            f"quality={item.mean_quality:.3f} "
            f"alias={item.alias_weight:.3f}"
        )


def print_selection(name: str, bins: tuple[int, ...]) -> None:
    freqs = tuple(round(bin_index * 48000.0 / 256.0, 1) for bin_index in bins)
    print(f"{name:<13} bins={bins}")
    print(f"{'':<13} freq_hz={freqs}")


def main() -> None:
    args = parse_args()
    all_mics = load_mics(args.coords)
    rng = random.Random(args.seed)
    scenarios = build_wide32_scenarios(32, args.random_trials, rng)

    # Add a few central/edge clean probes so the selector does not overfit only
    # to randomized noisy cases.
    scenarios.extend(
        [
            AcousticScenario("selector_center_20db", (AcousticSource(0.0, 0.0),), snr_db=20.0),
            AcousticScenario("selector_left_10db", (AcousticSource(-45.0, 0.0),), snr_db=10.0),
            AcousticScenario("selector_right_10db", (AcousticSource(45.0, 0.0),), snr_db=10.0),
        ]
    )

    print(f"scenarios: {len(scenarios)}")
    scores = [score_bin(bin_index, all_mics, scenarios, args.seed) for bin_index in range(3, 43)]
    print_scores(scores)
    print()
    print("=== Band-balanced selected bins ===")
    print_selection("STANDARD_B12", select_band_balanced(scores, {"low": 4, "mid": 4, "high": 4}))
    print_selection("STANDARD_B16", select_band_balanced(scores, {"low": 5, "mid": 6, "high": 5}))
    print_selection("STANDARD_B24", select_band_balanced(scores, {"low": 8, "mid": 8, "high": 8}))


if __name__ == "__main__":
    main()
