"""Train the NECCS sound classifier as a small CNN that maps 100% onto
Neural-ART hardware epochs (same topology as make_toy_model.py, which was
verified to compile to pure-HW epochs on this board).

Input : [1, 1, 32, 64] raw int8 spectrum window as float (-128..127)
Output: [1, 6] class logits (softmax left to the app on purpose)

Usage: python tools/npu/train_cnn.py
Writes tools/npu/neccs_cnn.onnx
"""

from pathlib import Path

import numpy as np
import torch
import torch.nn as nn

DATASET = Path(__file__).parent / "dataset"
LABELS = ["listening", "gas_leak", "bearing", "arc", "impact", "ambient"]


class SoundCnn(nn.Module):
    def __init__(self):
        super().__init__()
        self.net = nn.Sequential(
            nn.Conv2d(1, 8, 3, padding=1), nn.ReLU(), nn.MaxPool2d(2),
            nn.Conv2d(8, 16, 3, padding=1), nn.ReLU(), nn.MaxPool2d(2),
            nn.Conv2d(16, 32, 3, padding=1), nn.ReLU(), nn.MaxPool2d(2),
            nn.Flatten(),
            nn.Linear(32 * 4 * 8, 6),
        )

    def forward(self, x):
        return self.net(x)


def load_dataset():
    xs, ys = [], []
    for idx, label in enumerate(LABELS):
        for f in sorted((DATASET / label).glob("*.npy")):
            xs.append(np.load(f).astype(np.float32))
            ys.append(idx)
    return np.stack(xs)[:, None, :, :], np.array(ys)


def augment(batch, rng):
    """Random circular time shift and small gain jitter (int8 domain)."""
    out = batch.clone()
    for i in range(out.shape[0]):
        shift = int(rng.integers(-4, 5))
        if shift:
            out[i] = torch.roll(out[i], shifts=shift, dims=1)
        gain = float(rng.uniform(0.9, 1.1))
        out[i] = torch.clamp(out[i] * gain, -128.0, 127.0)
    return out


def main():
    torch.manual_seed(7)
    rng = np.random.default_rng(7)

    x, y = load_dataset()
    print(f"dataset: {x.shape[0]} windows, classes: {np.bincount(y, minlength=6)}")

    perm = rng.permutation(len(x))
    x, y = x[perm], y[perm]
    split = int(len(x) * 0.85)
    x_train = torch.tensor(x[:split])
    y_train = torch.tensor(y[:split], dtype=torch.long)
    x_val = torch.tensor(x[split:])
    y_val = torch.tensor(y[split:], dtype=torch.long)

    model = SoundCnn()
    opt = torch.optim.Adam(model.parameters(), lr=1e-3, weight_decay=1e-4)
    loss_fn = nn.CrossEntropyLoss()

    best_acc, best_state = 0.0, None
    for epoch in range(60):
        model.train()
        order = torch.randperm(len(x_train))
        for i in range(0, len(order), 32):
            idx = order[i:i + 32]
            xb = augment(x_train[idx], rng)
            opt.zero_grad()
            loss = loss_fn(model(xb), y_train[idx])
            loss.backward()
            opt.step()

        model.eval()
        with torch.no_grad():
            acc = (model(x_val).argmax(1) == y_val).float().mean().item()
        if acc > best_acc:
            best_acc = acc
            best_state = {k: v.clone() for k, v in model.state_dict().items()}
        if epoch % 10 == 0:
            print(f"epoch {epoch}: val_acc={acc:.3f}")

    print(f"best val_acc={best_acc:.3f}")
    model.load_state_dict(best_state)
    model.eval()

    dummy = torch.zeros(1, 1, 32, 64)
    torch.onnx.export(
        model, dummy, str(Path(__file__).parent / "neccs_cnn.onnx"),
        input_names=["spectrogram"], output_names=["class_scores"],
        opset_version=13, dynamo=False,
    )
    print("saved tools/npu/neccs_cnn.onnx")


if __name__ == "__main__":
    main()
