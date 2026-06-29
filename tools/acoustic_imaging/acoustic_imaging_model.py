#!/usr/bin/env python3
"""Shared offline geometry model for the NECCS acoustic imaging route."""

from __future__ import annotations

from dataclasses import dataclass
import csv
import math
from pathlib import Path
from typing import Iterable

SPEED_OF_SOUND_MPS = 343.0
COARSE_ANGLES_DEG = [-60.0, -45.0, -30.0, -15.0, 0.0, 15.0, 30.0, 45.0, 60.0]

MODE_WIDE32 = "wide32"
MODE_CORE16 = "core16"

PROFILE_FAST = "fast"
PROFILE_BALANCED = "balanced"
PROFILE_QUALITY = "quality"
PROFILE_OFFLINE_ALL = "offline_all"


@dataclass(frozen=True)
class Mic:
    mic_id: str
    x_m: float
    y_m: float
    bus: str
    chip: str
    chip_slot: int
    tdm_slot_48k: int
    tdm_slot_192k_core: int | None
    is_core16: bool


@dataclass(frozen=True)
class ImagingConfig:
    mode: str
    profile: str
    sample_rate_hz: int
    frame_len: int
    nfft: int
    active_bin_start: int
    active_bin_end: int
    pair_count: int
    fine_top_k: int = 3
    fine_grid_size: int = 4
    fine_span_deg: float = 10.0

    @property
    def active_bins(self) -> range:
        return range(self.active_bin_start, self.active_bin_end + 1)


@dataclass(frozen=True)
class Pair:
    mic_a: int
    mic_b: int
    dx_m: float
    dy_m: float
    baseline_m: float


def load_mics(csv_path: Path) -> list[Mic]:
    with csv_path.open("r", encoding="utf-8", newline="") as f:
        rows = list(csv.DictReader(f))

    mics: list[Mic] = []
    for row in rows:
        slot_192k = row["tdm_slot_192k_core"].strip()
        mics.append(
            Mic(
                mic_id=row["mic_id"].strip(),
                x_m=float(row["x_mm"]) / 1000.0,
                y_m=float(row["y_mm"]) / 1000.0,
                bus=row["bus"].strip(),
                chip=row["chip"].strip(),
                chip_slot=int(row["chip_slot"]),
                tdm_slot_48k=int(row["tdm_slot_48k"]),
                tdm_slot_192k_core=int(slot_192k) if slot_192k else None,
                is_core16=row["is_core16"].strip() == "1",
            )
        )

    if len(mics) != 32:
        raise ValueError(f"expected 32 microphones, got {len(mics)}")
    return mics


def select_mode_mics(all_mics: list[Mic], mode: str) -> list[Mic]:
    if mode == MODE_WIDE32:
        return list(all_mics)
    if mode == MODE_CORE16:
        core = [mic for mic in all_mics if mic.is_core16]
        if len(core) != 16:
            raise ValueError(f"expected 16 core microphones, got {len(core)}")
        return core
    raise ValueError(f"unsupported mode: {mode}")


def build_config(mode: str, profile: str) -> ImagingConfig:
    if mode == MODE_WIDE32:
        pairs_by_profile = {
            PROFILE_FAST: 96,
            PROFILE_BALANCED: 160,
            PROFILE_QUALITY: 240,
            PROFILE_OFFLINE_ALL: 496,
        }
        return ImagingConfig(
            mode=mode,
            profile=profile,
            sample_rate_hz=48000,
            frame_len=256,
            nfft=256,
            active_bin_start=3,
            active_bin_end=42,
            pair_count=pairs_by_profile[profile],
        )

    if mode == MODE_CORE16:
        pairs_by_profile = {
            PROFILE_FAST: 80,
            PROFILE_BALANCED: 120,
            PROFILE_QUALITY: 120,
            PROFILE_OFFLINE_ALL: 120,
        }
        return ImagingConfig(
            mode=mode,
            profile=profile,
            sample_rate_hz=192000,
            frame_len=512,
            nfft=512,
            active_bin_start=11,
            active_bin_end=107,
            pair_count=pairs_by_profile[profile],
        )

    raise ValueError(f"unsupported mode/profile: {mode}/{profile}")


def all_pairs(mics: list[Mic]) -> list[Pair]:
    pairs: list[Pair] = []
    for a in range(len(mics)):
        for b in range(a + 1, len(mics)):
            dx = mics[a].x_m - mics[b].x_m
            dy = mics[a].y_m - mics[b].y_m
            pairs.append(Pair(a, b, dx, dy, math.hypot(dx, dy)))
    return pairs


def _coverage(pairs: Iterable[Pair], channel_count: int) -> list[int]:
    counts = [0] * channel_count
    for pair in pairs:
        counts[pair.mic_a] += 1
        counts[pair.mic_b] += 1
    return counts


def select_longest_pairs(mics: list[Mic], pair_count: int) -> list[Pair]:
    candidates = sorted(all_pairs(mics), key=lambda pair: pair.baseline_m, reverse=True)
    if pair_count < 1 or pair_count > len(candidates):
        raise ValueError(f"pair_count must be in [1, {len(candidates)}]")

    selected = candidates[:pair_count]
    selected_keys = {(pair.mic_a, pair.mic_b) for pair in selected}
    coverage = _coverage(selected, len(mics))

    for missing in [idx for idx, count in enumerate(coverage) if count == 0]:
        best = next(
            (
                pair
                for pair in candidates
                if missing in (pair.mic_a, pair.mic_b)
                and (pair.mic_a, pair.mic_b) not in selected_keys
            ),
            None,
        )
        if best is None:
            raise RuntimeError(f"cannot repair coverage for channel {missing}")

        replace_index = None
        for idx in range(len(selected) - 1, -1, -1):
            pair = selected[idx]
            if coverage[pair.mic_a] > 1 and coverage[pair.mic_b] > 1:
                replace_index = idx
                break
        if replace_index is None:
            raise RuntimeError(f"cannot find replaceable pair for channel {missing}")

        old = selected[replace_index]
        selected_keys.remove((old.mic_a, old.mic_b))
        coverage[old.mic_a] -= 1
        coverage[old.mic_b] -= 1
        selected[replace_index] = best
        selected_keys.add((best.mic_a, best.mic_b))
        coverage[best.mic_a] += 1
        coverage[best.mic_b] += 1

    return sorted(selected, key=lambda pair: pair.baseline_m, reverse=True)


def coarse_grid() -> list[tuple[float, float]]:
    return [(theta, phi) for theta in COARSE_ANGLES_DEG for phi in COARSE_ANGLES_DEG]


def tdoa_seconds(pair: Pair, theta_deg: float, phi_deg: float) -> float:
    theta = math.radians(theta_deg)
    phi = math.radians(phi_deg)
    return (
        pair.dx_m * math.sin(theta) * math.cos(phi)
        + pair.dy_m * math.sin(phi)
    ) / SPEED_OF_SOUND_MPS


def build_tdoa_lut(pairs: list[Pair]) -> list[list[float]]:
    return [[tdoa_seconds(pair, theta, phi) for pair in pairs] for theta, phi in coarse_grid()]


def active_frequencies_hz(config: ImagingConfig) -> list[float]:
    delta_f = config.sample_rate_hz / config.nfft
    return [bin_index * delta_f for bin_index in config.active_bins]


def summarize_pair_set(mics: list[Mic], pairs: list[Pair]) -> dict[str, float | int | list[int]]:
    coverage = _coverage(pairs, len(mics))
    baselines = [pair.baseline_m for pair in pairs]
    return {
        "mic_count": len(mics),
        "pair_count": len(pairs),
        "coverage_min": min(coverage),
        "coverage_max": max(coverage),
        "missing_channels": [idx for idx, count in enumerate(coverage) if count == 0],
        "baseline_min_mm": min(baselines) * 1000.0,
        "baseline_max_mm": max(baselines) * 1000.0,
        "baseline_mean_mm": sum(baselines) * 1000.0 / len(baselines),
    }
