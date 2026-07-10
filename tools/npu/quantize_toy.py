"""Static int8 (QDQ) quantization of the toy classifier with synthetic
calibration data, so Neural-ART maps the convolutions onto hardware epochs."""

import numpy as np
from onnxruntime.quantization import CalibrationDataReader, QuantFormat, QuantType, quantize_static


class SyntheticSpectra(CalibrationDataReader):
    def __init__(self, count=64):
        rng = np.random.default_rng(20260711)
        # Log-spectrum-like data: mostly low floor with sparse peaks, 0..1.
        self.batches = []
        for _ in range(count):
            base = rng.uniform(0.02, 0.15, size=(1, 1, 32, 64)).astype(np.float32)
            for _ in range(rng.integers(1, 5)):
                bin_center = rng.integers(2, 62)
                base[..., bin_center - 1 : bin_center + 2] += rng.uniform(0.4, 0.9)
            self.batches.append({"spectrogram": np.clip(base, 0.0, 1.0)})
        self.it = iter(self.batches)

    def get_next(self):
        return next(self.it, None)


quantize_static(
    "tools/npu/toy_sound_classifier.onnx",
    "tools/npu/toy_sound_classifier_int8.onnx",
    SyntheticSpectra(),
    quant_format=QuantFormat.QDQ,
    activation_type=QuantType.QInt8,
    weight_type=QuantType.QInt8,
    per_channel=True,
)
print("saved tools/npu/toy_sound_classifier_int8.onnx")
