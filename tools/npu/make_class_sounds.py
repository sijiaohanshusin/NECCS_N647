"""Generate characteristic training sounds for the 5 audible NPU classes.

Each WAV is 48 kHz mono 16-bit, `--seconds` long, and mimics the spectral
signature the on-device classifier should associate with the class:

    gas_leak : high-passed hiss with slow amplitude flutter
    bearing  : rotating-machine whine (fundamental + harmonics + resonance, light FM)
    arc      : impulsive crackle bursts over a faint HF bed
    impact   : sparse decaying knocks
    ambient  : Windows TTS speech handles this one (see collect_all.ps1)

Usage: python make_class_sounds.py --out _debug_logs\\npu_sounds --seconds 70
"""

import argparse
import struct
import wave
from pathlib import Path

import numpy as np

RATE = 48000


def write_wav(path: Path, x: np.ndarray) -> None:
    x = np.clip(x, -1.0, 1.0)
    pcm = (x * 32000.0).astype(np.int16)
    with wave.open(str(path), "wb") as w:
        w.setnchannels(1)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(pcm.tobytes())


def highpass(x: np.ndarray, cutoff_hz: float) -> np.ndarray:
    spec = np.fft.rfft(x)
    freqs = np.fft.rfftfreq(len(x), 1.0 / RATE)
    ramp = np.clip((freqs - cutoff_hz * 0.5) / (cutoff_hz * 0.5), 0.0, 1.0)
    return np.fft.irfft(spec * ramp, n=len(x))


def gas_leak(n: int, rng) -> np.ndarray:
    hiss = highpass(rng.standard_normal(n), 2500.0)
    t = np.arange(n) / RATE
    flutter = 0.8 + 0.2 * np.sin(2 * np.pi * 0.7 * t + rng.uniform(0, 6.28))
    drift = 0.9 + 0.1 * np.sin(2 * np.pi * 0.13 * t)
    return 0.55 * hiss / np.max(np.abs(hiss)) * flutter * drift


def bearing(n: int, rng) -> np.ndarray:
    t = np.arange(n) / RATE
    f0 = rng.uniform(110.0, 160.0)
    fm = 1.0 + 0.004 * np.sin(2 * np.pi * 1.7 * t)
    x = np.zeros(n)
    for k, amp in ((1, 0.5), (2, 0.30), (3, 0.22), (4, 0.15), (6, 0.10), (8, 0.07)):
        x += amp * np.sin(2 * np.pi * f0 * k * fm * t + rng.uniform(0, 6.28))
    reso = rng.uniform(2200.0, 3400.0)
    x += 0.25 * np.sin(2 * np.pi * reso * t) * (0.7 + 0.3 * np.sin(2 * np.pi * 2.3 * t))
    x += 0.05 * rng.standard_normal(n)
    return 0.6 * x / np.max(np.abs(x))


def arc(n: int, rng) -> np.ndarray:
    x = 0.02 * highpass(rng.standard_normal(n), 4000.0)
    # Dense Poisson crackle: every ~2.7s analysis window must contain bursts,
    # otherwise windows between crackles look like "impact" tails or silence.
    count = int(n / RATE * rng.uniform(50, 70))
    for _ in range(count):
        pos = rng.integers(0, n - 400)
        length = rng.integers(60, 380)
        burst = rng.standard_normal(length) * np.exp(-np.arange(length) / (length / 4.0))
        x[pos:pos + length] += burst * rng.uniform(0.5, 1.0)
    # Moderate HF sizzle bed between crackles: enough to separate arc from
    # the low-band knock decay of "impact", but weak enough that the crackle
    # bursts (not the hiss) dominate - keeps it separable from gas_leak.
    x += 0.15 * highpass(rng.standard_normal(n), 6000.0)
    return 0.7 * x / np.max(np.abs(x))


def impact(n: int, rng) -> np.ndarray:
    x = 0.01 * rng.standard_normal(n)
    t_hit = 0
    while t_hit < n - RATE // 2:
        t_hit += int(RATE * rng.uniform(0.35, 0.9))
        if t_hit >= n - RATE // 2:
            break
        length = int(RATE * rng.uniform(0.08, 0.16))
        tt = np.arange(length) / RATE
        f = rng.uniform(180.0, 420.0)
        knock = np.sin(2 * np.pi * f * tt) * np.exp(-tt * 35.0)
        knock += 0.6 * rng.standard_normal(length) * np.exp(-tt * 90.0)
        x[t_hit:t_hit + length] += knock * rng.uniform(0.7, 1.0)
    return 0.8 * x / np.max(np.abs(x))


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", default="_debug_logs/npu_sounds")
    parser.add_argument("--seconds", type=int, default=70)
    args = parser.parse_args()

    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)
    n = args.seconds * RATE
    rng = np.random.default_rng(7)

    for name, fn in (("gas_leak", gas_leak), ("bearing", bearing),
                     ("arc", arc), ("impact", impact)):
        write_wav(out / f"{name}.wav", fn(n, rng))
        print(f"{name}.wav written")


if __name__ == "__main__":
    main()
