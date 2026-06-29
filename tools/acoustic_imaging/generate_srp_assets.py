#!/usr/bin/env python3
"""Generate pair-set and TDOA assets from the 32-channel microphone CSV."""

from __future__ import annotations

import argparse
import csv
from pathlib import Path

from acoustic_imaging_model import (
    MODE_CORE16,
    MODE_WIDE32,
    PROFILE_BALANCED,
    PROFILE_FAST,
    PROFILE_OFFLINE_ALL,
    PROFILE_QUALITY,
    active_frequencies_hz,
    build_config,
    build_tdoa_lut,
    coarse_grid,
    load_mics,
    select_longest_pairs,
    select_mode_mics,
    summarize_pair_set,
)

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_COORDS = ROOT / "docs" / "knowledge" / "microphone-array" / "array_32ch_coords.csv"
DEFAULT_OUTPUT_DIR = ROOT / "docs" / "knowledge" / "microphone-array" / "generated"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--coords", type=Path, default=DEFAULT_COORDS)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument("--mode", choices=[MODE_WIDE32, MODE_CORE16], default=MODE_WIDE32)
    parser.add_argument(
        "--profile",
        choices=[PROFILE_FAST, PROFILE_BALANCED, PROFILE_QUALITY, PROFILE_OFFLINE_ALL],
        default=PROFILE_BALANCED,
    )
    parser.add_argument("--no-lut", action="store_true", help="write only pairset and summary")
    return parser.parse_args()


def write_pairset(path: Path, mics, pairs) -> None:
    with path.open("w", encoding="utf-8", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(["pair_index", "mic_a", "mic_b", "dx_m", "dy_m", "baseline_mm"])
        for idx, pair in enumerate(pairs):
            writer.writerow(
                [
                    idx,
                    mics[pair.mic_a].mic_id,
                    mics[pair.mic_b].mic_id,
                    f"{pair.dx_m:.9f}",
                    f"{pair.dy_m:.9f}",
                    f"{pair.baseline_m * 1000.0:.4f}",
                ]
            )


def write_tdoa_lut(path: Path, pairs) -> None:
    lut = build_tdoa_lut(pairs)
    with path.open("w", encoding="utf-8", newline="") as f:
        writer = csv.writer(f)
        writer.writerow(
            ["grid_index", "theta_deg", "phi_deg"]
            + [f"pair_{idx:03d}_tdoa_s" for idx in range(len(pairs))]
        )
        for grid_index, ((theta, phi), row) in enumerate(zip(coarse_grid(), lut)):
            writer.writerow(
                [grid_index, f"{theta:.1f}", f"{phi:.1f}"]
                + [f"{value:.12e}" for value in row]
            )


def write_summary(path: Path, config, mics, pairs, wrote_lut: bool) -> None:
    summary = summarize_pair_set(mics, pairs)
    freqs = active_frequencies_hz(config)

    lines = [
        f"# Acoustic Imaging Assets: {config.mode} / {config.profile}",
        "",
        "Generated from `array_32ch_coords.csv`.",
        "",
        "## Runtime Profile",
        "",
        f"- Sample rate: {config.sample_rate_hz} Hz",
        f"- Frame length / NFFT: {config.frame_len} / {config.nfft}",
        f"- Active bins: {config.active_bin_start}..{config.active_bin_end}",
        f"- Active frequency span: {freqs[0]:.1f}..{freqs[-1]:.1f} Hz",
        f"- Pair count: {config.pair_count}",
        "- Coarse grid: 9 x 9, -60..60 deg",
        "- Fine search contract: top3 x 4 x 4, span 10 deg",
        "",
        "## Pair-Set Summary",
        "",
        f"- Microphones: {summary['mic_count']}",
        f"- Coverage min/max: {summary['coverage_min']} / {summary['coverage_max']}",
        f"- Missing channels: {summary['missing_channels']}",
        f"- Baseline min/max/mean: {summary['baseline_min_mm']:.2f} / "
        f"{summary['baseline_max_mm']:.2f} / {summary['baseline_mean_mm']:.2f} mm",
        "",
        "## Files",
        "",
        "- Pair set CSV: generated next to this summary",
        f"- TDOA LUT CSV: {'generated' if wrote_lut else 'skipped by --no-lut'}",
        "",
        "PCMD/SAI/DMA configuration is intentionally not represented here.",
    ]
    path.write_text("\n".join(lines) + "\n", encoding="utf-8")


def main() -> None:
    args = parse_args()
    all_mics = load_mics(args.coords)
    config = build_config(args.mode, args.profile)
    mics = select_mode_mics(all_mics, args.mode)
    pairs = select_longest_pairs(mics, config.pair_count)

    args.output_dir.mkdir(parents=True, exist_ok=True)
    stem = f"{config.mode}_{config.profile}"
    pairset_path = args.output_dir / f"{stem}_pairset.csv"
    lut_path = args.output_dir / f"{stem}_tdoa_lut.csv"
    summary_path = args.output_dir / f"{stem}_summary.md"

    write_pairset(pairset_path, mics, pairs)
    if not args.no_lut:
        write_tdoa_lut(lut_path, pairs)
    write_summary(summary_path, config, mics, pairs, wrote_lut=not args.no_lut)

    summary = summarize_pair_set(mics, pairs)
    print(f"mode/profile: {config.mode}/{config.profile}")
    print(f"pair_count: {summary['pair_count']}")
    print(f"coverage_min/max: {summary['coverage_min']}/{summary['coverage_max']}")
    print(f"baseline_mm min/max/mean: {summary['baseline_min_mm']:.2f}/"
          f"{summary['baseline_max_mm']:.2f}/{summary['baseline_mean_mm']:.2f}")
    print(f"wrote: {pairset_path}")
    if not args.no_lut:
        print(f"wrote: {lut_path}")
    print(f"wrote: {summary_path}")


if __name__ == "__main__":
    main()
