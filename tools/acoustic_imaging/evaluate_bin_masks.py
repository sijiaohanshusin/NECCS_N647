#!/usr/bin/env python3
"""Evaluate fixed Wide32 SRP bin masks for the N6 standard acoustic path."""

from __future__ import annotations

import argparse
from dataclasses import dataclass
import random
from pathlib import Path

from acoustic_imaging_model import (
    BIN_POLICY_QUALITY_B40,
    BIN_POLICY_STANDARD_B12,
    BIN_POLICY_STANDARD_B16,
    BIN_POLICY_STANDARD_B24,
    AcousticScenario,
    DirectionEstimate,
    MODE_WIDE32,
    PROFILE_BALANCED,
    PROFILE_FAST,
    PROFILE_QUALITY,
    angular_error_deg,
    bin_mask_for_policy,
    build_config,
    estimate_direction,
    load_mics,
    percentile,
    select_longest_pairs,
    select_mode_mics,
    with_bin_indices,
    with_bin_policy,
)
from evaluate_srp_profiles import build_wide32_scenarios


ROOT = Path(__file__).resolve().parents[2]
DEFAULT_COORDS = ROOT / "docs" / "knowledge" / "microphone-array" / "array_32ch_coords.csv"


@dataclass(frozen=True)
class Candidate:
    name: str
    profile: str
    bin_policy: str
    pair_count: int


DEFAULT_CANDIDATES = (
    Candidate("QUALITY_B40", PROFILE_QUALITY, BIN_POLICY_QUALITY_B40, 240),
    Candidate("STANDARD_B24", PROFILE_BALANCED, BIN_POLICY_STANDARD_B24, 160),
    Candidate("STANDARD_B16", PROFILE_BALANCED, BIN_POLICY_STANDARD_B16, 160),
    Candidate("STANDARD_B12", PROFILE_BALANCED, BIN_POLICY_STANDARD_B12, 160),
    Candidate("FAST_B12", PROFILE_FAST, BIN_POLICY_STANDARD_B12, 96),
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--coords", type=Path, default=DEFAULT_COORDS)
    parser.add_argument("--seed", type=int, default=647)
    parser.add_argument("--random-trials", type=int, default=24)
    parser.add_argument("--ablation", action="store_true", help="also run per-bin single-bin ablation")
    return parser.parse_args()


def estimate_work_units(pair_count: int, bin_count: int, fine_top_k: int, fine_grid_size: int) -> int:
    search_points = 81 + (fine_top_k * fine_grid_size * fine_grid_size)
    return pair_count * bin_count * search_points


def local_rng(seed: int, candidate_name: str, scenario_index: int) -> random.Random:
    return random.Random(seed + (scenario_index * 1009) + sum(ord(ch) for ch in candidate_name))


def evaluate_candidate(
    candidate: Candidate,
    all_mics,
    scenarios: list[AcousticScenario],
    seed: int,
) -> tuple[dict[str, float | int | str], list[DirectionEstimate], list[float]]:
    mics = select_mode_mics(all_mics, MODE_WIDE32)
    config = with_bin_policy(build_config(MODE_WIDE32, candidate.profile), candidate.bin_policy)
    config = with_bin_indices(config, bin_mask_for_policy(candidate.bin_policy))
    config = config.__class__(
        mode=config.mode,
        profile=config.profile,
        sample_rate_hz=config.sample_rate_hz,
        frame_len=config.frame_len,
        nfft=config.nfft,
        active_bin_start=config.active_bin_start,
        active_bin_end=config.active_bin_end,
        pair_count=candidate.pair_count,
        bin_indices=config.bin_indices,
        fine_top_k=config.fine_top_k,
        fine_grid_size=config.fine_grid_size,
        fine_span_deg=config.fine_span_deg,
    )
    pairs = select_longest_pairs(mics, candidate.pair_count)
    estimates: list[DirectionEstimate] = []
    errors: list[float] = []
    high_snr_errors: list[float] = []
    qualities: list[float] = []
    contrasts: list[float] = []

    for index, scenario in enumerate(scenarios):
        estimate = estimate_direction(
            config,
            mics,
            pairs,
            scenario,
            "wide32_general_srp",
            local_rng(seed, candidate.name, index),
        )
        error = angular_error_deg(estimate, scenario.sources[0])
        estimates.append(estimate)
        errors.append(error)
        if scenario.snr_db >= 10.0:
            high_snr_errors.append(error)
        qualities.append(estimate.quality)
        contrasts.append(estimate.contrast)

    row = {
        "name": candidate.name,
        "profile": candidate.profile,
        "pairs": candidate.pair_count,
        "bins": len(config.active_bins),
        "work_units": estimate_work_units(candidate.pair_count, len(config.active_bins), config.fine_top_k, config.fine_grid_size),
        "p50_error_deg": percentile(errors, 50.0),
        "p90_error_deg": percentile(errors, 90.0),
        "p90_high_snr_deg": percentile(high_snr_errors, 90.0),
        "max_error_deg": max(errors) if errors else 0.0,
        "mean_quality": sum(qualities) / max(len(qualities), 1),
        "mean_contrast": sum(contrasts) / max(len(contrasts), 1),
        "miss_count": sum(1 for error in errors if error > 18.0),
        "scenario_count": len(errors),
    }
    return row, estimates, errors


def angular_distance(a: DirectionEstimate, b: DirectionEstimate) -> float:
    return ((a.theta_deg - b.theta_deg) ** 2 + (a.phi_deg - b.phi_deg) ** 2) ** 0.5


def print_candidate_table(rows: list[dict[str, float | int | str]]) -> None:
    quality = next(row for row in rows if row["name"] == "QUALITY_B40")
    quality_p90 = float(quality["p90_error_deg"])
    quality_work = int(quality["work_units"])

    print("=== Wide32 fixed-bin candidate comparison ===")
    print("target: STANDARD p90@SNR>=10dB <=12deg, p90 degradation vs QUALITY <=1.5deg")
    print()
    for row in rows:
        work = int(row["work_units"])
        print(
            f"{row['name']:<13} "
            f"pairs={row['pairs']:>3} bins={row['bins']:>2} "
            f"work={work:>7} ({work / quality_work:>4.1%} of QUALITY) "
            f"p50={row['p50_error_deg']:>5.1f}deg "
            f"p90={row['p90_error_deg']:>5.1f}deg "
            f"p90>=10dB={row['p90_high_snr_deg']:>5.1f}deg "
            f"dP90={float(row['p90_error_deg']) - quality_p90:>+5.1f}deg "
            f"quality={row['mean_quality']:.3f} "
            f"miss={row['miss_count']}/{row['scenario_count']}"
        )


def print_agreement(rows: list[dict[str, float | int | str]], estimate_map: dict[str, list[DirectionEstimate]]) -> None:
    quality_estimates = estimate_map["QUALITY_B40"]
    print()
    print("=== Agreement with QUALITY_B40 peak ===")
    for row in rows:
        name = str(row["name"])
        distances = [
            angular_distance(candidate, quality)
            for candidate, quality in zip(estimate_map[name], quality_estimates)
        ]
        print(f"{name:<13} agree_p50={percentile(distances, 50.0):>5.1f}deg agree_p90={percentile(distances, 90.0):>5.1f}deg")


def run_ablation(all_mics, scenarios: list[AcousticScenario], seed: int) -> None:
    print()
    print("=== Single-bin ablation, Balanced/160 pairs ===")
    rows: list[tuple[int, float, float, float]] = []
    for bin_index in range(3, 43):
        candidate = Candidate(f"BIN_{bin_index:02d}", PROFILE_BALANCED, BIN_POLICY_QUALITY_B40, 160)
        config = with_bin_indices(build_config(MODE_WIDE32, PROFILE_BALANCED), (bin_index,))
        mics = select_mode_mics(all_mics, MODE_WIDE32)
        pairs = select_longest_pairs(mics, candidate.pair_count)
        errors: list[float] = []
        qualities: list[float] = []
        for index, scenario in enumerate(scenarios):
            estimate = estimate_direction(
                config,
                mics,
                pairs,
                scenario,
                "wide32_general_srp",
                local_rng(seed, candidate.name, index),
            )
            errors.append(angular_error_deg(estimate, scenario.sources[0]))
            qualities.append(estimate.quality)
        rows.append((bin_index, percentile(errors, 90.0), max(errors), sum(qualities) / max(len(qualities), 1)))

    for bin_index, p90, worst, quality in sorted(rows, key=lambda item: (item[1], -item[3]))[:12]:
        freq_hz = bin_index * 48000.0 / 256.0
        print(f"bin={bin_index:>2} freq={freq_hz:>7.1f}Hz p90={p90:>5.1f}deg max={worst:>5.1f}deg quality={quality:.3f}")


def main() -> None:
    args = parse_args()
    all_mics = load_mics(args.coords)
    rng = random.Random(args.seed)
    scenarios = build_wide32_scenarios(32, args.random_trials, rng)
    rows: list[dict[str, float | int | str]] = []
    estimate_map: dict[str, list[DirectionEstimate]] = {}

    for candidate in DEFAULT_CANDIDATES:
        row, estimates, _errors = evaluate_candidate(candidate, all_mics, scenarios, args.seed)
        rows.append(row)
        estimate_map[candidate.name] = estimates

    print(f"scenarios: {len(scenarios)}")
    print_candidate_table(rows)
    print_agreement(rows, estimate_map)

    if args.ablation:
        run_ablation(all_mics, scenarios, args.seed)


if __name__ == "__main__":
    main()
