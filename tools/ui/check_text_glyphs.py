"""Verify every Chinese character used in UI sources exists in the
TouchGFX typography wildcard sets. A missing glyph renders as BLANK on
the device with no build error, so this must pass before any UI commit.

Usage:  python tools/ui/check_text_glyphs.py     (exit 0 = OK)
"""

import io
import re
import sys
from pathlib import Path

# Windows consoles default to GBK; force UTF-8 so glyph reports print.
sys.stdout = io.TextIOWrapper(sys.stdout.buffer, encoding="utf-8", errors="replace")

# No .resolve(): the repo is reachable through a directory junction and
# resolving would escape it to a stale path.
REPO = Path(__file__).absolute().parents[2]
TEXTS_XML = REPO / "NECCS_N647_App/Appli/TouchGFX/assets/texts/texts.xml"
UI_SOURCES = [
    REPO / "NECCS_N647_App/Appli/TouchGFX/gui/src",
]

CJK = re.compile(r"[\u3400-\u9fff\uf900-\ufaff\u3000-\u303f\uff00-\uffef·°µ]")


def collect_wildcards() -> set:
    xml = TEXTS_XML.read_text(encoding="utf-8")
    chars = set()
    for match in re.finditer(r'WidgetWildcardCharacters="([^"]*)"', xml):
        chars.update(match.group(1))
    for match in re.finditer(r'WildcardCharacters="([^"]*)"', xml):
        chars.update(match.group(1))
    return chars


def collect_used() -> dict:
    used = {}
    for root in UI_SOURCES:
        for path in root.rglob("*.cpp"):
            try:
                text = path.read_text(encoding="utf-8")
            except UnicodeDecodeError:
                text = path.read_text(encoding="gbk", errors="ignore")
            for lineno, line in enumerate(text.splitlines(), 1):
                stripped = line.strip()
                if stripped.startswith("//") or stripped.startswith("/*") or stripped.startswith("*"):
                    continue  # comments do not render
                for ch in CJK.findall(line):
                    used.setdefault(ch, []).append(f"{path.name}:{lineno}")
    return used


def main() -> int:
    wildcards = collect_wildcards()
    used = collect_used()
    missing = {ch: locs for ch, locs in used.items() if ch not in wildcards}

    print(f"wildcard set: {len(wildcards)} chars; UI sources use {len(used)} unique CJK chars")
    if not missing:
        print("OK - every rendered character has a glyph")
        return 0

    print(f"\nMISSING {len(missing)} glyph(s) - these render BLANK on device:")
    for ch, locs in sorted(missing.items()):
        print(f"  '{ch}' (U+{ord(ch):04X})  first use: {locs[0]}  (x{len(locs)})")
    print("\nFix: append the characters to every Typography's "
          "WidgetWildcardCharacters in assets/texts.xml, then rebuild.")
    return 1


if __name__ == "__main__":
    sys.exit(main())
