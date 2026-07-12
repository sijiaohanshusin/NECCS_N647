"""Static int8 (QDQ) quantization of the trained NECCS classifier.

Calibration uses real collected windows (raw int8 values as float, matching
the firmware feed and the Mul(1/128) front-end baked into the ONNX graph).
"""

from pathlib import Path

import numpy as np
from onnxruntime.quantization import CalibrationDataReader, QuantFormat, QuantType, quantize_static

DATASET = Path(__file__).parent / "dataset"


class RealSpectra(CalibrationDataReader):
    def __init__(self, per_class=24):
        rng = np.random.default_rng(20260712)
        self.batches = []
        for folder in sorted(p for p in DATASET.iterdir() if p.is_dir()):
            files = sorted(folder.glob("*.npy"))
            picks = rng.choice(len(files), size=min(per_class, len(files)), replace=False)
            for i in picks:
                w = np.load(files[i]).astype(np.float32).reshape(1, 1, 32, 64)
                self.batches.append({"spectrogram": w})
        rng.shuffle(self.batches)
        self.it = iter(self.batches)

    def get_next(self):
        return next(self.it, None)


quantize_static(
    "tools/npu/neccs_cnn.onnx",
    "tools/npu/neccs_cnn_int8.onnx",
    RealSpectra(),
    quant_format=QuantFormat.QDQ,
    activation_type=QuantType.QInt8,
    weight_type=QuantType.QInt8,
    per_channel=True,
)
print("saved tools/npu/neccs_cnn_int8.onnx")
