#!/usr/bin/env python3
"""Makes assets/gameover.png: the GAME OVER panel of the desert scene.

The text screen of MT2D is drawn under the images, so a message over a full screen scene has to be an image.
The letters are cut from the engine's own font sheet (examples/assets/msdos_fnt.png, 32x8 glyphs of 8x8, CP850 order).
Needs Pillow:  python3 examples/welcome/tools/make_banner.py
"""
import os
from PIL import Image

here = os.path.dirname(os.path.abspath(__file__))
font = Image.open(os.path.join(here, "..", "..", "assets", "msdos_fnt.png")).convert("L")
out = os.path.join(here, "..", "assets", "gameover.png")

def text(img, s, x, y, scale, color):
    for ch in s:
        i = ord(ch)
        g = font.crop(((i % 32) * 8, (i // 32) * 8, (i % 32) * 8 + 8, (i // 32) * 8 + 8))
        g = g.resize((8 * scale, 8 * scale), Image.NEAREST)
        img.paste(Image.new("RGB", g.size, color), (x, y), g.point(lambda v: 255 if v > 127 else 0))
        x += 8 * scale

W, H = 232, 64
img = Image.new("RGB", (W, H), (30, 16, 36))
for x in range(W):
    for y in (0, 1, H - 2, H - 1):
        img.putpixel((x, y), (255, 196, 60))
for y in range(H):
    for x in (0, 1, W - 2, W - 1):
        img.putpixel((x, y), (255, 196, 60))
text(img, "GAME OVER", (W - 9 * 24) // 2, 10, 3, (255, 70, 60))
text(img, "PRESS ENTER", (W - 11 * 16) // 2, 40, 2, (255, 255, 255))
img.save(out)
print("wrote", out)
