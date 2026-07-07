"""Compose N647 LTDC framebuffer dumps into PNG screenshots.

Inputs (produced by capture_n647_screen.ps1 via GDB):
  fb_ui.bin   1024x600 RGB565 UI layer (LTDC Layer2, color-keyed)
  fb_cam.bin  640x480  RGB565 camera+overlay compose buffer (optional)

Outputs:
  <out>.png            composited screen (UI over camera through color key)
  <out>_ui.png         raw UI layer
  <out>_cam.png        raw camera layer (when present)
"""
import argparse
import os
import struct

from PIL import Image

SCREEN_W, SCREEN_H = 1024, 600
CAM_W, CAM_H = 640, 480
CAM_X0, CAM_Y0 = 192, 60
COLOR_KEY_565 = 0xF81F  # RGB888 0xFF00FF


def load_rgb565(path, width, height):
    with open(path, "rb") as f:
        data = f.read(width * height * 2)
    if len(data) < width * height * 2:
        raise ValueError("%s: expected %d bytes, got %d" % (path, width * height * 2, len(data)))
    vals = struct.unpack("<%dH" % (width * height), data)
    img = Image.new("RGB", (width, height))
    px = img.load()
    for i, v in enumerate(vals):
        r = ((v >> 11) & 0x1F) << 3
        g = ((v >> 5) & 0x3F) << 2
        b = (v & 0x1F) << 3
        px[i % width, i // width] = (r | (r >> 5), g | (g >> 6), b | (b >> 5))
    return img, vals


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--ui", required=True, help="UI framebuffer dump (1024x600 RGB565)")
    ap.add_argument("--cam", default=None, help="camera compose dump (640x480 RGB565), optional")
    ap.add_argument("--out", required=True, help="output PNG path (composited)")
    ap.add_argument("--keep-layers", action="store_true", help="also write _ui/_cam layer PNGs")
    args = ap.parse_args()

    ui_img, ui_vals = load_rgb565(args.ui, SCREEN_W, SCREEN_H)

    cam_img = None
    if args.cam and os.path.isfile(args.cam):
        cam_img, _ = load_rgb565(args.cam, CAM_W, CAM_H)

    comp = ui_img.copy()
    if cam_img is not None:
        cpx = comp.load()
        campx = cam_img.load()
        for y in range(SCREEN_H):
            row = y * SCREEN_W
            for x in range(SCREEN_W):
                if ui_vals[row + x] == COLOR_KEY_565:
                    cx, cy = x - CAM_X0, y - CAM_Y0
                    if 0 <= cx < CAM_W and 0 <= cy < CAM_H:
                        cpx[x, y] = campx[cx, cy]
                    else:
                        cpx[x, y] = (0, 0, 0)
    comp.save(args.out)

    if args.keep_layers:
        base, _ = os.path.splitext(args.out)
        ui_img.save(base + "_ui.png")
        if cam_img is not None:
            cam_img.save(base + "_cam.png")

    print("saved %s" % args.out)


if __name__ == "__main__":
    main()
