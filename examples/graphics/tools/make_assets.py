#!/usr/bin/env python3
"""Generates the PNG images used by the graphics example (original art, public domain / CC0).

    python3 tools/make_assets.py

MT2D treats pure cyan (0,255,255) as the transparent color.
Only the Python standard library is needed.
"""
import math
import os
import random
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


def canvas(w, h, color):
    return [color] * (w * h)


def put(px, w, h, x, y, c):
    if 0 <= x < w and 0 <= y < h:
        px[y * w + x] = c


def mix(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3))


def background(path, w, h, seed, density, tint):
    rnd = random.Random(seed)
    px = []
    for y in range(h):
        row = mix((4, 4, 24), (34, 12, 64), y / h)
        px.extend([row] * w)
    for _ in range(density):
        x, y = rnd.randrange(w), rnd.randrange(h)
        b = rnd.randrange(120, 256)
        c = mix((b, b, b), tint, 0.35)
        put(px, w, h, x, y, c)
        if rnd.random() < 0.15:  # bigger twinkling star
            for dx, dy in ((1, 0), (-1, 0), (0, 1), (0, -1)):
                put(px, w, h, x + dx, y + dy, mix(c, (0, 0, 0), 0.6))
    write_png(path, w, h, px)


def stars_layer(path, w, h, seed, density):
    """Transparent layer with only stars (used for the parallax)."""
    rnd = random.Random(seed)
    px = canvas(w, h, KEY)
    for _ in range(density):
        x, y = rnd.randrange(w), rnd.randrange(h)
        b = rnd.randrange(150, 256)
        put(px, w, h, x, y, (b, b, b))
    write_png(path, w, h, px)


def ship(path):
    w, h = 32, 16
    px = canvas(w, h, KEY)
    for x in range(w):
        # body: a pointy triangle pointing to the right
        half = max(0, int((w - x) * 0.28))
        for y in range(h // 2 - half, h // 2 + half + 1):
            t = (y - (h // 2 - half)) / max(1, 2 * half)
            put(px, w, h, x, y, mix((235, 235, 245), (110, 120, 150), t))
    for y in range(h // 2 - 2, h // 2 + 3):          # cockpit
        for x in range(18, 24):
            put(px, w, h, x, y, (60, 200, 255))
    for y in (2, 3, 12, 13):                         # wings
        for x in range(2, 12):
            put(px, w, h, x, y, (220, 60, 60))
    for y in range(h // 2 - 1, h // 2 + 2):          # engine flame
        for x in range(0, 4):
            put(px, w, h, x, y, (255, 190, 40))
    write_png(path, w, h, px)


def planet(path, size, base, ring):
    px = canvas(size, size, KEY)
    r = size / 2.0 - 1
    for y in range(size):
        for x in range(size):
            dx, dy = x - size / 2.0 + 0.5, y - size / 2.0 + 0.5
            d = math.hypot(dx, dy)
            if d <= r:
                light = max(0.0, 1.0 - math.hypot(dx + r * 0.4, dy + r * 0.4) / (r * 1.7))
                band = 0.12 * math.sin(dy * 0.9)
                c = mix((0, 0, 0), base, min(1.0, 0.25 + light + band))
                put(px, size, size, x, y, c)
    if ring:
        for x in range(size):
            for y in range(size):
                dx, dy = x - size / 2.0 + 0.5, (y - size / 2.0 + 0.5) * 3.2
                d = math.hypot(dx, dy)
                if r * 0.95 < d < r * 1.25 and not (y < size / 2.0 and math.hypot(dx, y - size / 2.0) < r):
                    put(px, size, size, x, y, mix((0, 0, 0), ring, 0.8))
    write_png(path, size, size, px)


def coin(path, frame):
    size = 16
    px = canvas(size, size, KEY)
    widths = [7, 4, 1, 4]            # spinning: horizontal radius per frame
    rx = widths[frame]
    for y in range(size):
        for x in range(size):
            dx, dy = x - 7.5, y - 7.5
            if (dx / max(rx, 0.6)) ** 2 + (dy / 7.0) ** 2 <= 1.0:
                shade = 1.0 - (dy + 7.5) / 22.0
                put(px, size, size, x, y, mix((120, 80, 0), (255, 220, 60), shade))
    write_png(path, size, size, px)


def rock(path):
    size = 24
    rnd = random.Random(7)
    px = canvas(size, size, KEY)
    radii = [9 + rnd.random() * 3 for _ in range(16)]
    for y in range(size):
        for x in range(size):
            dx, dy = x - 11.5, y - 11.5
            a = (math.atan2(dy, dx) + math.pi) / (2 * math.pi) * 16
            i = int(a) % 16
            r = radii[i] * (1 - (a - int(a))) + radii[(i + 1) % 16] * (a - int(a))
            if math.hypot(dx, dy) <= r:
                s = 0.45 + 0.35 * ((x + y) % 5 == 0) + 0.2 * (dx < 0)
                put(px, size, size, x, y, mix((0, 0, 0), (150, 130, 110), s))
    write_png(path, size, size, px)


if __name__ == "__main__":
    os.makedirs(OUT, exist_ok=True)
    stars_layer(os.path.join(OUT, "stars_far.png"), 320, 240, 2, 70)
    stars_layer(os.path.join(OUT, "stars_near.png"), 320, 240, 3, 35)
    ship(os.path.join(OUT, "ship.png"))
    planet(os.path.join(OUT, "planet_red.png"), 56, (220, 90, 60), None)
    planet(os.path.join(OUT, "planet_blue.png"), 40, (70, 130, 240), (230, 210, 150))
    rock(os.path.join(OUT, "rock.png"))
    for i in range(4):
        coin(os.path.join(OUT, "coin%d.png" % i), i)
    print("done")
