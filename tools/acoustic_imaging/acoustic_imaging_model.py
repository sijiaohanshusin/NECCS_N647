#!/usr/bin/env python3
"""Shared offline geometry model for the NECCS acoustic imaging route."""

from __future__ import annotations

from dataclasses import dataclass
import csv
import math
from pathlib import Path
import random
from typing import Iterable

SPEED_OF_SOUND_MPS = 343.0
COARSE_ANGLES_DEG = [-60.0, -45.0, -30.0, -15.0, 0.0, 15.0, 30.0, 45.0, 60.0]

MODE_WIDE32 = "wide32"
MODE_CORE16 = "core16"

PROFILE_FAST = "fast"
PROFILE_BALANCED = "balanced"
PROFILE_QUALITY = "quality"
PROFILE_OFFLINE_ALL = "offline_all"

ALGORITHM_WIDE32_FAST_SRP = "wide32_fast_srp"
ALGORITHM_WIDE32_GENERAL_SRP = "wide32_general_srp"
ALGORITHM_WIDE32_QUALITY_SRP = "wide32_quality_srp"
ALGORITHM_WIDE32_HF_HINT = "wide32_hf_hint"
ALGORITHM_CORE16_HF_NEARFIELD = "core16_hf_nearfield"


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


@dataclass(frozen=True)
class AcousticSource:
    theta_deg: float
    phi_deg: float
    amplitude: float = 1.0
    distance_m: float | None = None
    extra_delay_s: float = 0.0


@dataclass(frozen=True)
class AcousticScenario:
    name: str
    sources: tuple[AcousticSource, ...]
    snr_db: float = 20.0
    bad_channels: tuple[int, ...] = ()
    polarity_flips: tuple[int, ...] = ()
    slot_permutation: tuple[int, ...] | None = None
    gain_error_db: tuple[float, ...] = ()
    delay_error_s: tuple[float, ...] = ()
    reflection_gain: float = 0.0
    reflection_delay_s: float = 0.0016


@dataclass(frozen=True)
class DirectionEstimate:
    theta_deg: float
    phi_deg: float
    score: float
    quality: float
    contrast: float
    active_feature_count: int


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


def build_algorithm_config(algorithm: str) -> ImagingConfig:
    if algorithm == ALGORITHM_WIDE32_FAST_SRP:
        return build_config(MODE_WIDE32, PROFILE_FAST)
    if algorithm == ALGORITHM_WIDE32_GENERAL_SRP:
        return build_config(MODE_WIDE32, PROFILE_BALANCED)
    if algorithm == ALGORITHM_WIDE32_QUALITY_SRP:
        return build_config(MODE_WIDE32, PROFILE_QUALITY)
    if algorithm == ALGORITHM_WIDE32_HF_HINT:
        return ImagingConfig(
            mode=MODE_WIDE32,
            profile=PROFILE_FAST,
            sample_rate_hz=48000,
            frame_len=256,
            nfft=256,
            active_bin_start=43,
            active_bin_end=80,
            pair_count=96,
        )
    if algorithm == ALGORITHM_CORE16_HF_NEARFIELD:
        return build_config(MODE_CORE16, PROFILE_BALANCED)
    raise ValueError(f"unsupported algorithm: {algorithm}")


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


def select_short_baseline_pairs(mics: list[Mic], pair_count: int) -> list[Pair]:
    candidates = sorted(all_pairs(mics), key=lambda pair: pair.baseline_m)
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

    return sorted(selected, key=lambda pair: pair.baseline_m)


def select_algorithm_pairs(mics: list[Mic], config: ImagingConfig, algorithm: str) -> list[Pair]:
    if algorithm == ALGORITHM_WIDE32_HF_HINT:
        return select_short_baseline_pairs(mics, config.pair_count)
    return select_longest_pairs(mics, config.pair_count)


def coarse_grid() -> list[tuple[float, float]]:
    return [(theta, phi) for theta in COARSE_ANGLES_DEG for phi in COARSE_ANGLES_DEG]


def tdoa_seconds(pair: Pair, theta_deg: float, phi_deg: float) -> float:
    theta = math.radians(theta_deg)
    phi = math.radians(phi_deg)
    return (
        pair.dx_m * math.sin(theta) * math.cos(phi)
        + pair.dy_m * math.sin(phi)
    ) / SPEED_OF_SOUND_MPS


def _source_tdoa_seconds(
    mics: list[Mic],
    pair: Pair,
    source: AcousticSource,
    scenario: AcousticScenario,
) -> float:
    mic_a = pair.mic_a
    mic_b = pair.mic_b
    if scenario.slot_permutation is not None:
        if len(scenario.slot_permutation) != len(mics):
            raise ValueError("slot_permutation length must match microphone count")
        mic_a = scenario.slot_permutation[mic_a]
        mic_b = scenario.slot_permutation[mic_b]

    if source.distance_m is None:
        theta = math.radians(source.theta_deg)
        phi = math.radians(source.phi_deg)
        dx = mics[mic_a].x_m - mics[mic_b].x_m
        dy = mics[mic_a].y_m - mics[mic_b].y_m
        return (
            dx * math.sin(theta) * math.cos(phi)
            + dy * math.sin(phi)
        ) / SPEED_OF_SOUND_MPS

    theta = math.radians(source.theta_deg)
    phi = math.radians(source.phi_deg)
    src_x = source.distance_m * math.sin(theta) * math.cos(phi)
    src_y = source.distance_m * math.sin(phi)
    src_z = max(0.05, source.distance_m * math.cos(theta) * math.cos(phi))
    da = math.sqrt((src_x - mics[mic_a].x_m) ** 2 + (src_y - mics[mic_a].y_m) ** 2 + src_z**2)
    db = math.sqrt((src_x - mics[mic_b].x_m) ** 2 + (src_y - mics[mic_b].y_m) ** 2 + src_z**2)
    return -(da - db) / SPEED_OF_SOUND_MPS


def phase_noise_sigma(snr_db: float) -> float:
    linear = 10.0 ** (snr_db / 20.0)
    return 0.55 / max(linear, 0.25)


def frequency_pair_weight(pair: Pair, frequency_hz: float, algorithm: str) -> float:
    if frequency_hz <= 0.0:
        return 0.0

    half_wavelength = SPEED_OF_SOUND_MPS / (2.0 * frequency_hz)
    alias_ratio = pair.baseline_m / max(half_wavelength, 1.0e-6)

    if algorithm == ALGORITHM_WIDE32_HF_HINT:
        if pair.baseline_m < 0.012:
            aperture = pair.baseline_m / 0.012
        else:
            aperture = 1.0
        if alias_ratio <= 1.0:
            return max(0.15, aperture)
        return max(0.05, aperture / (alias_ratio * alias_ratio))

    if alias_ratio <= 1.0:
        return 1.0
    if alias_ratio <= 2.0:
        return 0.65
    return 0.35


def _pair_reliability(pair: Pair, scenario: AcousticScenario) -> tuple[float, float]:
    bad = set(scenario.bad_channels)
    if pair.mic_a in bad or pair.mic_b in bad:
        return 0.0, 0.0

    polarity = 0.0
    flips = set(scenario.polarity_flips)
    if (pair.mic_a in flips) != (pair.mic_b in flips):
        polarity = math.pi

    if scenario.gain_error_db:
        if len(scenario.gain_error_db) <= max(pair.mic_a, pair.mic_b):
            raise ValueError("gain_error_db length must cover all used channels")
        mismatch_db = abs(scenario.gain_error_db[pair.mic_a] - scenario.gain_error_db[pair.mic_b])
        reliability = max(0.25, 1.0 - (mismatch_db / 24.0))
    else:
        reliability = 1.0

    return reliability, polarity


def _pair_delay_error(pair: Pair, scenario: AcousticScenario) -> float:
    if not scenario.delay_error_s:
        return 0.0
    if len(scenario.delay_error_s) <= max(pair.mic_a, pair.mic_b):
        raise ValueError("delay_error_s length must cover all used channels")
    return scenario.delay_error_s[pair.mic_a] - scenario.delay_error_s[pair.mic_b]


def build_observed_features(
    config: ImagingConfig,
    mics: list[Mic],
    pairs: list[Pair],
    scenario: AcousticScenario,
    algorithm: str,
    rng: random.Random,
) -> list[tuple[int, float, float, float, float]]:
    freqs = active_frequencies_hz(config)
    noise_amp = 10.0 ** (-scenario.snr_db / 20.0)
    features: list[tuple[int, float, float, float, float]] = []

    for pair_index, pair in enumerate(pairs):
        reliability, polarity = _pair_reliability(pair, scenario)
        if reliability <= 0.0:
            continue

        delay_error = _pair_delay_error(pair, scenario)
        for freq in freqs:
            obs_re = 0.0
            obs_im = 0.0
            for source in scenario.sources:
                tau = _source_tdoa_seconds(mics, pair, source, scenario)
                phase = (2.0 * math.pi * freq * (tau + delay_error + source.extra_delay_s)) + polarity
                obs_re += source.amplitude * math.cos(phase)
                obs_im += source.amplitude * math.sin(phase)

                if scenario.reflection_gain > 0.0:
                    reflected = AcousticSource(
                        theta_deg=-0.65 * source.theta_deg,
                        phi_deg=-source.phi_deg,
                        amplitude=source.amplitude * scenario.reflection_gain,
                        distance_m=source.distance_m,
                        extra_delay_s=source.extra_delay_s + scenario.reflection_delay_s,
                    )
                    tau_ref = _source_tdoa_seconds(mics, pair, reflected, scenario)
                    phase_ref = (2.0 * math.pi * freq * (tau_ref + delay_error + reflected.extra_delay_s)) + polarity
                    obs_re += reflected.amplitude * math.cos(phase_ref)
                    obs_im += reflected.amplitude * math.sin(phase_ref)

            if noise_amp > 0.0:
                noise_phase = rng.uniform(-math.pi, math.pi)
                obs_re += noise_amp * math.cos(noise_phase)
                obs_im += noise_amp * math.sin(noise_phase)

            mag = math.hypot(obs_re, obs_im)
            if mag <= 1.0e-9:
                continue

            weight = reliability * frequency_pair_weight(pair, freq, algorithm)
            if weight > 0.0:
                features.append((pair_index, freq, obs_re / mag, obs_im / mag, weight))

    return features


def estimate_direction(
    config: ImagingConfig,
    mics: list[Mic],
    pairs: list[Pair],
    scenario: AcousticScenario,
    algorithm: str,
    rng: random.Random,
) -> DirectionEstimate:
    features = build_observed_features(config, mics, pairs, scenario, algorithm, rng)
    best_theta = 0.0
    best_phi = 0.0
    best_score = -1.0e30
    second_score = -1.0e30
    total_weight = sum(feature[4] for feature in features)

    for test_theta, test_phi in coarse_grid():
        score = 0.0
        for pair_index, freq, obs_re, obs_im, weight in features:
            tau_test = tdoa_seconds(pairs[pair_index], test_theta, test_phi)
            phase = 2.0 * math.pi * freq * tau_test
            score += weight * ((obs_re * math.cos(phase)) + (obs_im * math.sin(phase)))

        if score > best_score:
            second_score = best_score
            best_score = score
            best_theta = test_theta
            best_phi = test_phi
        elif score > second_score:
            second_score = score

    if total_weight <= 0.0:
        return DirectionEstimate(0.0, 0.0, 0.0, 0.0, 0.0, 0)

    quality = (best_score - second_score) / max(abs(best_score), 1.0e-6)
    contrast = (best_score - second_score) / total_weight
    return DirectionEstimate(
        theta_deg=best_theta,
        phi_deg=best_phi,
        score=best_score / total_weight,
        quality=max(0.0, min(1.0, quality)),
        contrast=max(0.0, min(1.0, contrast)),
        active_feature_count=len(features),
    )


def angular_error_deg(estimate: DirectionEstimate, source: AcousticSource) -> float:
    return math.hypot(estimate.theta_deg - source.theta_deg, estimate.phi_deg - source.phi_deg)


def percentile(values: list[float], pct: float) -> float:
    if not values:
        return 0.0
    ordered = sorted(values)
    rank = (pct / 100.0) * (len(ordered) - 1)
    low = int(math.floor(rank))
    high = int(math.ceil(rank))
    if low == high:
        return ordered[low]
    frac = rank - low
    return ordered[low] * (1.0 - frac) + ordered[high] * frac


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
