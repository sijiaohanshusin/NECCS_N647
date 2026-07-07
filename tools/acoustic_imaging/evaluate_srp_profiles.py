#!/usr/bin/env python3
"""Evaluate NECCS N6 acoustic-imaging SRP profiles against synthetic scenarios."""

from __future__ import annotations

import argparse
import random
from pathlib import Path

from acoustic_imaging_model import (
    ALGORITHM_CORE16_HF_NEARFIELD,
    ALGORITHM_WIDE32_FAST_SRP,
    ALGORITHM_WIDE32_GENERAL_SRP,
    ALGORITHM_WIDE32_HF_HINT,
    ALGORITHM_WIDE32_QUALITY_SRP,
    AcousticScenario,
    AcousticSource,
    angular_error_deg,
    build_algorithm_config,
    estimate_direction,
    load_mics,
    percentile,
    select_algorithm_pairs,
    select_mode_mics,
    summarize_pair_set,
)

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_COORDS = ROOT / "docs" / "knowledge" / "microphone-array" / "array_32ch_coords.csv"

DEFAULT_WIDE32_ALGORITHMS = (
    ALGORITHM_WIDE32_FAST_SRP,
    ALGORITHM_WIDE32_GENERAL_SRP,
    ALGORITHM_WIDE32_QUALITY_SRP,
    ALGORITHM_WIDE32_HF_HINT,
)


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--coords", type=Path, default=DEFAULT_COORDS)
    parser.add_argument("--seed", type=int, default=647)
    parser.add_argument("--random-trials", type=int, default=12)
    parser.add_argument("--include-core16", action="store_true")
    parser.add_argument("--strict", action="store_true", help="exit non-zero when Wide32-General misses the v1 target")
    return parser.parse_args()


def _gain_errors(count: int, rng: random.Random, span_db: float) -> tuple[float, ...]:
    return tuple(rng.uniform(-span_db, span_db) for _ in range(count))


def _delay_errors(count: int, rng: random.Random, span_us: float) -> tuple[float, ...]:
    scale = span_us * 1.0e-6
    return tuple(rng.uniform(-scale, scale) for _ in range(count))


def _slot_swap(count: int, a: int, b: int) -> tuple[int, ...]:
    mapping = list(range(count))
    mapping[a], mapping[b] = mapping[b], mapping[a]
    return tuple(mapping)


def build_wide32_scenarios(channel_count: int, random_trials: int, rng: random.Random) -> list[AcousticScenario]:
    base_sources = [
        (-45.0, -30.0),
        (-30.0, 15.0),
        (0.0, 0.0),
        (30.0, -15.0),
        (45.0, 30.0),
    ]
    scenarios: list[AcousticScenario] = []

    for snr_db in (0.0, 5.0, 10.0, 20.0):
        for theta, phi in base_sources:
            scenarios.append(
                AcousticScenario(
                    name=f"single_{theta:+.0f}_{phi:+.0f}_{snr_db:.0f}db",
                    sources=(AcousticSource(theta, phi),),
                    snr_db=snr_db,
                )
            )

    for idx in range(random_trials):
        theta = rng.uniform(-55.0, 55.0)
        phi = rng.uniform(-50.0, 50.0)
        snr_db = rng.choice((0.0, 5.0, 10.0, 20.0))
        bad_count = rng.choice((0, 1, 2, 4))
        bad_channels = tuple(sorted(rng.sample(range(channel_count), bad_count)))
        scenarios.append(
            AcousticScenario(
                name=f"random_{idx:02d}_{snr_db:.0f}db_bad{bad_count}",
                sources=(AcousticSource(theta, phi),),
                snr_db=snr_db,
                bad_channels=bad_channels,
                gain_error_db=_gain_errors(channel_count, rng, 2.0),
                delay_error_s=_delay_errors(channel_count, rng, 2.0),
                reflection_gain=0.22 if idx % 3 == 0 else 0.0,
            )
        )

    scenarios.extend(
        [
            AcousticScenario(
                name="two_sources_10db",
                sources=(AcousticSource(-30.0, 0.0, 1.0), AcousticSource(25.0, 25.0, 0.55)),
                snr_db=10.0,
                reflection_gain=0.15,
            ),
            AcousticScenario(
                name="nearfield_0p7m_10db",
                sources=(AcousticSource(20.0, -20.0, 1.0, distance_m=0.7),),
                snr_db=10.0,
                reflection_gain=0.18,
            ),
            AcousticScenario(
                name="polarity_flip_12_10db",
                sources=(AcousticSource(15.0, 30.0),),
                snr_db=10.0,
                polarity_flips=(12,),
            ),
            AcousticScenario(
                name="slot_swap_0_1_20db",
                sources=(AcousticSource(-15.0, -30.0),),
                snr_db=20.0,
                slot_permutation=_slot_swap(channel_count, 0, 1),
            ),
        ]
    )

    return scenarios


def estimate_work_units(config, pair_count: int) -> int:
    active_bins = len(config.active_bins)
    search_points = 81 + (config.fine_top_k * config.fine_grid_size * config.fine_grid_size)
    return pair_count * active_bins * search_points


def evaluate_algorithm(algorithm: str, all_mics, scenarios: list[AcousticScenario], seed: int) -> dict[str, float | int | str]:
    config = build_algorithm_config(algorithm)
    mics = select_mode_mics(all_mics, config.mode)
    pairs = select_algorithm_pairs(mics, config, algorithm)
    summary = summarize_pair_set(mics, pairs)
    rng = random.Random(seed + sum(ord(ch) for ch in algorithm))
    errors: list[float] = []
    qualities: list[float] = []
    contrasts: list[float] = []
    active_counts: list[int] = []

    for scenario in scenarios:
        local_scenario = scenario
        if len(mics) < 32:
            local_scenario = AcousticScenario(
                name=scenario.name,
                sources=scenario.sources,
                snr_db=scenario.snr_db,
                bad_channels=tuple(ch for ch in scenario.bad_channels if ch < len(mics)),
                polarity_flips=tuple(ch for ch in scenario.polarity_flips if ch < len(mics)),
                gain_error_db=scenario.gain_error_db[: len(mics)],
                delay_error_s=scenario.delay_error_s[: len(mics)],
                reflection_gain=scenario.reflection_gain,
                reflection_delay_s=scenario.reflection_delay_s,
            )

        estimate = estimate_direction(config, mics, pairs, local_scenario, algorithm, rng)
        errors.append(angular_error_deg(estimate, local_scenario.sources[0]))
        qualities.append(estimate.quality)
        contrasts.append(estimate.contrast)
        active_counts.append(estimate.active_feature_count)

    return {
        "algorithm": algorithm,
        "mode": config.mode,
        "profile": config.profile,
        "pair_count": len(pairs),
        "coverage_min": int(summary["coverage_min"]),
        "active_bins": len(config.active_bins),
        "work_units": estimate_work_units(config, len(pairs)),
        "p50_error_deg": percentile(errors, 50.0),
        "p90_error_deg": percentile(errors, 90.0),
        "max_error_deg": max(errors) if errors else 0.0,
        "mean_quality": sum(qualities) / max(len(qualities), 1),
        "mean_contrast": sum(contrasts) / max(len(contrasts), 1),
        "min_features": min(active_counts) if active_counts else 0,
        "miss_count": sum(1 for err in errors if err > 18.0),
        "scenario_count": len(errors),
    }


def print_result(row: dict[str, float | int | str]) -> None:
    print(
        f"{row['algorithm']:<24} "
        f"pairs={row['pair_count']:>3} bins={row['active_bins']:>2} "
        f"work={row['work_units']:>7} "
        f"p50={row['p50_error_deg']:>5.1f}deg "
        f"p90={row['p90_error_deg']:>5.1f}deg "
        f"max={row['max_error_deg']:>5.1f}deg "
        f"quality={row['mean_quality']:.3f} "
        f"miss={row['miss_count']}/{row['scenario_count']}"
    )


def main() -> None:
    args = parse_args()
    all_mics = load_mics(args.coords)
    rng = random.Random(args.seed)
    scenarios = build_wide32_scenarios(32, args.random_trials, rng)
    algorithms = list(DEFAULT_WIDE32_ALGORITHMS)
    if args.include_core16:
        algorithms.append(ALGORITHM_CORE16_HF_NEARFIELD)

    print("=== NECCS N6 SRP Profile Evaluation ===")
    print(f"scenarios: {len(scenarios)}")
    print("target: Wide32-General p50<=5deg, p90<=12deg for SNR>=10dB subsets; total matrix p90 is reported wider.")
    print()

    rows = [evaluate_algorithm(algorithm, all_mics, scenarios, args.seed) for algorithm in algorithms]
    for row in rows:
        print_result(row)

    general = next(row for row in rows if row["algorithm"] == ALGORITHM_WIDE32_GENERAL_SRP)
    fast = next(row for row in rows if row["algorithm"] == ALGORITHM_WIDE32_FAST_SRP)
    quality = next(row for row in rows if row["algorithm"] == ALGORITHM_WIDE32_QUALITY_SRP)

    recommendation = ALGORITHM_WIDE32_GENERAL_SRP
    if quality["p90_error_deg"] + 2.0 < general["p90_error_deg"] and quality["work_units"] < 2 * general["work_units"]:
        recommendation = ALGORITHM_WIDE32_QUALITY_SRP
    if general["p90_error_deg"] > 18.0 and fast["p90_error_deg"] <= general["p90_error_deg"] + 3.0:
        recommendation = ALGORITHM_WIDE32_FAST_SRP

    print()
    print(f"recommended_first_firmware_profile: {recommendation}")

    if args.strict and (general["p50_error_deg"] > 5.0 or general["p90_error_deg"] > 18.0):
        raise SystemExit(1)


if __name__ == "__main__":
    main()
