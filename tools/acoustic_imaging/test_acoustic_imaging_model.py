import math
import random
import unittest
from pathlib import Path

from acoustic_imaging_model import (
    ALGORITHM_WIDE32_GENERAL_SRP,
    ALGORITHM_WIDE32_HF_HINT,
    AcousticScenario,
    AcousticSource,
    BIN_POLICY_QUALITY_B40,
    BIN_POLICY_STANDARD_B12,
    BIN_POLICY_STANDARD_B16,
    BIN_POLICY_STANDARD_B24,
    MODE_CORE16,
    MODE_WIDE32,
    PROFILE_BALANCED,
    PROFILE_FAST,
    PROFILE_QUALITY,
    angular_error_deg,
    active_frequencies_hz,
    bin_mask_for_policy,
    build_algorithm_config,
    build_config,
    build_tdoa_lut,
    coarse_grid,
    estimate_direction,
    load_mics,
    select_algorithm_pairs,
    select_longest_pairs,
    select_mode_mics,
    summarize_pair_set,
    tdoa_seconds,
    with_bin_policy,
)


ROOT = Path(__file__).resolve().parents[2]
COORDS = ROOT / "docs" / "knowledge" / "microphone-array" / "array_32ch_coords.csv"


class AcousticImagingModelTest(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.all_mics = load_mics(COORDS)

    def test_wide32_profiles_cover_all_mics(self):
        mics = select_mode_mics(self.all_mics, MODE_WIDE32)
        for profile in (PROFILE_FAST, PROFILE_BALANCED, PROFILE_QUALITY):
            config = build_config(MODE_WIDE32, profile)
            pairs = select_longest_pairs(mics, config.pair_count)
            summary = summarize_pair_set(mics, pairs)
            self.assertEqual(summary["pair_count"], config.pair_count)
            self.assertEqual(summary["missing_channels"], [])
            self.assertGreaterEqual(summary["coverage_min"], 1)

    def test_core16_balanced_uses_all_pairs(self):
        mics = select_mode_mics(self.all_mics, MODE_CORE16)
        config = build_config(MODE_CORE16, PROFILE_BALANCED)
        pairs = select_longest_pairs(mics, config.pair_count)
        summary = summarize_pair_set(mics, pairs)
        self.assertEqual(len(mics), 16)
        self.assertEqual(config.sample_rate_hz, 192000)
        self.assertEqual(config.nfft, 512)
        self.assertEqual(summary["pair_count"], 120)
        self.assertEqual(summary["coverage_min"], 15)

    def test_tdoa_lut_shape(self):
        mics = select_mode_mics(self.all_mics, MODE_WIDE32)
        config = build_config(MODE_WIDE32, PROFILE_FAST)
        pairs = select_longest_pairs(mics, config.pair_count)
        lut = build_tdoa_lut(pairs)
        self.assertEqual(len(lut), 81)
        self.assertEqual(len(lut[0]), config.pair_count)

    def test_wide32_balanced_synthetic_direction_error(self):
        mics = select_mode_mics(self.all_mics, MODE_WIDE32)
        config = build_config(MODE_WIDE32, PROFILE_BALANCED)
        pairs = select_longest_pairs(mics, config.pair_count)
        freqs = active_frequencies_hz(config)

        for true_theta, true_phi in [(-45.0, -30.0), (0.0, 0.0), (45.0, 30.0)]:
            best_theta = 0.0
            best_phi = 0.0
            best_score = -1.0e30

            for test_theta, test_phi in coarse_grid():
                score = 0.0
                for pair in pairs:
                    tau_true = tdoa_seconds(pair, true_theta, true_phi)
                    tau_test = tdoa_seconds(pair, test_theta, test_phi)
                    for freq in freqs:
                        phase = 2.0 * math.pi * freq * (tau_true - tau_test)
                        score += math.cos(phase)

                if score > best_score:
                    best_score = score
                    best_theta = test_theta
                    best_phi = test_phi

            error_deg = ((best_theta - true_theta) ** 2 + (best_phi - true_phi) ** 2) ** 0.5
            self.assertLessEqual(error_deg, 15.0)

    def test_hf_hint_selects_shorter_pairs_than_general(self):
        mics = select_mode_mics(self.all_mics, MODE_WIDE32)
        general_config = build_algorithm_config(ALGORITHM_WIDE32_GENERAL_SRP)
        hf_config = build_algorithm_config(ALGORITHM_WIDE32_HF_HINT)
        general_pairs = select_algorithm_pairs(mics, general_config, ALGORITHM_WIDE32_GENERAL_SRP)
        hf_pairs = select_algorithm_pairs(mics, hf_config, ALGORITHM_WIDE32_HF_HINT)

        general_mean = sum(pair.baseline_m for pair in general_pairs) / len(general_pairs)
        hf_mean = sum(pair.baseline_m for pair in hf_pairs) / len(hf_pairs)
        self.assertLess(hf_mean, general_mean * 0.55)

    def test_wide32_bin_policies_are_sorted_and_sized(self):
        expected_counts = {
            BIN_POLICY_STANDARD_B12: 12,
            BIN_POLICY_STANDARD_B16: 16,
            BIN_POLICY_STANDARD_B24: 24,
            BIN_POLICY_QUALITY_B40: 40,
        }

        for policy, count in expected_counts.items():
            bins = bin_mask_for_policy(policy)
            self.assertEqual(len(bins), count)
            self.assertEqual(tuple(sorted(set(bins))), bins)
            self.assertGreaterEqual(min(bins), 3)
            self.assertLessEqual(max(bins), 42)

    def test_with_bin_policy_changes_active_frequencies(self):
        config = build_config(MODE_WIDE32, PROFILE_BALANCED)
        b16_config = with_bin_policy(config, BIN_POLICY_STANDARD_B16)
        self.assertEqual(len(b16_config.active_bins), 16)
        self.assertEqual(len(active_frequencies_hz(b16_config)), 16)
        self.assertEqual(b16_config.active_bins[0], 6)
        self.assertEqual(b16_config.active_bins[-1], 41)

    def test_wide32_general_robust_direction_estimate(self):
        rng = random.Random(647)
        mics = select_mode_mics(self.all_mics, MODE_WIDE32)
        config = build_algorithm_config(ALGORITHM_WIDE32_GENERAL_SRP)
        pairs = select_algorithm_pairs(mics, config, ALGORITHM_WIDE32_GENERAL_SRP)
        scenario = AcousticScenario(
            name="unit_robust",
            sources=(AcousticSource(30.0, -15.0),),
            snr_db=10.0,
            bad_channels=(0, 7),
            gain_error_db=tuple(1.5 if idx % 3 == 0 else -1.0 for idx in range(32)),
            delay_error_s=tuple((idx % 5) * 0.2e-6 for idx in range(32)),
            reflection_gain=0.12,
        )

        estimate = estimate_direction(config, mics, pairs, scenario, ALGORITHM_WIDE32_GENERAL_SRP, rng)
        self.assertGreater(estimate.active_feature_count, 0)
        self.assertLessEqual(angular_error_deg(estimate, scenario.sources[0]), 15.0)


if __name__ == "__main__":
    unittest.main()
