#!/usr/bin/env python3
"""Offline SRP-PHAT route sanity checks for selected NECCS profiles."""

from __future__ import annotations

import argparse
import math
from pathlib import Path
import random

from acoustic_imaging_model import (
    MODE_CORE16,
    MODE_WIDE32,
    PROFILE_BALANCED,
    PROFILE_FAST,
    PROFILE_QUALITY,
    active_frequencies_hz,
    build_config,
    coarse_grid,
    load_mics,
    select_longest_pairs,
    select_mode_mics,
    summarize_pair_set,
    tdoa_seconds,
)

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_COORDS = ROOT / "docs" / "knowledge" / "microphone-array" / "array_32ch_coords.csv"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--coords", type=Path, default=DEFAULT_COORDS)
    parser.add_argument("--mode", choices=[MODE_WIDE32, MODE_CORE16], default=MODE_WIDE32)
    parser.add_argument(
        "--profile",
        choices=[PROFILE_FAST, PROFILE_BALANCED, PROFILE_QUALITY],
        default=PROFILE_BALANCED,
    )
    parser.add_argument("--seed", type=int, default=647)
    return parser.parse_args()


def phase_noise_sigma(snr_db: float) -> float:
    linear = 10.0 ** (snr_db / 20.0)
    return 0.55 / max(linear, 0.25)


def score_direction(
    pairs,
    freqs_hz: list[float],
    true_theta: float,
    true_phi: float,
    test_theta: float,
    test_phi: float,
    rng: random.Random,
    snr_db: float,
) -> float:
    sigma = phase_noise_sigma(snr_db)
    total = 0.0
    count = 0

    for pair in pairs:
        tau_true = tdoa_seconds(pair, true_theta, true_phi)
        tau_test = tdoa_seconds(pair, test_theta, test_phi)
        tau_delta = tau_true - tau_test
        for freq in freqs_hz:
            phase = 2.0 * math.pi * freq * tau_delta
            phase += rng.gauss(0.0, sigma)
            total += math.cos(phase)
            count += 1

    return total / max(count, 1)


def run_grid_check(config, pairs, rng: random.Random) -> list[tuple[float, float, float, float]]:
    sources = [
        (-45.0, -30.0),
        (-45.0, 0.0),
        (-45.0, 30.0),
        (0.0, -30.0),
        (0.0, 0.0),
        (0.0, 30.0),
        (45.0, -30.0),
        (45.0, 0.0),
        (45.0, 30.0),
    ]
    freqs = active_frequencies_hz(config)
    grid = coarse_grid()
    rows: list[tuple[float, float, float, float]] = []

    for snr_db in (0.0, 10.0, 20.0):
        errors = []
        for true_theta, true_phi in sources:
            best_theta = 0.0
            best_phi = 0.0
            best_score = -1.0e30

            for test_theta, test_phi in grid:
                score = score_direction(
                    pairs,
                    freqs,
                    true_theta,
                    true_phi,
                    test_theta,
                    test_phi,
                    rng,
                    snr_db,
                )
                if score > best_score:
                    best_score = score
                    best_theta = test_theta
                    best_phi = test_phi

            err = math.hypot(best_theta - true_theta, best_phi - true_phi)
            errors.append(err)

        mean_err = sum(errors) / len(errors)
        max_err = max(errors)
        rows.append((snr_db, mean_err, max_err, len(errors)))

    return rows


def main() -> None:
    args = parse_args()
    rng = random.Random(args.seed)
    all_mics = load_mics(args.coords)
    config = build_config(args.mode, args.profile)
    mics = select_mode_mics(all_mics, args.mode)
    pairs = select_longest_pairs(mics, config.pair_count)
    summary = summarize_pair_set(mics, pairs)

    print("=== Pair Coverage ===")
    print(f"mode/profile: {config.mode}/{config.profile}")
    print(f"mic_count: {summary['mic_count']}")
    print(f"pair_count: {summary['pair_count']}")
    print(f"coverage_min/max: {summary['coverage_min']}/{summary['coverage_max']}")
    print(f"missing_channels: {summary['missing_channels']}")
    print(
        "baseline_mm min/max/mean: "
        f"{summary['baseline_min_mm']:.2f}/"
        f"{summary['baseline_max_mm']:.2f}/"
        f"{summary['baseline_mean_mm']:.2f}"
    )

    print("\n=== Idealized Coarse SRP Check ===")
    for snr_db, mean_err, max_err, count in run_grid_check(config, pairs, rng):
        print(
            f"SNR {snr_db:>4.0f} dB: "
            f"sources={int(count)}, mean_error={mean_err:.2f} deg, max_error={max_err:.2f} deg"
        )

    print("\nThis is a coarse-grid synthetic check only; hardware phase/order validation still gates real capture.")


if __name__ == "__main__":
    main()
