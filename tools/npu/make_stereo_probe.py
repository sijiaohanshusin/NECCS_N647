"""Generate gentle stereo localization probes: smooth band-limited noise
(1.5-4 kHz, soft 4 Hz tremolo) placed in ONE channel, silence in the other.
Amplitude kept low (0.22) so system volume can stay untouched.

Usage: python make_stereo_probe.py --out _debug_logs\\npu_sounds
Writes probe_left.wav, probe_right.wav, probe_alternate.wav (4 s L / 4 s R).
"""

import argparse
import wave
from pathlib import Path

import numpy as np

RATE = 48000


def soft_noise(n: int, rng) -> np.ndarray:
    spec = np.fft.rfft(rng.standard_normal(n))
    freqs = np.fft.rfftfreq(n, 1.0 / RATE)
    band = np.clip((freqs - 1200.0) / 600.0, 0.0, 1.0) * np.clip((4500.0 - freqs) / 800.0, 0.0, 1.0)
    x = np.fft.irfft(spec * band, n=n)
    x /= np.max(np.abs(x))
    t = np.arange(n) / RATE
    tremolo = 0.75 + 0.25 * np.sin(2.0 * np.pi * 4.0 * t)
    # Same loudness class as the (accepted) classifier sounds; smooth spectrum
    # keeps it non-piercing at this level.
    return x * tremolo * 0.55


def write_stereo(path: Path, left: np.ndarray, right: np.ndarray) -> None:
    pcm = np.empty(left.size * 2, dtype=np.int16)
    pcm[0::2] = (np.clip(left, -1, 1) * 32000).astype(np.int16)
    pcm[1::2] = (np.clip(right, -1, 1) * 32000).astype(np.int16)
    with wave.open(str(path), "wb") as w:
        w.setnchannels(2)
        w.setsampwidth(2)
        w.setframerate(RATE)
        w.writeframes(pcm.tobytes())


def main() -> None:
    parser = argparse.ArgumentParser()
    parser.add_argument("--out", default="_debug_logs/npu_sounds")
    parser.add_argument("--seconds", type=int, default=24)
    args = parser.parse_args()
    out = Path(args.out)
    out.mkdir(parents=True, exist_ok=True)

    rng = np.random.default_rng(11)
    n = args.seconds * RATE
    sig = soft_noise(n, rng)
    zero = np.zeros(n)

    write_stereo(out / "probe_left.wav", sig, zero)
    write_stereo(out / "probe_right.wav", zero, sig)

    # alternate: 4 s left, 4 s right, repeated
    seg = 4 * RATE
    left = sig.copy()
    right = sig.copy()
    for start in range(0, n, seg):
        if (start // seg) % 2 == 0:
            right[start:start + seg] = 0.0
        else:
            left[start:start + seg] = 0.0
    write_stereo(out / "probe_alternate.wav", left, right)
    print("probe_left / probe_right / probe_alternate written")


if __name__ == "__main__":
    main()
