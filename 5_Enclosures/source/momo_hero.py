"""
Momo 2.2 - Design C "Hero Momo" (CadQuery, units: mm)

A mini superhero desk body for Momo 2.2:
  * a black domino-mask visor around the screen
  * a red cape that clips onto the head with two pegs; a big star window in the cape
    shows the BACK BUTTON (the hero emblem is the button)
  * a star on top of the head = the touch spot (TTP223 underneath)
  * engraved lightning bolts on both sides, a smile of sound holes
  * a battery bay under the electronics deck: 500 mAh LiPo + TP4056 USB-C charger
  * on the back: CODE (ESP32 USB-C), CHARGE (TP4056 USB-C), power slide switch,
    and the 7 mm back button

It uses the same electronics tray and OLED retainer as designs A and B.
Axes: X = right, Y = back (face at Y = 0), Z = up (desk at Z = 0).
Every part prints on a Bambu Lab A1 mini (180 x 180 x 180 mm) without supports.

Run:  python momo_hero.py      -> writes STEP + STL for every part into ./out
"""
import math
import os
import cadquery as cq
from shapely.geometry import Polygon
import momo_enclosures as M
from momo_enclosures import (CL, T, FT, VISOR_T, VIS_RING, WIN, OLED_PCB, TRAY_Y0, TRAY_L, TRAY_WING,
                             TRAY_NARROW, USB_OPEN, box, cyl_x, cyl_y, cyl_z, prism_x, prism_y, prism_z,
                             loft_y, rr_pts, window_cut, mouth_holes, face_inside_features, touch_pocket,
                             floor_stops, tray_usb_z)

# ============================== PARAMETERS ==============================
C_W, C_H = 70.0, 70.0            # outer width and height
C_RT, C_RB = 14.0, 5.0           # top / bottom corner radius (seen from the front)
C_CHAMFER = 2.5                  # chamfer around the face
C_BACK_IN = TRAY_Y0 + TRAY_L + 0.8   # inside face of the back plate (55.8)
C_D = C_BACK_IN + T              # total depth (58.2)
C_OLED_Z = 46.0                  # OLED PCB centre
C_BUZ_Z = 21.0                   # buzzer centre (the smile is just below it)
C_TOUCH_Y = 16.0                 # star touch spot on the head

# battery deck: battery + charger lie on the floor, the electronics tray slides on the deck
BAT = (32.0, 7.0)                # bay width, bay height (fits 502035 / 503035 / 603030 500 mAh LiPos)
BAT_X0 = -31.0                   # left edge of the bay
DECK_T = 1.6
DECK_Z0 = T + BAT[1]             # deck underside (9.4)
DECK_TOP = DECK_Z0 + DECK_T      # deck top = floor of the electronics tray (11.0)
DECK_END = C_BACK_IN - 9.0       # the last 9 mm stay open: wires go up there (46.8)
DIV_X = BAT_X0 + BAT[0]          # divider between battery and charger (x = 1.0)

# TP4056 USB-C charger board (with protection), lying flat, USB-C facing back
TP_W, TP_L, TP_PCB = 17.5, 28.0, 1.6
TP_X = 16.5                      # centre
TP_LEDGE = 1.0                   # board sits on two 1 mm ledges
TP_USB_OVER = 0.8                # the USB-C socket sticks out past the board edge
TP_BACK = C_BACK_IN - 0.3 - TP_USB_OVER     # back edge of the board
TP_FRONT = TP_BACK - TP_L
TP_USB_Z = T + TP_LEDGE + TP_PCB + 1.6      # USB-C centre height (6.6)

# back panel parts
BTN_HOLE = 7.2                   # 7 mm panel push button (PBS-110 type)
BTN_Z = 48.5                     # back button centre height (middle of the cape star)
SW_X, SW_Z = -TP_X, 7.0          # power slide switch (SS12D00, 4-5 mm lever) - mirror of CHARGE
SW_BODY = (8.8, 3.8, 3.8)        # pocket for the switch body: along X, along Z, depth
SW_SLOT = (4.4, 2.2)             # slot for the lever (1.5 mm lever + 2 mm travel)
SCREW_X = 28.5                   # back plate screws (from below, M2 x 8)

# cape
CAPE_T = 1.6
CAPE_GAP = 0.8                   # between the cape and the back plate
CAPE_TOP_W = 40.0
CAPE_HEM_W = 58.0
CAPE_HEM_Z = 31.5                # lowest point of the hem (the CODE port top is at 28.5)
PEG_X, PEG_Y, PEG_R = 14.0, 50.0, 2.0
STAR_R, STAR_r = 15.5, 7.8       # star window: inner circle 12.6 mm, clears a 12 mm nut/collar


# ============================== SHAPES ==============================

def star_pts(cx, cy, R, r, n=5, rot=90.0):
    pts = []
    for i in range(2 * n):
        a = math.radians(rot + i * 180.0 / n)
        rad = R if i % 2 == 0 else r
        pts.append((cx + rad * math.cos(a), cy + rad * math.sin(a)))
    return pts


def rounded(pts, radius, res=6):
    """round the corners of a polygon (shapely: shrink then grow)"""
    p = Polygon(pts).buffer(-radius, join_style=1).buffer(radius, quad_segs=res)
    return list(p.exterior.coords)[:-1]


def chaikin(pts, it=3):
    for _ in range(it):
        out = []
        n = len(pts)
        for i in range(n):
            (x0, y0), (x1, y1) = pts[i], pts[(i + 1) % n]
            out.append((0.75 * x0 + 0.25 * x1, 0.75 * y0 + 0.25 * y1))
            out.append((0.25 * x0 + 0.75 * x1, 0.25 * y0 + 0.75 * y1))
        pts = out
    return pts


def mask_pts():
    """domino mask outline around the screen window (local origin = window centre)"""
    half = [(0, 13.6), (6, 14.6), (12, 16.6), (18, 17.4), (23, 16.2), (27, 13.4), (30, 9.4), (31.2, 5.0),
            (29.8, 0.8), (27.2, -4.2), (23, -10.0), (17, -14.4), (10, -15.8), (5, -15.0), (2, -13.8), (0, -13.4)]
    right = half[:-1]                                   # top centre -> bottom centre
    left = [(-x, y) for (x, y) in reversed(half[1:-1])]  # bottom -> top on the left side
    pts = right + [half[-1]] + left
    return chaikin(pts, 3)[::-1]                         # counter-clockwise


def bolt_pts(cy, cz, s=1.0):
    """lightning bolt (local: y, z), counter-clockwise"""
    b = [(1.0, 12.0), (-5.6, -2.4), (-0.6, -2.4), (-4.6, -14.0), (7.6, 2.6), (2.6, 2.6), (7.0, 12.0)]
    return [(cy + y * s, cz + z * s) for (y, z) in b]


def c_profile(inset=0.0):
    return rr_pts(C_W - 2 * inset, C_H - 2 * inset, C_RT - inset, 0, C_H / 2, n=10, rb=max(C_RB - inset, 0.8))


# ============================== SHELL ==============================

def c_shell():
    # outer: face chamfer (ruled loft) + straight body
    front = rr_pts(C_W - 2 * C_CHAMFER, C_H - 2 * C_CHAMFER, C_RT - C_CHAMFER, 0, C_H / 2, n=10, rb=C_RB - C_CHAMFER * 0.6)
    outer = loft_y([(0.0, front), (C_CHAMFER, c_profile()), (C_BACK_IN, c_profile())], ruled=True)
    inner = prism_y(c_profile(T), FT, C_BACK_IN + 1)
    s = outer.cut(inner)
    win_cz = C_OLED_Z + M.WIN_UP
    s = s.cut(window_cut(0, win_cz, FT))
    s = s.cut(mouth_holes(C_BUZ_Z, -1, FT + 1))
    s = s.union(face_inside_features(FT, C_OLED_Z, C_BUZ_Z))

    # star touch spot on the head (0.8 mm deep), TTP223 pocket under it
    s = s.cut(prism_z(rounded(star_pts(0, C_TOUCH_Y, 9.5, 4.6), 0.8), C_H - 0.8, C_H + 1))
    recess, rim = touch_pocket(C_H - T, C_TOUCH_Y)
    s = s.cut(recess).union(rim)

    # ---- battery deck ----
    xin = C_W / 2 - T
    deck = box(-xin - 0.5, xin + 0.5, FT - 0.3, DECK_END, DECK_Z0, DECK_TOP).intersect(prism_y(c_profile(T - 0.4), FT - 1, C_BACK_IN))
    divider = box(DIV_X, DIV_X + 1.6, FT - 0.3, DECK_END, T - 0.3, DECK_Z0 + 0.1)
    divider = divider.cut(box(DIV_X - 1, DIV_X + 3, DECK_END - 7.0, DECK_END + 1, T + 3.5, DECK_Z0 + 1))   # battery wires
    # TP4056 slot: two side guides from the face back, ledges, a 45-degree front stop
    g = None
    for x0 in (TP_X - TP_W / 2 - 0.5 - 1.6, TP_X + TP_W / 2 + 0.5):
        part = box(x0, x0 + 1.6, FT - 0.3, DECK_END, T - 0.3, T + 4.6)
        g = part if g is None else g.union(part)
    for xl in (TP_X - TP_W / 2 + 0.3, TP_X + TP_W / 2 - 1.8):
        g = g.union(box(xl, xl + 1.5, TP_FRONT - 0.6, DECK_END, T - 0.3, T + TP_LEDGE))
    stop = (cq.Workplane(M.plane_x(TP_X - TP_W / 2 - 0.5))
            .polyline([(TP_FRONT - 0.4, T - 0.3), (TP_FRONT - 0.4, T + 4.6), (TP_FRONT - 5.3, T - 0.3)]).close()
            .extrude(TP_W + 1.0))
    s = s.union(deck).union(divider).union(g).union(stop)
    # the tray rides on the deck
    s = s.union(floor_stops(DECK_TOP))

    # cape pegs go into two holes in the head
    for sx in (-1, 1):
        s = s.cut(cyl_z(sx * PEG_X, PEG_Y, PEG_R + 0.15, C_H - T - 1, C_H + 1))
    # back plate: top hook recess + two screws from below
    s = s.cut(box(-8.5, 8.5, C_BACK_IN - 3.5, C_BACK_IN + 1, C_H - T - 0.01, C_H - T + 1.0))
    for sx in (-1, 1):
        s = s.cut(cyl_z(sx * SCREW_X, C_BACK_IN - 3.5, 1.2, -1, T + 1))
        s = s.cut(cyl_z(sx * SCREW_X, C_BACK_IN - 3.5, 2.3, -1, 1.4))
        for (x, y) in ((26.0, 9.0), (21.0, 44.0)):
            s = s.cut(cyl_z(sx * x, y, 4.2, -1, 0.8))            # rubber feet (8 mm bumpons)
    # lightning bolts on both sides
    s = s.cut(prism_x(bolt_pts(30.0, 36.0, 1.05), C_W / 2 - 0.6, C_W / 2 + 1.0))
    s = s.cut(prism_x(bolt_pts(30.0, 36.0, 1.05), -C_W / 2 - 1.0, -C_W / 2 + 0.6))
    return s


# ============================== BACK PLATE ==============================

def c_back():
    y0, y1 = C_BACK_IN, C_D
    plate = prism_y(c_profile(), y0, y1)
    plate = plate.faces(">Y").edges().chamfer(1.0)
    # lip inside the opening: sides and top only (the bottom is the battery / charger area)
    lw = c_profile(T + CL)
    li = c_profile(T + CL + 1.2)
    lip = prism_y(lw, y0 - 3.0, y0 + 0.01).cut(prism_y(li, y0 - 4, y0 + 1))
    lip = lip.cut(box(-40, 40, y0 - 4, y0 + 1, -1, DECK_TOP + 3.0))
    p = plate.union(lip)
    # top hook
    p = p.union(box(-8, 8, y0 - 3.0, y0 + 0.01, C_H - T - CL - 1.2, C_H - T + 0.8))
    # screw blocks (screws come from below through the floor)
    for sx in (-1, 1):
        blk = box(sx * SCREW_X - 2.6, sx * SCREW_X + 2.6, y0 - 7.0, y0 + 0.01, T + 0.6, T + 7.5)
        blk = blk.cut(cyl_z(sx * SCREW_X, y0 - 3.5, 0.85, T, T + 7.0))
        p = p.union(blk)
    # tongue that keeps the tray in place
    p = p.union(box(-8, 8, y0 - 0.6, y0 + 0.01, DECK_TOP + 0.2, DECK_TOP + 1.4))
    # USB-C openings: CODE (ESP32) and CHARGE (TP4056)
    p = p.cut(prism_y(rr_pts(USB_OPEN[0], USB_OPEN[1], 3.2, 0, tray_usb_z(DECK_TOP)), y0 - 4, y1 + 1))
    p = p.cut(prism_y(rr_pts(USB_OPEN[0], USB_OPEN[1], 3.2, TP_X, TP_USB_Z), y0 - 4, y1 + 1))
    # back button
    p = p.cut(cyl_y(0, BTN_Z, BTN_HOLE / 2, y0 - 1, y1 + 1))
    # power slide switch: body pocket from the inside, lever slot through
    sw_w, sw_h, sw_d = SW_BODY
    frame = box(SW_X - sw_w / 2 - 1.2, SW_X + sw_w / 2 + 1.2, y0 - sw_d, y0 + 0.01, SW_Z - sw_h / 2 - 1.2, SW_Z + sw_h / 2 + 1.2)
    frame = frame.cut(box(SW_X - sw_w / 2, SW_X + sw_w / 2, y0 - sw_d - 1, y0 + 0.02, SW_Z - sw_h / 2, SW_Z + sw_h / 2))
    p = p.union(frame)
    p = p.cut(box(SW_X - sw_w / 2, SW_X + sw_w / 2, y0 - 0.01, y0 + 0.8, SW_Z - sw_h / 2, SW_Z + sw_h / 2))   # thinner plate here
    p = p.cut(prism_y(rr_pts(SW_SLOT[0], SW_SLOT[1], 0.6, SW_X, SW_Z), y0 - 1, y1 + 1))
    # labels (engraved, readable from behind)
    def label(txt, x, z, size=3.4):
        try:
            t = (cq.Workplane(cq.Plane(origin=(x, y1, z), xDir=(-1, 0, 0), normal=(0, 1, 0)))
                 .text(txt, size, -0.5, font="DejaVu Sans", kind="bold", halign="center", valign="center"))
            return t
        except Exception as e:  # no font: skip
            print("text skipped:", e)
            return None
    for txt, x, z in (("CODE", 0.0, tray_usb_z(DECK_TOP) - USB_OPEN[1] / 2 - 2.8),
                      ("CHARGE", TP_X, TP_USB_Z + USB_OPEN[1] / 2 + 2.6),
                      ("ON", SW_X + 7.4, SW_Z), ("POWER", SW_X, SW_Z + 5.2)):
        t = label(txt, x, z, 3.0 if txt in ("ON",) else 3.2)
        if t is not None:
            p = p.cut(t)
    return p


# ============================== MASK VISOR ==============================

def c_mask():
    """black domino-mask visor (print front face down). Local origin = window centre."""
    pts = mask_pts()
    plate = prism_y(pts, -VISOR_T, 0.0)
    plate = plate.faces("<Y").edges().chamfer(0.35)
    ring = prism_y(rr_pts(WIN[0] + 3.3, WIN[1] + 3.3, 3.65, n=6), -0.01, VIS_RING - 0.05)
    v = plate.union(ring)

    def bore(y):
        gg = 2.0 - 2.0 * y
        return rr_pts(WIN[0] + gg, WIN[1] + gg, 2.0 + gg / 2, n=6)
    return v.cut(loft_y([(-VISOR_T - 0.01, bore(-VISOR_T - 0.01)), (VIS_RING, bore(VIS_RING))]))


# ============================== CAPE ==============================

def hem_z(x):
    """wavy hem: four scallops"""
    k = (x + CAPE_HEM_W / 2) / CAPE_HEM_W * 4.0
    return CAPE_HEM_Z + 2.4 * (0.5 + 0.5 * math.cos(2 * math.pi * k))


def c_cape():
    """red cape: a collar on the head (two pegs into the head) and a cape down the back.
    Print it upside down: collar flat on the plate, the cape standing up."""
    top = C_H + CAPE_T
    yb0, yb1 = C_D + CAPE_GAP, C_D + CAPE_GAP + CAPE_T
    collar = prism_z(rounded([(-CAPE_TOP_W / 2, 44.0), (CAPE_TOP_W / 2, 44.0), (CAPE_TOP_W / 2, yb1), (-CAPE_TOP_W / 2, yb1)], 4.0),
                     C_H, top)
    # cape outline (X, Z): narrow at the shoulders, wider at the wavy hem, slight outward curve
    side = [(CAPE_TOP_W / 2, top), (CAPE_TOP_W / 2 + 3.2, 58.0), (CAPE_HEM_W / 2 - 1.0, 44.0), (CAPE_HEM_W / 2, hem_z(CAPE_HEM_W / 2) + 0.01)]
    hem = [(x, hem_z(x)) for x in [CAPE_HEM_W / 2 - i * CAPE_HEM_W / 60 for i in range(61)]]
    left = [(-x, z) for (x, z) in reversed(side)]
    outline = side + hem[1:-1] + [(-CAPE_HEM_W / 2, hem_z(-CAPE_HEM_W / 2) + 0.01)] + left[1:]
    sheet = prism_y(outline, yb0, yb1)
    star = rounded(star_pts(0, BTN_Z, STAR_R, STAR_r), 1.0)
    sheet = sheet.cut(prism_y(star, yb0 - 1, yb1 + 1))
    # raised star frame on the outside (the hero emblem)
    frame_o = list(Polygon(star).buffer(2.0, join_style=1).exterior.coords)[:-1]
    frame = prism_y(frame_o, yb1 - 0.01, yb1 + 0.7).cut(prism_y(star, yb1 - 1, yb1 + 2))
    c = collar.union(sheet).union(frame)
    for sx in (-1, 1):
        peg = cyl_z(sx * PEG_X, PEG_Y, PEG_R, C_H - 3.4, C_H + 0.01).faces("<Z").edges().chamfer(0.4)
        c = c.union(peg)
    return c


# ============================== BUILD ==============================
PARTS = {
    "C_hero_shell": (c_shell, "hero_blue", "face down on the plate"),
    "C_hero_back":  (c_back,  "hero_blue", "outside face down"),
    "C_hero_mask":  (c_mask,  "black",     "flat, front face down"),
    "C_hero_cape":  (c_cape,  "hero_red",  "upside down: collar flat on the plate"),
}

if __name__ == "__main__":
    import sys
    os.makedirs("out", exist_ok=True)
    only = sys.argv[1:]
    for name, (fn, col, note) in PARTS.items():
        if only and name not in only:
            continue
        part = fn()
        solid = part.val()
        print(f"{name:16s} valid={solid.isValid()} volume={solid.Volume():9.1f} mm3")
        cq.exporters.export(part, f"out/{name}.step")
        cq.exporters.export(part, f"out/{name}.stl", tolerance=0.02, angularTolerance=0.15)
