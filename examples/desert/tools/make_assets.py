#!/usr/bin/env python3
"""Generates the PNG images used by the desert example (original art, CC0).

    python3 tools/make_assets.py

Everything is drawn with a tiny polygon rasterizer (4x supersampled) so the monster truck and the
sand worm can be rotated. MT2D treats pure cyan (0,255,255) as the transparent color.
Only the Python standard library is needed.
"""
import math
import os
import random
import struct
import zlib

KEY = (0, 255, 255)
OUT = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets")
SS = 4  # supersampling


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


class Canvas:
    """A w x h picture drawn at SS times the size. None = transparent."""

    def __init__(self, w, h):
        self.w, self.h = w, h
        self.W, self.H = w * SS, h * SS
        self.px = [[None] * self.W for _ in range(self.H)]

    def poly(self, pts, color):
        pts = [(x * SS, y * SS) for x, y in pts]
        n = len(pts)
        ymin = max(0, int(min(p[1] for p in pts)))
        ymax = min(self.H - 1, int(max(p[1] for p in pts)))
        for y in range(ymin, ymax + 1):
            yc = y + 0.5
            xs = []
            for i in range(n):
                x0, y0 = pts[i]
                x1, y1 = pts[(i + 1) % n]
                if (y0 <= yc < y1) or (y1 <= yc < y0):
                    xs.append(x0 + (yc - y0) * (x1 - x0) / (y1 - y0))
            xs.sort()
            row = self.px[y]
            for i in range(0, len(xs) - 1, 2):
                for x in range(max(0, int(math.ceil(xs[i] - 0.5))), min(self.W, int(math.ceil(xs[i + 1] - 0.5)))):
                    row[x] = color

    def circle(self, cx, cy, r, color):
        self.ellipse(cx, cy, r, r, 0, color)

    def ellipse(self, cx, cy, rx, ry, rot, color, steps=28):
        c, s = math.cos(rot), math.sin(rot)
        pts = []
        for i in range(steps):
            a = 2 * math.pi * i / steps
            x, y = rx * math.cos(a), ry * math.sin(a)
            pts.append((cx + x * c - y * s, cy + x * s + y * c))
        self.poly(pts, color)

    def rect(self, x0, y0, x1, y1, color):
        self.poly([(x0, y0), (x1, y0), (x1, y1), (x0, y1)], color)

    def line(self, x0, y0, x1, y1, width, color):
        dx, dy = x1 - x0, y1 - y0
        d = math.hypot(dx, dy) or 1
        nx, ny = -dy / d * width / 2, dx / d * width / 2
        self.poly([(x0 + nx, y0 + ny), (x1 + nx, y1 + ny), (x1 - nx, y1 - ny), (x0 - nx, y0 - ny)], color)

    def pixels(self):
        out = []
        for y in range(self.h):
            for x in range(self.w):
                acc, n = [0, 0, 0], 0
                for j in range(SS):
                    row = self.px[y * SS + j]
                    for i in range(SS):
                        c = row[x * SS + i]
                        if c is not None:
                            acc[0] += c[0]
                            acc[1] += c[1]
                            acc[2] += c[2]
                            n += 1
                out.append(KEY if n * 2 < SS * SS else (acc[0] // n, acc[1] // n, acc[2] // n))
        return out

    def save(self, name):
        write_png(os.path.join(OUT, name + ".png"), self.w, self.h, self.pixels())


def rot(pts, ang, cx, cy):
    c, s = math.cos(ang), math.sin(ang)
    return [(cx + (x - cx) * c - (y - cy) * s, cy + (x - cx) * s + (y - cy) * c) for x, y in pts]


# ---------------------------------------------------------------------------------------------
# Monster truck (64 x 44, faces right, the tires touch the bottom row)
# ---------------------------------------------------------------------------------------------
TRUCK_W, TRUCK_H = 64, 44
RED, DARK_RED = (214, 44, 38), (150, 26, 30)
YELLOW, ORANGE = (255, 214, 60), (255, 130, 30)
BLACK, GRAY, LGRAY = (24, 24, 30), (120, 124, 136), (190, 194, 204)
GLASS = (130, 200, 235)


def draw_truck(angle=0.0, bounce=0, wheel=0.0, flame=False):
    c = Canvas(TRUCK_W, TRUCK_H)
    cx, cy = 32, 38

    def T(pts):
        return rot([(x, y + bounce) for x, y in pts], angle, cx, cy)

    wheels = [(14, 32), (50, 32)]
    # suspension and chassis
    c.poly(T([(8, 21), (56, 21), (56, 25), (8, 25)]), BLACK)
    for wx, wy in wheels:
        (ax, ay), = T([(wx, wy)])
        top = T([(wx, 22)])[0]
        c.line(top[0], top[1], ax, ay, 2.4, LGRAY)
        c.line(top[0] - 3, top[1] + 1, ax, ay, 1.0, GRAY)
        c.line(top[0] + 3, top[1] + 1, ax, ay, 1.0, GRAY)
    # body
    c.poly(T([(6, 20), (6, 12), (20, 11), (26, 5), (45, 5), (50, 11), (60, 13), (60, 20)]), RED)
    c.poly(T([(6, 17), (60, 17), (60, 20), (6, 20)]), DARK_RED)
    c.poly(T([(8, 14), (58, 14), (58, 16), (8, 16)]), YELLOW)  # stripe
    # cab window
    c.poly(T([(27, 7), (44, 7), (47, 11), (25, 11)]), GLASS)
    c.line(*T([(36, 7)])[0], *T([(36, 11)])[0], 1.0, DARK_RED)
    # roof light bar and exhaust stacks
    c.poly(T([(30, 3), (42, 3), (42, 5), (30, 5)]), YELLOW)
    c.line(*T([(11, 12)])[0], *T([(11, 5)])[0], 2.2, LGRAY)
    c.line(*T([(15, 12)])[0], *T([(15, 5)])[0], 2.2, LGRAY)
    if flame:
        for fx in (11, 15):
            p = T([(fx, 5)])[0]
            c.poly([(p[0] - 1.5, p[1]), (p[0], p[1] - 4.5), (p[0] + 1.5, p[1])], ORANGE)
    # headlight and bumper
    c.circle(*T([(58, 14)])[0], 1.7, (255, 250, 200))
    c.poly(T([(58, 18), (63, 19), (63, 21), (58, 21)]), LGRAY)
    # giant tires
    for wx, wy in wheels:
        x, y = T([(wx, wy)])[0]
        c.circle(x, y, 11, BLACK)
        c.circle(x, y, 6.6, GRAY)
        for k in range(5):  # knobby tread
            a = wheel + k * 2 * math.pi / 5
            c.circle(x + 10.2 * math.cos(a), y + 10.2 * math.sin(a), 1.7, (60, 60, 66))
        for k in range(4):  # spokes show the rotation
            a = wheel + k * math.pi / 2
            c.line(x, y, x + 6 * math.cos(a), y + 6 * math.sin(a), 1.4, LGRAY)
        c.circle(x, y, 1.8, BLACK)
    return c


def make_truck():
    draw_truck(0, 0, 0.0).save("truck0")
    draw_truck(0, 1, 0.6, True).save("truck1")
    draw_truck(0, 0, 1.2).save("truck2")
    # drifting toward the camera (nose goes down) and away from it, then the counter steer
    draw_truck(math.radians(8), 0, 0.5).save("drift_d1")
    draw_truck(math.radians(14), 1, 1.0, True).save("drift_d2")
    draw_truck(math.radians(-8), 0, 0.5).save("drift_u1")
    draw_truck(math.radians(-14), 1, 1.0, True).save("drift_u2")


# ---------------------------------------------------------------------------------------------
# Sand worm: head (40 x 32) at three angles, body segment, sand mound
# ---------------------------------------------------------------------------------------------
HEAD_W, HEAD_H = 40, 32
SKIN, SKIN_DARK, SKIN_LIGHT = (176, 128, 84), (122, 80, 52), (214, 170, 120)
MAW, TOOTH = (78, 20, 34), (246, 240, 220)


def draw_head(angle, open_maw=True):
    c = Canvas(HEAD_W, HEAD_H)
    cx, cy = 22, 16
    # neck ring behind the head
    c.circle(cx - 9 * math.cos(angle), cy - 9 * math.sin(angle), 10, SKIN_DARK)
    c.circle(cx, cy, 13.5, SKIN)
    c.circle(cx - 3, cy - 4, 6, SKIN_LIGHT)
    # the round mouth looks along the angle (an ellipse seen from the side)
    mx, my = cx + 7 * math.cos(angle), cy + 7 * math.sin(angle)
    rx = 5.0 if open_maw else 2.5
    c.ellipse(mx, my, rx, 11.5, angle, MAW)
    c.ellipse(mx + 1.5 * math.cos(angle), my + 1.5 * math.sin(angle), rx * 0.45, 6.5, angle, (30, 4, 14))
    # teeth around the rim, pointing to the middle
    for k in range(10):
        a = 2 * math.pi * k / 10
        ex, ey = rx * math.cos(a), 11.5 * math.sin(a)
        ix, iy = ex * 0.45, ey * 0.45
        px_, py_ = ex - ey * 0.12, ey + ex * 0.12
        qx, qy = ex + ey * 0.12, ey - ex * 0.12
        pts = [(px_, py_), (qx, qy), (ix, iy)]
        pts = rot([(mx + x, my + y) for x, y in pts], angle, mx, my)
        c.poly(pts, TOOTH)
    return c


def make_worm():
    draw_head(math.radians(-55)).save("head_up")
    draw_head(math.radians(0)).save("head_mid")
    draw_head(math.radians(60)).save("head_down")
    # body segment: tight 24 x 24 images, the script spawns them at the head's position + (3, 4)
    c = Canvas(24, 24)
    c.circle(12, 12, 10.5, SKIN_DARK)
    c.circle(12, 12, 9, SKIN)
    c.circle(10, 9, 5.5, SKIN_LIGHT)
    c.save("seg_big")
    c = Canvas(24, 24)
    c.circle(12, 13, 7, SKIN_DARK)
    c.circle(12, 13, 5.8, SKIN)
    c.save("seg_small")
    # the mound the worm makes while it hunts under the sand (40 x 32, sits on the bottom)
    for i, (h, w) in enumerate(((7, 17), (9, 19))):
        c = Canvas(HEAD_W, HEAD_H)
        base = 29
        pts = [(2, base)]
        for k in range(21):
            x = 2 + k * 36 / 20
            u = (x - 20) / w
            pts.append((x, base - h * max(0.0, 1 - u * u)))
        pts.append((38, base))
        c.poly(pts, (214, 170, 110))
        pts2 = [(p[0], p[1] + 1.5) for p in pts[1:-1]]
        c.poly([(2, base)] + pts2 + [(38, base)], (232, 192, 130))
        c.rect(0, base, HEAD_W, HEAD_H, (196, 150, 92))
        c.save("mound%d" % i)


# ---------------------------------------------------------------------------------------------
# Particles
# ---------------------------------------------------------------------------------------------
def make_particles():
    # 24 x 24 puffs, the scripts place them at the tires
    for name, col, shade in (("dust", (222, 184, 124), (200, 160, 100)), ("smoke", (240, 240, 244), (176, 176, 186))):
        for i, r in enumerate((3, 5.5, 8, 10.5)):
            c = Canvas(24, 24)
            cx, cy = 9 - i * 0.6, 18 - i * 1.5
            c.circle(cx + 1.5, cy + 1.5, r, shade)
            c.circle(cx - r * 0.2, cy - r * 0.4, r * 0.7 if i < 3 else r * 0.6, col)
            c.save("%s%d" % (name, i))
    for i, r in enumerate((2.2, 1.4)):
        c = Canvas(6, 6)
        c.circle(3, 3, r, (232, 192, 130))
        c.save("grain%d" % i)


# ---------------------------------------------------------------------------------------------
# Background layers (all 320 wide and seamless when they scroll)
# ---------------------------------------------------------------------------------------------
W = 320


def lerp(a, b, t):
    return tuple(int(a[i] + (b[i] - a[i]) * t) for i in range(3))


def make_sky():
    h = 120
    top, mid, hor = (52, 36, 96), (214, 92, 84), (255, 196, 108)
    px = []
    sun = (222, 80)
    for y in range(h):
        t = y / (h - 1)
        col = lerp(top, mid, t / 0.62) if t < 0.62 else lerp(mid, hor, (t - 0.62) / 0.38)
        for x in range(W):
            d = math.hypot(x - sun[0], (y - sun[1]) * 1.0)
            c = col
            if d < 34:
                c = (255, 244, 200)
            elif d < 40:
                c = lerp(col, (255, 220, 150), 0.6)
            elif d < 60:
                c = lerp(col, (255, 200, 130), 0.28 * (1 - (d - 40) / 20))
            px.append(c)
    # a few stars in the dark part and long thin clouds
    rnd = random.Random(7)
    for _ in range(28):
        x, y = rnd.randrange(W), rnd.randrange(0, 34)
        px[y * W + x] = (255, 250, 235)
    for cxx, cyy, cw in ((40, 36, 60), (130, 52, 78), (270, 30, 50)):
        for x in range(cxx - cw, cxx + cw):
            if 0 <= x < W:
                t = 1 - abs(x - cxx) / cw
                for dy in range(-1, 2):
                    y = cyy + dy
                    px[y * W + x] = lerp(px[y * W + x], (255, 170, 130), 0.5 * t)
    write_png(os.path.join(OUT, "sky.png"), W, h, px)


def ridge(width, base_h, amp, seed, color, flat=False):
    """Silhouette heights that wrap around at the image edges."""
    rnd = random.Random(seed)
    harm = [(rnd.uniform(0.5, 1.0), rnd.randrange(1, 5), rnd.uniform(0, 6.28)) for _ in range(3)]
    hs = []
    for x in range(width):
        v = sum(a * math.sin(2 * math.pi * k * x / width + p) for a, k, p in harm) / 3
        if flat:
            v = max(-0.15, min(0.15, v * 0.5)) * 3
        hs.append(base_h + amp * v)
    return hs


def make_mesas():
    h = 52
    px = [KEY] * (W * h)
    rnd = random.Random(3)
    mesas = [(30, 70, 34), (130, 50, 26), (210, 80, 40), (290, 56, 30)]  # center, width, height
    for mc, mw, mh in mesas:
        for x in range(mc - mw // 2 - 8, mc + mw // 2 + 8):
            edge = max(0, abs(x - mc) - mw // 2)
            top = h - mh + edge * 1.6
            for y in range(int(top), h):
                col = lerp((120, 62, 90), (150, 78, 96), (y - top) / max(1, mh))
                px[y * W + (x % W)] = col
    # fill the bottom with a haze
    for y in range(h - 6, h):
        for x in range(W):
            if px[y * W + x] == KEY:
                px[y * W + x] = (170, 100, 100)
    write_png(os.path.join(OUT, "mesas.png"), W, h, px)


def make_dunes():
    h = 44
    px = [KEY] * (W * h)
    top = ridge(W, 16, 9, 11, None)
    for x in range(W):
        for y in range(h):
            if y >= top[x]:
                t = (y - top[x]) / (h - top[x])
                px[y * W + x] = lerp((216, 128, 86), (236, 170, 106), min(1, t * 1.4))
    # lit crest
    for x in range(W):
        y = int(top[x])
        if 0 <= y < h:
            px[y * W + x] = (255, 214, 150)
    write_png(os.path.join(OUT, "dunes.png"), W, h, px)


def make_ground():
    h = 120
    px = []
    rnd = random.Random(21)
    base_top, base_bot = (238, 190, 120), (214, 150, 88)
    # random ripples made of sine waves with whole periods so the picture wraps
    ripples = [(rnd.uniform(0, 6.28), rnd.randrange(2, 7), rnd.uniform(0.4, 1.0)) for _ in range(5)]
    for y in range(h):
        t = y / (h - 1)
        base = lerp(base_top, base_bot, t)
        # the ripples get longer and fatter near the camera
        for x in range(W):
            v = 0.0
            for ph, k, a in ripples:
                v += a * math.sin(2 * math.pi * k * x / W + ph + y * 0.35 * (1 + t))
            v = v / len(ripples)
            c = base
            if v > 0.28 + 0.1 * t:
                c = lerp(base, (255, 224, 160), 0.4)
            elif v < -0.3:
                c = lerp(base, (170, 104, 66), 0.35)
            px.append(c)
    # tire tracks from far to near would be wrong when the truck changes lane: use grains only
    for _ in range(420):
        x, y = rnd.randrange(W), rnd.randrange(h)
        px[y * W + x] = lerp(px[y * W + x], (120, 74, 48), 0.45)
    write_png(os.path.join(OUT, "ground.png"), W, h, px)


def make_foreground():
    """Rocks and cacti that rush by in front of everything (transparent, wraps)."""
    h = 40
    c = Canvas(W, h)
    stone, stone_l = (118, 84, 78), (156, 116, 100)
    for rx_, rw, rh in ((34, 18, 9), (176, 24, 12), (262, 12, 6)):
        c.poly([(rx_ - rw, h), (rx_ - rw * 0.8, h - rh * 0.6), (rx_ - rw * 0.2, h - rh), (rx_ + rw * 0.5, h - rh * 0.8),
                (rx_ + rw, h - rh * 0.2), (rx_ + rw * 1.1, h)], stone)
        c.poly([(rx_ - rw * 0.7, h - rh * 0.55), (rx_ - rw * 0.2, h - rh), (rx_ + rw * 0.2, h - rh * 0.9), (rx_ - rw * 0.2, h - rh * 0.4)], stone_l)
    cactus, cactus_l = (60, 120, 72), (96, 164, 100)
    for kx, kh in ((100, 34), (228, 28)):
        c.rect(kx - 3, h - kh, kx + 3, h, cactus)
        c.circle(kx, h - kh, 3, cactus)
        c.rect(kx - 10, h - kh * 0.62, kx - 3, h - kh * 0.62 + 3.5, cactus)
        c.rect(kx - 10, h - kh * 0.62 - 9, kx - 6.5, h - kh * 0.62 + 3.5, cactus)
        c.rect(kx + 3, h - kh * 0.45, kx + 10, h - kh * 0.45 + 3.5, cactus)
        c.rect(kx + 6.5, h - kh * 0.45 - 8, kx + 10, h - kh * 0.45 + 3.5, cactus)
        c.rect(kx - 1.5, h - kh + 1, kx, h, cactus_l)
    c.save("foreground")


FONT = {
    "A": ["01110", "10001", "10001", "11111", "10001", "10001", "10001"],
    "C": ["01111", "10000", "10000", "10000", "10000", "10000", "01111"],
    "D": ["11110", "10001", "10001", "10001", "10001", "10001", "11110"],
    "E": ["11111", "10000", "10000", "11110", "10000", "10000", "11111"],
    "K": ["10001", "10010", "10100", "11000", "10100", "10010", "10001"],
    "L": ["10000", "10000", "10000", "10000", "10000", "10000", "11111"],
    "M": ["10001", "11011", "10101", "10101", "10001", "10001", "10001"],
    "N": ["10001", "11001", "10101", "10011", "10001", "10001", "10001"],
    "O": ["01110", "10001", "10001", "10001", "10001", "10001", "01110"],
    "R": ["11110", "10001", "10001", "11110", "10100", "10010", "10001"],
    "S": ["01111", "10000", "10000", "01110", "00001", "00001", "11110"],
    "T": ["11111", "00100", "00100", "00100", "00100", "00100", "00100"],
    "U": ["10001", "10001", "10001", "10001", "10001", "10001", "01110"],
    "V": ["10001", "10001", "10001", "10001", "10001", "01010", "00100"],
    "W": ["10001", "10001", "10001", "10101", "10101", "11011", "10001"],
    "!": ["00100", "00100", "00100", "00100", "00100", "00000", "00100"],
    " ": ["00000"] * 7,
}


def make_text(name, text, scale, color, shade, outline=(30, 16, 36)):
    """A baked 5x7 pixel font text with a drop shadow and an outline."""
    w = len(text) * 6 * scale + 4
    h = 7 * scale + 4
    fill, shadow = set(), set()
    for i, ch in enumerate(text):
        for y, row in enumerate(FONT[ch]):
            for x, bit in enumerate(row):
                if bit == "1":
                    for dy in range(scale):
                        for dx in range(scale):
                            px_, py_ = 1 + i * 6 * scale + x * scale + dx, 1 + y * scale + dy
                            fill.add((px_, py_))
                            shadow.add((px_ + 1, py_ + 2))
    out = [KEY] * (w * h)
    ring = set()
    for (x, y) in fill | shadow:
        for dx in (-1, 0, 1):
            for dy in (-1, 0, 1):
                ring.add((x + dx, y + dy))
    for (x, y) in ring:
        if 0 <= x < w and 0 <= y < h:
            out[y * w + x] = outline
    for (x, y) in shadow:
        if 0 <= x < w and 0 <= y < h:
            out[y * w + x] = shade
    for (x, y) in fill:
        # a little gradient from top to bottom
        out[y * w + x] = lerp(color, (255, 255, 255), 0.35) if y < 1 + 2 * scale else color
    write_png(os.path.join(OUT, name + ".png"), w, h, out)
    return w, h


def make_texts():
    print("title", make_text("title", "TRUCK VS SANDWORM", 2, (255, 196, 60), (150, 40, 40)))
    print("alert", make_text("alert", "WORM ALERT!", 2, (255, 70, 60), (90, 10, 30)))


def main():
    os.makedirs(OUT, exist_ok=True)
    make_sky()
    make_mesas()
    make_dunes()
    make_ground()
    make_foreground()
    make_truck()
    make_worm()
    make_particles()
    make_texts()


if __name__ == "__main__":
    main()
