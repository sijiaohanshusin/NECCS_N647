"""Quick sanity check: per-class window counts and coarse spectral signature."""
import numpy as np
from pathlib import Path

root = Path(__file__).parent / "dataset"
for d in sorted(p for p in root.iterdir() if p.is_dir()):
    files = sorted(d.glob("*.npy"))
    arr = np.stack([np.load(f) for f in files]).astype(np.float32)
    lo = arr[:, :, :21].mean()
    mid = arr[:, :, 21:42].mean()
    hi = arr[:, :, 42:].mean()
    print(f"{d.name:12s} n={len(files):4d} lo={lo:6.1f} mid={mid:6.1f} "
          f"hi={hi:6.1f} std={arr.std():5.1f}")
