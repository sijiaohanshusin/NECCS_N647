"""Export trained_linear.npz to a deployable ONNX graph for stedgeai.

Input matches the firmware feed: [1, 1, 32, 64] int8-quantized spectra
(as float in ONNX; QDQ quantization happens after export). The pooled
features (per-bin mean, std, mean|frame-diff|) are expressed as ONNX ops
so the whole classifier runs on the NPU/accelerated path.

Pipeline:
    python tools/npu/train_classifier.py
    python tools/npu/export_trained_onnx.py
    python tools/npu/quantize_toy.py  --model tools/npu/neccs_classifier.onnx (adapt)
    stedgeai generate --model tools/npu/neccs_classifier_int8.onnx --target stm32n6 \
        --st-neural-art "neccs-debug@tools/npu/neccs_neural_art.json" --output ...
"""

from pathlib import Path

import numpy as np
import onnx
from onnx import TensorProto, helper


def main():
    blob = np.load(Path(__file__).parent / "trained_linear.npz")
    w = blob["w"].astype(np.float32)          # (192, 6)
    b = blob["b"].astype(np.float32)          # (6,)
    mu = blob["mu"].astype(np.float32)        # (192,)
    sigma = blob["sigma"].astype(np.float32)  # (192,)

    # Fold feature normalization into the linear layer:
    # logits = ((f - mu)/sigma) @ w + b = f @ (w/sigma[:,None]) + (b - (mu/sigma) @ w)
    w_folded = (w / sigma[:, None]).astype(np.float32)
    b_folded = (b - (mu / sigma) @ w).astype(np.float32)

    inp = helper.make_tensor_value_info("spectrogram", TensorProto.FLOAT, [1, 1, 32, 64])
    out = helper.make_tensor_value_info("class_scores", TensorProto.FLOAT, [1, 6])

    # x: [1,1,32,64] -> scaled to match training (int8/128).
    scale = helper.make_tensor("inv128", TensorProto.FLOAT, [], np.array([1.0 / 128.0], np.float32).tobytes(), raw=True)
    fc_w = helper.make_tensor("fc_w", TensorProto.FLOAT, [6, 192], w_folded.T.tobytes(), raw=True)
    fc_b = helper.make_tensor("fc_b", TensorProto.FLOAT, [6], b_folded.tobytes(), raw=True)

    nodes = [
        helper.make_node("Mul", ["spectrogram", "inv128"], ["xs"]),
        # mean over frames: [1,1,32,64] -> [1,1,1,64]
        helper.make_node("ReduceMean", ["xs"], ["mean"], axes=[2], keepdims=1),
        # std = sqrt(mean(x^2) - mean^2 + eps)
        helper.make_node("Mul", ["xs", "xs"], ["xsq"]),
        helper.make_node("ReduceMean", ["xsq"], ["meansq"], axes=[2], keepdims=1),
        helper.make_node("Mul", ["mean", "mean"], ["meanmean"]),
        helper.make_node("Sub", ["meansq", "meanmean"], ["var"]),
        helper.make_node("Sqrt", ["var"], ["std"]),
        # frame diff: x[1:] - x[:-1] via two slices
        helper.make_node("Slice", ["xs", "one", "int32max", "axis2"], ["tail"]),
        helper.make_node("Slice", ["xs", "zero", "thirtyone", "axis2"], ["head"]),
        helper.make_node("Sub", ["tail", "head"], ["dx"]),
        helper.make_node("Abs", ["dx"], ["absdx"]),
        helper.make_node("ReduceMean", ["absdx"], ["diff"], axes=[2], keepdims=1),
        # concat features along the bin axis: [1,1,1,192]
        helper.make_node("Concat", ["mean", "std", "diff"], ["feat4d"], axis=3),
        helper.make_node("Flatten", ["feat4d"], ["feat"]),
        helper.make_node("Gemm", ["feat", "fc_w", "fc_b"], ["class_scores"], transB=1),
    ]

    inits = [
        scale, fc_w, fc_b,
        helper.make_tensor("zero", TensorProto.INT64, [1], np.array([0], np.int64).tobytes(), raw=True),
        helper.make_tensor("one", TensorProto.INT64, [1], np.array([1], np.int64).tobytes(), raw=True),
        helper.make_tensor("thirtyone", TensorProto.INT64, [1], np.array([31], np.int64).tobytes(), raw=True),
        helper.make_tensor("int32max", TensorProto.INT64, [1], np.array([2**31 - 1], np.int64).tobytes(), raw=True),
        helper.make_tensor("axis2", TensorProto.INT64, [1], np.array([2], np.int64).tobytes(), raw=True),
    ]

    graph = helper.make_graph(nodes, "neccs_sound_classifier", [inp], [out], initializer=inits)
    model = helper.make_model(graph, opset_imports=[helper.make_opsetid("", 13)], producer_name="neccs")
    model.ir_version = 8
    onnx.checker.check_model(model)
    onnx.save(model, str(Path(__file__).parent / "neccs_classifier.onnx"))
    print("saved tools/npu/neccs_classifier.onnx")


if __name__ == "__main__":
    main()
