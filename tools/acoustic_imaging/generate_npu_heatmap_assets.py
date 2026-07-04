#!/usr/bin/env python3
"""Generate the first no-MIC NPU heatmap model assets.

The generated model is a fixed Wide32/BALANCED coarse-grid linear projection:

  int8 GCC-PHAT features [1, 160 pairs * 40 bins * 2] -> int8 heatmap [1, 81]

The script always writes a small JSON spec using only the Python standard
library. With the optional `onnx` Python package installed, `--emit-onnx` also
writes a QLinearMatMul ONNX model that can be passed to ST Edge AI.
"""

from __future__ import annotations

import argparse
import json
import math
from pathlib import Path
import sys

from acoustic_imaging_model import (
    MODE_WIDE32,
    PROFILE_BALANCED,
    build_config,
    coarse_grid,
    load_mics,
    select_longest_pairs,
    select_mode_mics,
    tdoa_seconds,
)

ROOT = Path(__file__).resolve().parents[2]
DEFAULT_COORDS = ROOT / "docs" / "knowledge" / "microphone-array" / "array_32ch_coords.csv"
DEFAULT_OUTPUT_DIR = ROOT / "tools" / "acoustic_imaging" / "npu" / "generated"
DEFAULT_ONNX_NAME = "npu_heatmap_qlinear_matmul.onnx"


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--coords", type=Path, default=DEFAULT_COORDS)
    parser.add_argument("--output-dir", type=Path, default=DEFAULT_OUTPUT_DIR)
    parser.add_argument("--emit-onnx", action="store_true")
    parser.add_argument("--onnx-name", default=DEFAULT_ONNX_NAME)
    parser.add_argument("--opset", type=int, default=13)
    return parser.parse_args()


def quantize_unit(value: float) -> int:
    value = max(-1.0, min(1.0, value))
    return max(-127, min(127, int(round(value * 127.0))))


def build_weight_matrix(config, pairs) -> list[int]:
    grid = coarse_grid()
    active_bins = list(config.active_bins)
    input_count = config.pair_count * len(active_bins) * 2
    output_count = len(grid)
    weights = [0] * (input_count * output_count)

    for grid_index, (theta_deg, phi_deg) in enumerate(grid):
        for pair_index, pair in enumerate(pairs):
            tau = tdoa_seconds(pair, theta_deg, phi_deg)
            for local_bin, bin_index in enumerate(active_bins):
                phase = 2.0 * math.pi * (config.sample_rate_hz / config.nfft) * bin_index * tau
                base = (pair_index * len(active_bins) + local_bin) * 2
                weights[(base * output_count) + grid_index] = quantize_unit(math.cos(phase))
                weights[((base + 1) * output_count) + grid_index] = quantize_unit(math.sin(phase))

    if len(weights) != input_count * output_count:
        raise AssertionError("unexpected weight matrix size")
    return weights


def write_spec(path: Path, config, input_count: int, output_count: int, y_scale: float) -> None:
    spec = {
        "name": "neccs_wide32_balanced_coarse_heatmap_qlinear",
        "purpose": "No-MIC STM32N6 NPU bring-up model for acoustic coarse heatmap projection.",
        "mode": config.mode,
        "profile": config.profile,
        "sample_rate_hz": config.sample_rate_hz,
        "nfft": config.nfft,
        "active_bin_start": config.active_bin_start,
        "active_bin_end": config.active_bin_end,
        "pair_count": config.pair_count,
        "input_order": "pair-major, active-bin-major, real then imaginary GCC-PHAT int8 values",
        "input_shape": [1, input_count],
        "output_shape": [1, output_count],
        "input_scale": 1.0 / 127.0,
        "weight_scale": 1.0 / 127.0,
        "output_scale": y_scale,
        "input_zero_point": 0,
        "weight_zero_point": 0,
        "output_zero_point": 0,
    }
    path.write_text(json.dumps(spec, indent=2) + "\n", encoding="utf-8")


def write_onnx(path: Path, config, input_count: int, output_count: int, y_scale: float, weights: list[int], opset: int) -> None:
    try:
        import onnx
        from onnx import TensorProto, helper
    except ModuleNotFoundError as exc:
        raise RuntimeError(
            "The optional 'onnx' package is required for --emit-onnx. "
            "Install it in your chosen Python environment, then rerun this script."
        ) from exc

    input_tensor = helper.make_tensor_value_info("gcc_phat_i8", TensorProto.INT8, [1, input_count])
    output_tensor = helper.make_tensor_value_info("heatmap_i8", TensorProto.INT8, [1, output_count])
    weight_tensor = helper.make_tensor("heatmap_w_i8", TensorProto.INT8, [input_count, output_count], weights)
    input_scale = helper.make_tensor("gcc_scale", TensorProto.FLOAT, [], [1.0 / 127.0])
    weight_scale = helper.make_tensor("weight_scale", TensorProto.FLOAT, [], [1.0 / 127.0])
    output_scale = helper.make_tensor("heatmap_scale", TensorProto.FLOAT, [], [y_scale])
    input_zero = helper.make_tensor("gcc_zero", TensorProto.INT8, [], [0])
    weight_zero = helper.make_tensor("weight_zero", TensorProto.INT8, [], [0])
    output_zero = helper.make_tensor("heatmap_zero", TensorProto.INT8, [], [0])

    node = helper.make_node(
        "QLinearMatMul",
        [
            "gcc_phat_i8",
            "gcc_scale",
            "gcc_zero",
            "heatmap_w_i8",
            "weight_scale",
            "weight_zero",
            "heatmap_scale",
            "heatmap_zero",
        ],
        ["heatmap_i8"],
        name="neccs_coarse_heatmap_qlinear",
    )

    graph = helper.make_graph(
        [node],
        "neccs_wide32_balanced_coarse_heatmap",
        [input_tensor],
        [output_tensor],
        [weight_tensor, input_scale, weight_scale, output_scale, input_zero, weight_zero, output_zero],
    )
    model = helper.make_model(
        graph,
        producer_name="NECCS_N647 acoustic imaging tools",
        opset_imports=[helper.make_operatorsetid("", opset)],
    )
    onnx.checker.check_model(model)
    onnx.save(model, path)


def main() -> int:
    args = parse_args()
    args.output_dir.mkdir(parents=True, exist_ok=True)

    all_mics = load_mics(args.coords)
    config = build_config(MODE_WIDE32, PROFILE_BALANCED)
    mics = select_mode_mics(all_mics, config.mode)
    pairs = select_longest_pairs(mics, config.pair_count)
    active_bin_count = (config.active_bin_end - config.active_bin_start) + 1
    input_count = config.pair_count * active_bin_count * 2
    output_count = len(coarse_grid())
    y_scale = float(config.pair_count * active_bin_count) / 127.0

    spec_path = args.output_dir / "npu_heatmap_model_spec.json"
    write_spec(spec_path, config, input_count, output_count, y_scale)
    print(f"wrote: {spec_path}")

    if args.emit_onnx:
      weights = build_weight_matrix(config, pairs)
      onnx_path = args.output_dir / args.onnx_name
      write_onnx(onnx_path, config, input_count, output_count, y_scale, weights, args.opset)
      print(f"wrote: {onnx_path}")

    return 0


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except RuntimeError as exc:
        print(f"error: {exc}", file=sys.stderr)
        raise SystemExit(2)
