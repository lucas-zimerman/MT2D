#!/usr/bin/env python3
"""Converts the PNG images of the examples to 8 bit BMP files for the MS-DOS VGA renderer (mode 13h).

    python3 examples/tools/png2dos.py examples/sprites examples/graphics ...

For each example folder, every assets/*.png becomes d/<name>.bmp, where <name> is the PNG name
cut to 8 characters (MS-DOS 8.3 file names), see mt2d_image() in examples/common/example.h.

All the images of one example share a single 256 color palette:
  * index 0  = transparent (cyan 0,255,255 pixels, the same color key the SDL backend uses)
  * index 16 = black and 31 = white, the colors the VGA text renderer draws with (17..30 = grays)
  * the other indexes hold the colors of the images (quantized when there are more than fit).
"""
import os
import struct
import sys
import glob

from PIL import Image


def pixels(im):
    """Pixels as a list of tuples (getdata is deprecated in new Pillow versions)."""
    return list(im.get_flattened_data()) if hasattr(im, "get_flattened_data") else list(im.getdata())

CYAN = (0, 255, 255)
RESERVED = {0: (255, 0, 255), 16: (0, 0, 0), 31: (255, 255, 255)}
for _i in range(17, 31):
    _g = (_i - 16) * 255 // 15
    RESERVED[_i] = (_g, _g, _g)


def dos_name(png_name):
    return os.path.splitext(png_name)[0][:8].lower() + ".bmp"


def build_palette(images):
    free = [i for i in range(256) if i not in RESERVED]
    colors = set()
    for im in images:
        colors.update(c for c in set(pixels(im)) if c != CYAN)
    colors -= set(RESERVED.values())
    colors = sorted(colors)
    if len(colors) > len(free):
        # quantize the pixels of all images at once
        total = sum(im.width * im.height for im in images)
        side = int(total ** 0.5) + 1
        mont = Image.new("RGB", (side, side), (0, 0, 0))
        mont.putdata([p for im in images for p in pixels(im) if p != CYAN])
        q = mont.quantize(colors=len(free), method=Image.Quantize.MEDIANCUT, dither=Image.Dither.NONE)
        pal = q.getpalette()[: 3 * len(free)]
        colors = [tuple(pal[i * 3: i * 3 + 3]) for i in range(len(free))]
    palette = [(0, 0, 0)] * 256
    for i, c in RESERVED.items():
        palette[i] = c
    for i, c in zip(free, colors):
        palette[i] = c
    return palette


def nearest_index(color, palette, cache):
    if color in cache:
        return cache[color]
    best, bd = 0, 1 << 30
    for i, p in enumerate(palette):
        if i == 0:
            continue  # reserved for transparency
        d = sum((a - b) ** 2 for a, b in zip(color, p))
        if d < bd:
            best, bd = i, d
            if d == 0:
                break
    cache[color] = best
    return best


def write_bmp(path, im, palette, cache):
    w, h = im.size
    px = pixels(im)
    pad = (4 - w % 4) % 4
    rows = []
    for y in range(h - 1, -1, -1):  # BMP rows are stored bottom up
        row = bytes(0 if px[y * w + x] == CYAN else nearest_index(px[y * w + x], palette, cache) for x in range(w))
        rows.append(row + b"\0" * pad)
    data = b"".join(rows)
    pal = b"".join(struct.pack("<BBBB", b, g, r, 0) for r, g, b in palette)
    off = 14 + 40 + len(pal)
    out = b"BM" + struct.pack("<IHHI", off + len(data), 0, 0, off)
    out += struct.pack("<IiiHHIIiiII", 40, w, h, 1, 8, 0, len(data), 2835, 2835, 256, 0)
    with open(path, "wb") as f:
        f.write(out + pal + data)


def convert(example):
    pngs = sorted(glob.glob(os.path.join(example, "assets", "*.png")))
    if not pngs:
        return
    names = [dos_name(os.path.basename(p)) for p in pngs]
    if len(set(names)) != len(names):
        raise SystemExit("%s: two images share the same 8.3 name: %s" % (example, names))
    images = [Image.open(p).convert("RGB") for p in pngs]
    palette = build_palette(images)
    outdir = os.path.join(example, "d")
    os.makedirs(outdir, exist_ok=True)
    cache = {}
    for name, im in zip(names, images):
        write_bmp(os.path.join(outdir, name), im, palette, cache)
    print("%s: %d images -> %s" % (example, len(pngs), outdir))


if __name__ == "__main__":
    for ex in sys.argv[1:]:
        convert(ex)
