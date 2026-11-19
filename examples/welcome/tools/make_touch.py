#!/usr/bin/env python3
"""Makes the touch controls of the graphics screens: assets/t_*.png.

The space and desert scenes cover the text screen with images, and images are drawn on top of the text, so ASCII
touch controls would be hidden. These images are the same ASCII art (as in examples/graphics), drawn with the engine's
own font sheet (examples/assets/msdos_fnt.png, 32x8 glyphs of 8x8, CP850 order): white with a black outline, yellow
while pressed. Cyan (0, 255, 255) is the transparent color of the SDL backend.
Each character is 8x8 here, the program scales the images to the size of text cells (4 x 9.6 pixels of the 320x240 screen).
Needs Pillow:  python3 examples/welcome/tools/make_touch.py
"""
import os
from PIL import Image

here = os.path.dirname(os.path.abspath(__file__))
font = Image.open(os.path.join(here, "..", "..", "assets", "msdos_fnt.png")).convert("L")
out_dir = os.path.join(here, "..", "assets")

CYAN = (0, 255, 255)
BLACK = (0, 0, 0)
WHITE = (255, 255, 255)
YELLOW = (255, 196, 60)


def glyph_mask(ch):
    i = ord(ch)
    g = font.crop(((i % 32) * 8, (i // 32) * 8, (i % 32) * 8 + 8, (i // 32) * 8 + 8))
    return g.point(lambda v: 255 if v > 127 else 0)


def make(name, rows, color):
    """rows: the lines of ASCII art, all the same length"""
    w, h = len(rows[0]) * 8, len(rows) * 8
    mask = Image.new("L", (w + 2, h + 2), 0)  # 1 pixel of border for the outline
    for r, line in enumerate(rows):
        assert len(line) == len(rows[0]), name
        for c, ch in enumerate(line):
            if ch != " ":
                mask.paste(glyph_mask(ch), (1 + c * 8, 1 + r * 8))
    img = Image.new("RGB", mask.size, CYAN)
    px, m = img.load(), mask.load()
    for y in range(mask.size[1]):
        for x in range(mask.size[0]):
            if m[x, y]:
                for dy in (-1, 0, 1):
                    for dx in (-1, 0, 1):
                        if 0 <= x + dx < mask.size[0] and 0 <= y + dy < mask.size[1]:
                            px[x + dx, y + dy] = BLACK
    for y in range(mask.size[1]):
        for x in range(mask.size[0]):
            if m[x, y]:
                px[x, y] = color
    path = os.path.join(out_dir, name)
    img.save(path)
    print("wrote", path)


make("t_area.png", [".---------.",
                    "|         |",
                    "|         |",
                    "|         |",
                    "'---------'"], WHITE)
make("t_thumb0.png", ["(o)"], WHITE)
make("t_thumb1.png", ["(O)"], YELLOW)
make("t_esc0.png", [".-----.", "| ESC |", "'-----'"], WHITE)
make("t_esc1.png", [".=====.", "[ ESC ]", "'====='"], YELLOW)
make("t_retry0.png", [".-------.", "| RETRY |", "'-------'"], WHITE)
make("t_retry1.png", [".=======.", "[ RETRY ]", "'======='"], YELLOW)
