#!/usr/bin/env python3
"""Draws the line icons in assets/icons: white strokes on transparent, for ImageView's templated mode.

Each icon is drawn on a 24-unit grid, then scaled so its artwork is ICON_SIZE pixels across at its widest,
and centered. The stroke stays STROKE pixels whatever the scale, so the set has one weight.
Strokes are distance fields, which gives round caps and joins and antialiased edges without any libraries.

    python3 tools/make_icons.py                 # writes every icon
    python3 tools/make_icons.py bell moon       # writes just these
    python3 tools/make_icons.py --sheet out.png # also writes all of them on black, side by side, to check
"""
import math
import os
import struct
import sys
import zlib

CANVAS = 128
STROKE = 10.0      # px, matching the existing icons
ICON_SIZE = 82.0   # px across the widest side of the artwork, stroke included
GAP = 5.0          # px of clear space where one stroke passes behind another

# --- Shapes, in grid units. Angles are in degrees and run clockwise on screen, with 0 pointing right.

def arc(cx, cy, r, a0, a1, step=2.0):
    n = max(2, int(abs(a1 - a0) / step) + 1)
    return [(cx + r * math.cos(math.radians(a0 + (a1 - a0) * i / (n - 1))),
             cy + r * math.sin(math.radians(a0 + (a1 - a0) * i / (n - 1)))) for i in range(n)]

def circle(cx, cy, r):
    return arc(cx, cy, r, 0, 360)

def rrect(x0, y0, x1, y1, r):
    return (arc(x1 - r, y0 + r, r, 270, 360) + arc(x1 - r, y1 - r, r, 0, 90)
            + arc(x0 + r, y1 - r, r, 90, 180) + arc(x0 + r, y0 + r, r, 180, 270) + [(x1 - r, y0)])

def arrowhead(tip, angle, length=3.0, spread=45):
    """A V whose point is at tip, pointing along angle."""
    back = math.radians(angle + 180)
    arm = lambda s: (tip[0] + length * math.cos(back + math.radians(s)), tip[1] + length * math.sin(back + math.radians(s)))
    return [arm(spread), tip, arm(-spread)]

def rotate(points, degrees, about=(12, 12)):
    c, s = math.cos(math.radians(degrees)), math.sin(math.radians(degrees))
    return [(about[0] + (x - about[0]) * c - (y - about[1]) * s, about[1] + (x - about[0]) * s + (y - about[1]) * c) for x, y in points]

def line(*points):
    return ("line", list(points))

def path(points):
    return ("line", points)

def dot(cx, cy, r):
    return ("dot", (cx, cy, r))

# --- Icons. Each returns layers: ("draw", shapes) adds strokes, ("cut", shapes) clears around them by GAP.

def calendar_page():
    return [path(rrect(4.5, 5.5, 19.5, 20, 2.5)), line((8.5, 3.75), (8.5, 7.25)), line((15.5, 3.75), (15.5, 7.25))]

def calendar_grid():
    dots = [dot(x, y, 0.9) for y in (9.75, 12.75, 15.75) for x in (8.5, 12, 15.5)]
    return [("draw", calendar_page() + dots)]

def today():
    return [("draw", calendar_page() + [dot(12, 12.75, 2.1)])]

def list_():
    shapes = []
    for y in (7, 12, 17):
        shapes += [line((9, y), (19.5, y)), dot(5, y, 1.2)]
    return [("draw", shapes)]

def flag():
    wave = lambda x: -0.9 * math.sin(2 * math.pi * (x - 6) / 12.5)
    xs = [6 + 12.5 * i / 40 for i in range(41)]
    top = [(x, 5 + wave(x)) for x in xs]
    bottom = [(x, 12.5 + wave(x)) for x in reversed(xs)]
    return [("draw", [line((6, 3.5), (6, 20.5)), path(top + bottom)])]

def repeat():
    top = [(5, 13.5)] + arc(8.5, 10.5, 3.5, 180, 270) + [(18.5, 7)]
    bottom = rotate(top, 180)
    return [("draw", [path(top), path(arrowhead((18.5, 7), 0)),
                      path(bottom), path(arrowhead(bottom[-1], 180))])]

def location_pin():
    cx, cy, r, tip = 12, 9.5, 6.25, (12, 20.5)
    side = math.degrees(math.acos(r / (tip[1] - cy)))
    outline = [tip] + arc(cx, cy, r, 90 + side, 450 - side) + [tip]
    return [("draw", [path(outline), path(circle(cx, cy, 2.4))])]

def bell():
    body = [(4.5, 17), (6, 15.5)] + arc(12, 10.5, 6, 180, 360) + [(18, 15.5), (19.5, 17), (4.5, 17)]
    return [("draw", [path(body), path(arc(12, 20.5, 2, 0, 180))])]

def note():
    return [("draw", [path(rrect(5.5, 3.5, 18.5, 20.5, 2.5)),
                      line((9, 8.5), (15, 8.5)), line((9, 12.5), (15, 12.5)), line((9, 16.5), (12.5, 16.5))])]

def person_shapes(cx, head_y, head_r, shoulder_y, shoulder_r, from_angle=180):
    return [path(circle(cx, head_y, head_r)), path(arc(cx, shoulder_y, shoulder_r, from_angle, 360))]

def person():
    return [("draw", person_shapes(12, 7.5, 3, 19.5, 6))]

def people():
    front = person_shapes(9.5, 8, 2.8, 19, 5.5)
    back = person_shapes(16, 6.5, 2.4, 17.5, 4.75, from_angle=240)
    return [("draw", back), ("cut", front), ("draw", front)]

def wifi_off():
    cx, cy = 12, 18.5
    arcs = [path(arc(cx, cy, r, 228, 312)) for r in (3.75, 7.5, 11.25)]
    slash = [line((5, 4.5), (19, 18.5))]
    return [("draw", arcs + [dot(cx, cy, 1.3)]), ("cut", slash), ("draw", slash)]

def sync_arrows(r=7.5):
    shapes = []
    for start in (200, 20):
        end = start + 130
        points = arc(12, 12, r, start, end)
        # The tangent going clockwise is a quarter turn past the point's own angle.
        shapes += [path(points), path(arrowhead(points[-1], end + 90))]
    return shapes

def sync():
    return [("draw", sync_arrows())]

def sync_error():
    return [("draw", sync_arrows() + [line((12, 8.75), (12, 12.5)), dot(12, 15.5, 1.15)])]

def settings():
    inner, outer, teeth = 6, 8, 6
    def radius(a):
        t = 0.5 + 0.5 * math.tanh(5 * (math.cos(math.radians(a * teeth)) - 0.15))
        return inner + (outer - inner) * t
    gear = [(12 + radius(a) * math.cos(math.radians(a)), 12 + radius(a) * math.sin(math.radians(a))) for a in range(0, 361)]
    return [("draw", [path(gear), path(circle(12, 12, 2.5))])]

def moon():
    (ox, oy, orad), (ix, iy, irad) = (12, 12, 8), (16.5, 8, 6.5)
    # Where the circles cross: the horns of the crescent.
    d = math.hypot(ix - ox, iy - oy)
    a = (orad ** 2 - irad ** 2 + d ** 2) / (2 * d)
    h = math.sqrt(orad ** 2 - a ** 2)
    mx, my = ox + a * (ix - ox) / d, oy + a * (iy - oy) / d
    p1 = (mx + h * (iy - oy) / d, my - h * (ix - ox) / d)
    p2 = (mx - h * (iy - oy) / d, my + h * (ix - ox) / d)
    angle = lambda c, p: math.degrees(math.atan2(p[1] - c[1], p[0] - c[0]))
    def side(c, r, p, q, keep):
        a0, a1 = angle(c, p), angle(c, q)
        for end in (a1, a1 + 360, a1 - 360):
            mid = math.radians((a0 + end) / 2)
            if abs(end - a0) < 360 and keep((c[0] + r * math.cos(mid), c[1] + r * math.sin(mid))):
                return arc(c[0], c[1], r, a0, end)
    outside_inner = lambda p: math.hypot(p[0] - ix, p[1] - iy) > irad
    inside_outer = lambda p: math.hypot(p[0] - ox, p[1] - oy) < orad
    outline = side((ox, oy), orad, p1, p2, outside_inner) + side((ix, iy), irad, p2, p1, inside_outer)
    return [("draw", [path(outline)])]

def sun():
    rays = [line((12 + 6.25 * math.cos(math.radians(a)), 12 + 6.25 * math.sin(math.radians(a))),
                 (12 + 8.25 * math.cos(math.radians(a)), 12 + 8.25 * math.sin(math.radians(a)))) for a in range(0, 360, 45)]
    return [("draw", [path(circle(12, 12, 3.75))] + rays)]

def edit():
    # Drawn along x with the tip at the origin, then turned so the tip is bottom left.
    body = [(0, 0), (4, -2.5), (16, -2.5), (16, 2.5), (4, 2.5), (0, 0)]
    band = [(12.5, -2.5), (12.5, 2.5)]
    turn = lambda pts: rotate([(x + 4, y + 12) for x, y in pts], -45, about=(12, 12))
    return [("draw", [path(turn(body)), path(turn(band))])]

def trash():
    return [("draw", [line((4.5, 6.5), (19.5, 6.5)),
                      line((9.5, 6.5), (9.5, 4.5), (14.5, 4.5), (14.5, 6.5)),
                      line((6.25, 6.5), (7.1, 19), (16.9, 19), (17.75, 6.5)),
                      line((10.25, 10), (10.25, 15.5)), line((13.75, 10), (13.75, 15.5))])]

ICONS = {
    "calendar_grid": calendar_grid, "today": today, "list": list_, "flag": flag, "repeat": repeat,
    "location_pin": location_pin, "bell": bell, "note": note, "person": person, "people": people,
    "wifi_off": wifi_off, "sync": sync, "sync_error": sync_error, "settings": settings,
    "moon": moon, "sun": sun, "edit": edit, "trash": trash,
}

# --- Rendering

def fit(layers):
    """Scale and offset that make the drawn artwork ICON_SIZE across at its widest, centered on the canvas."""
    shapes = [s for kind, group in layers if kind == "draw" for s in group]
    def bounds(scale):
        r = STROKE / 2
        boxes = []
        for kind, data in shapes:
            if kind == "line":
                boxes += [(x * scale - r, y * scale - r, x * scale + r, y * scale + r) for x, y in data]
            else:
                cx, cy, rad = data
                boxes.append(((cx - rad) * scale, (cy - rad) * scale, (cx + rad) * scale, (cy + rad) * scale))
        return (min(b[0] for b in boxes), min(b[1] for b in boxes), max(b[2] for b in boxes), max(b[3] for b in boxes))
    scale = 1.0
    for _ in range(4):
        x0, y0, x1, y1 = bounds(scale)
        widest = max(x1 - x0, y1 - y0)
        scale *= (ICON_SIZE - STROKE) / (widest - STROKE)
    x0, y0, x1, y1 = bounds(scale)
    return scale, (CANVAS - (x0 + x1)) / 2, (CANVAS - (y0 + y1)) / 2

def coverage(shapes, scale, dx, dy, grow=0.0):
    """How much of each pixel the shapes cover, 0 to 1, keyed by pixel index."""
    out = {}
    def mark(x0, y0, x1, y1, edge):
        for py in range(max(0, int(y0) - 1), min(CANVAS, int(y1) + 2)):
            for px in range(max(0, int(x0) - 1), min(CANVAS, int(x1) + 2)):
                c = 0.5 - edge(px + 0.5, py + 0.5)
                if c > 0:
                    i = py * CANVAS + px
                    out[i] = max(out.get(i, 0.0), min(c, 1.0))
    r = STROKE / 2 + grow
    for kind, data in shapes:
        if kind == "dot":
            cx, cy, rad = data[0] * scale + dx, data[1] * scale + dy, data[2] * scale + grow
            mark(cx - rad, cy - rad, cx + rad, cy + rad, lambda x, y: math.hypot(x - cx, y - cy) - rad)
            continue
        pts = [(x * scale + dx, y * scale + dy) for x, y in data]
        for (ax, ay), (bx, by) in zip(pts, pts[1:]):
            vx, vy = bx - ax, by - ay
            length2 = vx * vx + vy * vy or 1e-9
            def edge(x, y, ax=ax, ay=ay, vx=vx, vy=vy, length2=length2):
                t = max(0.0, min(1.0, ((x - ax) * vx + (y - ay) * vy) / length2))
                return math.hypot(x - ax - t * vx, y - ay - t * vy) - r
            mark(min(ax, bx) - r, min(ay, by) - r, max(ax, bx) + r, max(ay, by) + r, edge)
    return out

def render(layers):
    scale, dx, dy = fit(layers)
    alpha = [0.0] * (CANVAS * CANVAS)
    for kind, shapes in layers:
        if kind == "draw":
            for i, c in coverage(shapes, scale, dx, dy).items():
                alpha[i] = max(alpha[i], c)
        else:
            for i, c in coverage(shapes, scale, dx, dy, grow=GAP).items():
                alpha[i] *= 1.0 - c
    return alpha

def write_png(path, width, height, rgba_rows):
    raw = b"".join(b"\x00" + bytes(row) for row in rgba_rows)
    chunk = lambda tag, body: struct.pack(">I", len(body)) + tag + body + struct.pack(">I", zlib.crc32(tag + body))
    with open(path, "wb") as f:
        f.write(b"\x89PNG\r\n\x1a\n" + chunk(b"IHDR", struct.pack(">IIBBBBB", width, height, 8, 6, 0, 0, 0))
                + chunk(b"IDAT", zlib.compress(raw, 9)) + chunk(b"IEND", b""))

def main(args):
    sheet = None
    if "--sheet" in args:
        i = args.index("--sheet")
        sheet = args[i + 1]
        args = args[:i] + args[i + 2:]
    names = args or list(ICONS)
    out_dir = os.path.join(os.path.dirname(os.path.abspath(__file__)), "..", "assets", "icons")
    rendered = {}
    for name in names:
        alpha = render(ICONS[name]())
        rendered[name] = alpha
        rows = [[v for i in range(y * CANVAS, (y + 1) * CANVAS) for v in (255, 255, 255, round(alpha[i] * 255))]
                for y in range(CANVAS)]
        write_png(os.path.join(out_dir, name + ".png"), CANVAS, CANVAS, rows)
        print("wrote", name + ".png")
    if sheet:
        # White on black, a row of six, so the set can be checked at a glance.
        cols, pad = 6, 8
        count = len(rendered)
        w = cols * (CANVAS + pad) + pad
        h = ((count + cols - 1) // cols) * (CANVAS + pad) + pad
        pixels = [[0] * (w * 4) for _ in range(h)]
        for row in pixels:
            row[3::4] = [255] * w
        for n, alpha in enumerate(rendered.values()):
            ox, oy = pad + (n % cols) * (CANVAS + pad), pad + (n // cols) * (CANVAS + pad)
            for y in range(CANVAS):
                for x in range(CANVAS):
                    v = round(alpha[y * CANVAS + x] * 255)
                    pixels[oy + y][(ox + x) * 4:(ox + x) * 4 + 3] = [v, v, v]
        write_png(sheet, w, h, pixels)
        print("wrote", sheet)

if __name__ == "__main__":
    main(sys.argv[1:])
