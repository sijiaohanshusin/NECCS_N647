"""Evaluate float and int8 ONNX classifiers over the full collected dataset."""
import sys
from pathlib import Path

import numpy as np
import onnxruntime as ort

DATASET = Path(__file__).parent / "dataset"
LABELS = ["listening", "gas_leak", "bearing", "arc", "impact", "ambient"]


def evaluate(model_path: str) -> None:
    sess = ort.InferenceSession(model_path, providers=["CPUExecutionProvider"])
    total, correct = 0, 0
    confusion = np.zeros((6, 6), dtype=int)
    for idx, label in enumerate(LABELS):
        folder = DATASET / label
        for f in sorted(folder.glob("*.npy")):
            x = np.load(f).astype(np.float32).reshape(1, 1, 32, 64)
            scores = sess.run(None, {"spectrogram": x})[0]
            pred = int(np.argmax(scores))
            confusion[idx, pred] += 1
            correct += int(pred == idx)
            total += 1
    print(f"{model_path}: acc={correct / total:.3f} ({correct}/{total})")
    print("confusion (rows=true, cols=pred):")
    header = " ".join(f"{l[:6]:>6s}" for l in LABELS)
    print(f"{'':12s}{header}")
    for i, l in enumerate(LABELS):
        row = " ".join(f"{confusion[i, j]:6d}" for j in range(6))
        print(f"{l:12s}{row}")


for m in sys.argv[1:]:
    evaluate(m)
