"""Collect labeled spectrum windows from the live board for NPU training.

The firmware keeps a rolling 32x64 int8 spectrum window in
`app_npu.c::s_spec_window`. This script polls it over the OpenOCD telnet
port while you make the target sound, and stores windows as .npy files.

Usage (OpenOCD must be running and the board executing the Debug build):

    python tools/npu/collect_spectra.py --label whistle --seconds 60
    python tools/npu/collect_spectra.py --label clap --seconds 60
    python tools/npu/collect_spectra.py --label speech --seconds 120
    python tools/npu/collect_spectra.py --label ambient --seconds 120

Samples land in tools/npu/dataset/<label>/NNNN.npy (shape (32, 64), int8).
Poll cadence ~1.5 s so consecutive windows overlap only partially.
"""

import argparse
import re
import socket
import subprocess
import time
from pathlib import Path

import numpy as np

NM = (r"C:\ST\STM32CubeIDE_2.1.1\STM32CubeIDE\plugins"
      r"\com.st.stm32cube.ide.mcu.externaltools.gnu-tools-for-stm32.14.3."
      r"rel1.win32_1.0.100.202602081740\tools\bin\arm-none-eabi-nm.exe")
ELF = (r"D:\Project\NECCS\Program\NECCS_N647\NECCS_N647_App\STM32CubeIDE"
       r"\Appli\Debug\NECCS_N647_App_Appli.elf")
WINDOW_BYTES = 32 * 64


def resolve_symbol(name: str) -> int:
    out = subprocess.check_output([NM, ELF], text=True)
    for line in out.splitlines():
        parts = line.split()
        if len(parts) == 3 and parts[2] == name:
            return int(parts[0], 16)
    raise SystemExit(f"symbol {name} not found in {ELF}")


class OpenOcdTelnet:
    def __init__(self, host="127.0.0.1", port=4444):
        self.sock = socket.create_connection((host, port), timeout=5)
        self.sock.settimeout(2)
        self._drain()

    def _drain(self) -> str:
        data = b""
        try:
            while True:
                chunk = self.sock.recv(65536)
                if not chunk:
                    break
                data += chunk
                if data.rstrip().endswith(b">"):
                    break
        except socket.timeout:
            pass
        return data.decode(errors="replace")

    def cmd(self, command: str) -> str:
        self.sock.sendall((command + "\n").encode())
        return self._drain()

    def read_block(self, addr: int, length: int) -> bytes:
        words = (length + 3) // 4
        resp = self.cmd(f"mdw 0x{addr:08x} {words}")
        data = bytearray()
        for match in re.finditer(r"0x[0-9a-f]{8}:((?:\s+[0-9a-f]{8})+)", resp):
            for word in match.group(1).split():
                data += int(word, 16).to_bytes(4, "little")
        if len(data) < length:
            raise RuntimeError(f"short read: {len(data)} < {length}")
        return bytes(data[:length])


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--label", required=True)
    parser.add_argument("--seconds", type=int, default=60)
    parser.add_argument("--period", type=float, default=1.5)
    args = parser.parse_args()

    window_addr = resolve_symbol("s_spec_window")
    feeds_addr = resolve_symbol("s_spec_feeds")
    out_dir = Path(__file__).parent / "dataset" / args.label
    out_dir.mkdir(parents=True, exist_ok=True)
    start_index = len(list(out_dir.glob("*.npy")))

    ocd = OpenOcdTelnet()
    print(f"window @0x{window_addr:08x}, feeds @0x{feeds_addr:08x}")
    print(f"collecting '{args.label}' for {args.seconds}s -> {out_dir}")

    saved = 0
    last_feeds = -1
    deadline = time.time() + args.seconds
    while time.time() < deadline:
        feeds = int.from_bytes(ocd.read_block(feeds_addr, 4), "little")
        if feeds >= 32 and feeds != last_feeds:
            last_feeds = feeds
            raw = ocd.read_block(window_addr, WINDOW_BYTES)
            window = np.frombuffer(raw, dtype=np.int8).reshape(32, 64)
            np.save(out_dir / f"{start_index + saved:04d}.npy", window)
            saved += 1
            print(f"\r{saved} windows (feeds={feeds})", end="", flush=True)
        time.sleep(args.period)

    print(f"\ndone: {saved} windows saved")


if __name__ == "__main__":
    main()
