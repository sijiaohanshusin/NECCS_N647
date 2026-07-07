"""Generate TouchGFX brand image assets from the master NECCS logo.

Source: the transparent-background master logo (white artwork + blue dot).
Outputs (into Appli/TouchGFX/assets/images/):
  boot_emblem.png  emblem-only artwork for the boot screen (wordmark is
                   rendered with fonts instead, for crispness and RAM budget)
  brand_mark.png   emblem-only small mark for the status bar
  boot_ring.png    soft glowing ring, scaled by GPU2D for the sonar pulse
"""
import argparse
import math
import os

from PIL import Image

DEFAULT_SRC = r"D:\Project\NECCS\N647_BaseBoard\png透明底文件_已抠除背景\png透明底文件_已抠除背景\作品主logo.png"
DEFAULT_OUT = r"D:\Project\NECCS\Program\NECCS_N647\NECCS_N647_App\Appli\TouchGFX\assets\images"


def content_bbox(img, alpha_min=8):
    alpha = img.getchannel("A")
    mask = alpha.point(lambda a: 255 if a >= alpha_min else 0)
    return mask.getbbox()


def emblem_art(src):
    # The wordmark occupies the lower ~28% of the artwork; keep the graphic.
    art = src.crop(content_bbox(src))
    emblem = art.crop((0, 0, art.width, round(art.height * 0.72)))
    return emblem.crop(content_bbox(emblem))


def save_quantized(img, path):
    """Save as palette PNG (<=255 colors + alpha) so TouchGFX can use
    L8_ARGB8888 (1 byte/pixel) instead of ARGB8888 (4 bytes/pixel)."""
    quant = img.quantize(colors=255, method=Image.FASTOCTREE)
    quant.save(path)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--src", default=DEFAULT_SRC)
    ap.add_argument("--out-dir", default=DEFAULT_OUT)
    ap.add_argument("--emblem-width", type=int, default=200)
    ap.add_argument("--mark-height", type=int, default=32)
    args = ap.parse_args()

    src = Image.open(args.src).convert("RGBA")
    emblem = emblem_art(src)

    scale = args.emblem_width / emblem.width
    boot = emblem.resize((args.emblem_width, max(1, round(emblem.height * scale))), Image.LANCZOS)
    boot_path = os.path.join(args.out_dir, "boot_emblem.png")
    boot.save(boot_path)
    print("boot_emblem: %dx%d -> %s" % (boot.width, boot.height, boot_path))

    scale = args.mark_height / emblem.height
    mark = emblem.resize((max(1, round(emblem.width * scale)), args.mark_height), Image.LANCZOS)
    mark_path = os.path.join(args.out_dir, "brand_mark.png")
    mark.save(mark_path)
    print("brand_mark: %dx%d -> %s" % (mark.width, mark.height, mark_path))

    # Sonar pulse ring: soft annulus, scaled at runtime.
    size = 120
    ring = Image.new("RGBA", (size, size), (61, 126, 255, 0))
    px = ring.load()
    centre = (size - 1) / 2.0
    core_radius = 52.0
    sigma = 4.5
    for y in range(size):
        for x in range(size):
            r = math.hypot(x - centre, y - centre)
            alpha = int(230.0 * math.exp(-((r - core_radius) ** 2) / (2.0 * sigma * sigma)))
            if alpha > 0:
                px[x, y] = (61, 126, 255, alpha)
    ring_path = os.path.join(args.out_dir, "boot_ring.png")
    ring.save(ring_path)
    print("boot_ring: %dx%d -> %s" % (size, size, ring_path))


if __name__ == "__main__":
    main()
