"""Generate a toy 1D-CNN sound-classifier ONNX model (random weights).

Input : [1, 1, 32, 64]  - 32 frames x 64 log-spectrum bins (AppAcousticService
                          already produces spectrum[64] per frame).
Output: [1, 6]          - class scores.

Purpose: walk the ST Edge AI (Neural-ART) toolchain end to end before the
real trained model exists. Architecture chosen to match the planned real
model so memory/latency numbers carry over.
"""

import numpy as np
import onnx
from onnx import TensorProto, helper


def conv_weights(name, cout, cin, kh, kw):
    w = (np.random.randn(cout, cin, kh, kw) * 0.1).astype(np.float32)
    b = np.zeros(cout, dtype=np.float32)
    return (
        helper.make_tensor(name + "_w", TensorProto.FLOAT, w.shape, w.tobytes(), raw=True),
        helper.make_tensor(name + "_b", TensorProto.FLOAT, b.shape, b.tobytes(), raw=True),
    )


def main():
    np.random.seed(20260711)

    inp = helper.make_tensor_value_info("spectrogram", TensorProto.FLOAT, [1, 1, 32, 64])
    out = helper.make_tensor_value_info("class_scores", TensorProto.FLOAT, [1, 6])

    c1w, c1b = conv_weights("c1", 8, 1, 3, 3)
    c2w, c2b = conv_weights("c2", 16, 8, 3, 3)
    c3w, c3b = conv_weights("c3", 32, 16, 3, 3)
    fcw = (np.random.randn(6, 32 * 4 * 8) * 0.05).astype(np.float32)
    fcb = np.zeros(6, dtype=np.float32)
    fc_w = helper.make_tensor("fc_w", TensorProto.FLOAT, fcw.shape, fcw.tobytes(), raw=True)
    fc_b = helper.make_tensor("fc_b", TensorProto.FLOAT, fcb.shape, fcb.tobytes(), raw=True)

    nodes = [
        helper.make_node("Conv", ["spectrogram", "c1_w", "c1_b"], ["c1"], pads=[1, 1, 1, 1]),
        helper.make_node("Relu", ["c1"], ["r1"]),
        helper.make_node("MaxPool", ["r1"], ["p1"], kernel_shape=[2, 2], strides=[2, 2]),
        helper.make_node("Conv", ["p1", "c2_w", "c2_b"], ["c2"], pads=[1, 1, 1, 1]),
        helper.make_node("Relu", ["c2"], ["r2"]),
        helper.make_node("MaxPool", ["r2"], ["p2"], kernel_shape=[2, 2], strides=[2, 2]),
        helper.make_node("Conv", ["p2", "c3_w", "c3_b"], ["c3"], pads=[1, 1, 1, 1]),
        helper.make_node("Relu", ["c3"], ["r3"]),
        helper.make_node("MaxPool", ["r3"], ["p3"], kernel_shape=[2, 2], strides=[2, 2]),
        helper.make_node("Flatten", ["p3"], ["flat"]),
        # No Softmax on purpose: raw logits keep the whole graph on NPU
        # hardware epochs (softmax was the lone SW epoch and drags in the
        # X-CUBE-AI kernel library); the app applies softmax to 6 values.
        helper.make_node("Gemm", ["flat", "fc_w", "fc_b"], ["class_scores"], transB=1),
    ]

    graph = helper.make_graph(
        nodes,
        "neccs_sound_classifier_toy",
        [inp],
        [out],
        initializer=[c1w, c1b, c2w, c2b, c3w, c3b, fc_w, fc_b],
    )
    model = helper.make_model(
        graph,
        opset_imports=[helper.make_opsetid("", 13)],
        producer_name="neccs-toy",
    )
    model.ir_version = 8
    onnx.checker.check_model(model)
    onnx.save(model, "tools/npu/toy_sound_classifier.onnx")
    print("saved tools/npu/toy_sound_classifier.onnx")


if __name__ == "__main__":
    main()
