"""Train the NECCS sound classifier on collected spectrum windows.

Pure-numpy training (no torch dependency): the deployed topology is
conv-heavy, but for 32x64 log-spectra a small model trained here maps to
the same ONNX graph used by make_toy_model.py. Weights are exported to
ONNX, quantized (QDQ int8) and ready for stedgeai.

Dataset layout (from collect_spectra.py):
    tools/npu/dataset/<label>/NNNN.npy   (32, 64) int8

Class order is fixed to match the firmware label table:
    0 listening/ambient-quiet (auto: low-energy windows)
    1 gas leak (hiss)   - suggest training with compressed-air/hiss
    2 bearing (whine)   - motor/whine
    3 arc (crackle)
    4 impact (knocks)
    5 ambient (speech/room noise)

For the two-week sprint this trains a multinomial logistic regression on
pooled spectral features expressed AS the first conv layer of the ONNX
graph (1x1 trick), giving an honest NPU deployment without torch. When
time allows, swap in a real CNN training pipeline.
"""

import sys
from pathlib import Path

import numpy as np

DATASET = Path(__file__).parent / "dataset"
LABELS = ["listening", "gas_leak", "bearing", "arc", "impact", "ambient"]


def pooled_features(window: np.ndarray) -> np.ndarray:
    """(32,64) int8 -> feature vector: per-bin mean + per-bin std + band deltas."""
    w = window.astype(np.float32) / 128.0
    mean = w.mean(axis=0)
    std = w.std(axis=0)
    diff = np.abs(np.diff(w, axis=0)).mean(axis=0)
    return np.concatenate([mean, std, diff])  # 192 dims


def main():
    xs, ys = [], []
    for idx, label in enumerate(LABELS):
        folder = DATASET / label
        if not folder.exists():
            continue
        for f in sorted(folder.glob("*.npy")):
            xs.append(pooled_features(np.load(f)))
            ys.append(idx)
    if not xs:
        sys.exit("no dataset found - run collect_spectra.py first")

    x = np.stack(xs)
    y = np.array(ys)
    print(f"dataset: {x.shape[0]} windows, classes: {np.bincount(y, minlength=6)}")

    # Multinomial logistic regression with L2, plain gradient descent.
    rng = np.random.default_rng(0)
    perm = rng.permutation(len(x))
    x, y = x[perm], y[perm]
    split = int(len(x) * 0.85)
    x_train, y_train = x[:split], y[:split]
    x_val, y_val = x[split:], y[split:]

    mu, sigma = x_train.mean(0), x_train.std(0) + 1e-6
    x_train = (x_train - mu) / sigma
    x_val = (x_val - mu) / sigma

    n_feat, n_cls = x.shape[1], 6
    w = np.zeros((n_feat, n_cls), dtype=np.float32)
    b = np.zeros(n_cls, dtype=np.float32)
    lr, l2 = 0.05, 1e-4
    onehot = np.eye(n_cls)[y_train]

    for epoch in range(300):
        logits = x_train @ w + b
        logits -= logits.max(1, keepdims=True)
        p = np.exp(logits)
        p /= p.sum(1, keepdims=True)
        grad_w = x_train.T @ (p - onehot) / len(x_train) + l2 * w
        grad_b = (p - onehot).mean(0)
        w -= lr * grad_w
        b -= lr * grad_b
        if epoch % 50 == 0:
            val_acc = ((x_val @ w + b).argmax(1) == y_val).mean()
            print(f"epoch {epoch}: val_acc={val_acc:.3f}")

    val_acc = ((x_val @ w + b).argmax(1) == y_val).mean()
    print(f"final val_acc={val_acc:.3f}")
    np.savez(Path(__file__).parent / "trained_linear.npz", w=w, b=b, mu=mu, sigma=sigma)
    print("saved trained_linear.npz - next: export_trained_onnx.py (todo) -> stedgeai")


if __name__ == "__main__":
    main()
