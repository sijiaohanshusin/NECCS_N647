import math
import unittest
from pathlib import Path

from acoustic_imaging_model import (
    MODE_CORE16,
    MODE_WIDE32,
    PROFILE_BALANCED,
    PROFILE_FAST,
    PROFILE_QUALITY,
    active_frequencies_hz,
    build_config,
    build_tdoa_lut,
    coarse_grid,
    load_mics,
    select_longest_pairs,
    select_mode_mics,
    summarize_pair_set,
    tdoa_seconds,
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


if __name__ == "__main__":
    unittest.main()
