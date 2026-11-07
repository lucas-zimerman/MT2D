#!/usr/bin/env python3
"""Generates the PNG sprites used by the sprites example (original pixel art, CC0).

    python3 tools/make_assets.py

Every sprite is drawn with characters ('.' = transparent). MT2D treats pure cyan (0,255,255)
as the transparent color. Only the Python standard library is needed.
"""
import math
import os
import struct
import zlib

KEY = (0, 255, 255)
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets")


def write_png(path, w, h, px):
    raw = b"".join(b"\x00" + bytes(c for p in px[y * w:(y + 1) * w] for c in p) for y in range(h))

    def chunk(tag, data):
        body = tag + data
        return struct.pack(">I", len(data)) + body + struct.pack(">I", zlib.crc32(body) & 0xFFFFFFFF)

    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n")
        f.write(chunk(b"IHDR", struct.pack(">IIBBBBB", w, h, 8, 2, 0, 0, 0)))
        f.write(chunk(b"IDAT", zlib.compress(raw, 9)))
        f.write(chunk(b"IEND", b""))


def from_rows(name, rows, palette):
    assert len(rows) == 16, "%s: %d rows" % (name, len(rows))
    for r in rows:
        assert len(r) == 16, "%s: bad row %r" % (name, r)
    px = [KEY if ch == "." else palette[ch] for r in rows for ch in r]
    write_png(os.path.join(OUT, name + ".png"), 16, 16, px)


ROBOT_PALETTE = {"k": (25, 25, 40), "w": (225, 230, 240), "g": (140, 150, 170),
                 "b": (60, 200, 255), "r": (220, 60, 60)}
ROBOT_TOP = [
    "......kkkk......",
    "....kkwwwwkk....",
    "...kwwwwwwwwk...",
    "...kwbbbbbbwk...",
    "...kwbbbbbbwk...",
    "...kwwwwwwwwk...",
    "....kkkkkkkk....",
    "..kkrrrrrrrrkk..",
    ".kwwkggggggkwwk.",
    ".kwwkggrrggkwwk.",
    ".kwwkggggggkwwk.",
    "..kkkggggggkkk..",
]
ROBOT_LEGS_APART = ["....kggkkggk....", "...kggk..kggk...", "..kggk....kggk..", "..kkkk....kkkk.."]
ROBOT_LEGS_TOGETHER = ["....kggkkggk....", "....kggkkggk....", "....kggkkggk....", "...kkkkkkkkkk..."]

GHOST_PALETTE = {"w": (235, 235, 255), "k": (20, 20, 40), "p": (255, 150, 180)}
GHOST_TOP = [
    ".....wwwwww.....",
    "...wwwwwwwwww...",
    "..wwwwwwwwwwww..",
    ".wwwwwwwwwwwwww.",
    ".wwkkwwwwwwkkww.",
    ".wwkkwwwwwwkkww.",
    ".wwwwwwwwwwwwww.",
    ".wwwwwwwwwwwwww.",
    ".wwwpwwwwwwpwww.",
    ".wwwwwwwwwwwwww.",
    ".wwwwwwwwwwwwww.",
    ".wwwwwwwwwwwwww.",
    ".wwwwwwwwwwwwww.",
]
GHOST_SKIRT_A = [".wwwwwwwwwwwwww.", ".www.wwww.wwww..", ".w....w..w....w."]
GHOST_SKIRT_B = [".wwwwwwwwwwwwww.", "..wwww.ww.wwww..", "..w..w....w..w.."]


def coin(frame):
    px = [KEY] * 256
    rx = [7, 4, 1, 4][frame]  # spinning: horizontal radius per frame
    for y in range(16):
        for x in range(16):
            dx, dy = x - 7.5, y - 7.5
            if (dx / max(rx, 0.6)) ** 2 + (dy / 7.0) ** 2 <= 1.0:
                t = 1.0 - (dy + 7.5) / 22.0
                lo, hi = (120, 80, 0), (255, 220, 60)
                px[y * 16 + x] = tuple(int(lo[i] + (hi[i] - lo[i]) * t) for i in range(3))
    write_png(os.path.join(OUT, "coin%d.png" % frame), 16, 16, px)


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    from_rows("robot0", ROBOT_TOP + ROBOT_LEGS_APART, ROBOT_PALETTE)
    from_rows("robot1", ROBOT_TOP + ROBOT_LEGS_TOGETHER, ROBOT_PALETTE)
    from_rows("ghost0", GHOST_TOP + GHOST_SKIRT_A, GHOST_PALETTE)
    from_rows("ghost1", GHOST_TOP + GHOST_SKIRT_B, GHOST_PALETTE)
    for i in range(4):
        coin(i)
    print("done")
