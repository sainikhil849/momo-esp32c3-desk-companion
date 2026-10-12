"""
Momo desk robot - two 3D-printable bodies (CadQuery, units: mm)

  Design A "Momo Cube"  : friendly rounded cube, dark visor, headphone ears, tilted dock
  Design B "Momo Pod"   : helmet-shaped pod with a domed back, big visor, two antennas

Both designs use the same printed ELECTRONICS TRAY (ESP32-C3 Super Mini, MPU6050,
DS3231) that slides in from the back, so the electronics go in the same way.

Axes: X = right, Y = back (the face is at Y = 0), Z = up (desk at Z = 0).
Every part is designed to print on a Bambu Lab A1 mini (180 x 180 x 180 mm)
without supports, in the orientation given in PRINT_ORIENTATION below.

Run:  python momo_enclosures.py      -> writes STEP + STL for every part into ./out
Change the numbers in PARAMETERS if your modules are a bit different.
"""
import math
import os
import cadquery as cq

# ============================== PARAMETERS ==============================
CL = 0.25            # sliding clearance per side
T = 2.4              # shell wall
FT = 2.6             # front wall (behind the visor)
VISOR_T = 1.0        # visor plate thickness (sits proud on the flat face)
VIS_RING = 0.9       # depth of the visor's locating ring / face pocket

# 1.3" OLED module (SH1106, 4 pins)
OLED_PCB = (35.4, 33.5)       # w x h
OLED_GLASS_T = 1.8            # glass + polariser
OLED_PCB_T = 1.6
WIN = (31.5, 18.0)            # visible window (active area 29.4 x 14.7 + margin)
WIN_UP = 1.2                  # active area sits this much above the PCB centre
WIN_CHAMFER = 1.4             # 45 deg bevel around the window
RET_X = 22.3                  # OLED retainer screw posts (M2)

# ESP32-C3 Super Mini
ESP_W, ESP_L, ESP_T = 18.0, 22.6, 1.0
ESP_UNDER = 9.5               # room under the board (straight header pins fit)
USB_OPEN = (13.4, 7.6)        # opening for the USB-C plug overmould
USB_ABOVE = 1.6               # USB-C centre above the PCB top

# GY-521 (MPU6050), mounted upright, long side left-right (its X axis = Momo's left-right)
MPU_W, MPU_H = 21.2, 16.0
# DS3231 (ZS-042 size), mounted upright
DS_W, DS_H = 38.0, 22.0
# passive buzzer
BUZ_D, BUZ_L = 12.0, 9.5
# TTP223 touch module
TTP = (15.0, 11.0)

# electronics tray
TRAY_Y0 = 15.0                # front edge of the tray
TRAY_L = 40.0                 # depth
TRAY_BASE = 1.6
TRAY_WING = 21.5              # half width of the front part (DS3231)
TRAY_NARROW = 13.5            # half width of the back part

# ============================== HELPERS ==============================

def plane_y(y):
    """plane at Y=y, local (x, y) = (X, Z), extrudes towards -Y"""
    return cq.Plane(origin=(0, y, 0), xDir=(1, 0, 0), normal=(0, -1, 0))


def plane_x(x):
    """plane at X=x, local (x, y) = (Y, Z), extrudes towards +X"""
    return cq.Plane(origin=(x, 0, 0), xDir=(0, 1, 0), normal=(1, 0, 0))


def plane_z(z):
    """plane at Z=z, local (x, y) = (X, Y), extrudes towards +Z"""
    return cq.Plane(origin=(0, 0, z), xDir=(1, 0, 0), normal=(0, 0, 1))


def rr_pts(w, h, r, cx=0.0, cy=0.0, n=8, rb=None):
    """rounded rectangle as points (counter-clockwise); rb = bottom corner radius"""
    rt = r
    rb = r if rb is None else rb
    pts = []
    def arc(xc, yc, rad, a0, a1):
        for i in range(n + 1):
            a = math.radians(a0 + (a1 - a0) * i / n)
            pts.append((xc + rad * math.cos(a), yc + rad * math.sin(a)))
    x0, x1, y0, y1 = cx - w / 2, cx + w / 2, cy - h / 2, cy + h / 2
    arc(x1 - rb, y0 + rb, rb, -90, 0)
    arc(x1 - rt, y1 - rt, rt, 0, 90)
    arc(x0 + rt, y1 - rt, rt, 90, 180)
    arc(x0 + rb, y0 + rb, rb, 180, 270)
    # drop duplicate neighbours
    out = []
    for p in pts:
        if not out or abs(out[-1][0] - p[0]) > 1e-6 or abs(out[-1][1] - p[1]) > 1e-6:
            out.append(p)
    return out


def prism_y(pts, y0, y1):
    """polygon (X, Z points) extruded from y0 to y1"""
    return cq.Workplane(plane_y(y1)).polyline(pts).close().extrude(y1 - y0)


def prism_z(pts, z0, z1):
    return cq.Workplane(plane_z(z0)).polyline(pts).close().extrude(z1 - z0)


def prism_x(pts, x0, x1):
    return cq.Workplane(plane_x(x0)).polyline(pts).close().extrude(x1 - x0)


def box(x0, x1, y0, y1, z0, z1):
    x0, x1 = sorted((x0, x1)); y0, y1 = sorted((y0, y1)); z0, z1 = sorted((z0, z1))
    return cq.Workplane("XY").box(x1 - x0, y1 - y0, z1 - z0).translate(((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2))


def cyl_y(x, z, r, y0, y1):
    return cq.Workplane(plane_y(y1)).center(x, z).circle(r).extrude(y1 - y0)


def cyl_z(x, y, r, z0, z1):
    return cq.Workplane(plane_z(z0)).center(x, y).circle(r).extrude(z1 - z0)


def cyl_x(y, z, r, x0, x1):
    return cq.Workplane(plane_x(x0)).center(y, z).circle(r).extrude(x1 - x0)


def loft_y(sections, ruled=False):
    """sections: list of (y, pts) with the same number of points -> smooth (or straight) loft"""
    wires = []
    for y, pts in sections:
        vecs = [cq.Vector(p[0], y, p[1]) for p in pts]
        wires.append(cq.Wire.makePolygon(vecs, close=True))
    return cq.Workplane("XY").add(cq.Solid.makeLoft(wires, ruled=ruled))


def window_cut(cx, cz, y_in):
    """the screen window through the face (prints face-down, so nothing starts in mid-air):
    a shallow straight pocket for the visor's locating ring, then a 45 deg funnel to the glass"""
    w, h = WIN
    cb = rr_pts(w + 3.6, h + 3.6, 3.8, cx, cz, n=6)
    pocket = prism_y(cb, -1.0, VIS_RING + 0.05)
    funnel = loft_y([(VIS_RING + 0.04, cb), (VIS_RING + 1.65, rr_pts(w + 0.4, h + 0.4, 2.2, cx, cz, n=6))])
    straight = prism_y(rr_pts(w + 0.4, h + 0.4, 2.2, cx, cz, n=6), VIS_RING + 1.6, y_in + 0.5)
    return pocket.union(funnel).union(straight)


def heart_pts(cx, cy, s=1.0, n=40):
    """heart shape (classic parametric curve), ~13 mm wide at s=1"""
    pts = []
    for i in range(n):
        t = 2 * math.pi * i / n
        x = 16 * math.sin(t) ** 3
        y = 13 * math.cos(t) - 5 * math.cos(2 * t) - 2 * math.cos(3 * t) - math.cos(4 * t)
        pts.append((cx + x * 0.4 * s, cy + y * 0.4 * s))
    return pts[::-1]


# ============================== SHARED INSIDE PARTS ==============================

def face_inside_features(face_in_y, oled_cz, buz_cz):
    """OLED pocket + retainer posts + buzzer cup, growing back from the inside of the face"""
    pw, ph = OLED_PCB[0] + 2 * 0.4, OLED_PCB[1] + 2 * 0.4
    depth = OLED_GLASS_T + OLED_PCB_T + 0.2
    frame = prism_y(rr_pts(pw + 2.4, ph + 2.4, 1.5, 0, oled_cz), face_in_y, face_in_y + depth)
    frame = frame.cut(prism_y(rr_pts(pw, ph, 0.6, 0, oled_cz), face_in_y - 1, face_in_y + depth + 1))
    # gap in the bottom side of the frame for the screen's ribbon cable
    frame = frame.cut(box(-11, 11, face_in_y - 1, face_in_y + depth + 1, oled_cz - ph / 2 - 3, oled_cz - ph / 2 + 0.1))
    post_top = face_in_y + OLED_GLASS_T + OLED_PCB_T + 0.4
    posts = None
    for sx in (-1, 1):
        p = cyl_y(sx * RET_X, oled_cz, 2.8, face_in_y - 0.5, post_top)
        p = p.cut(cyl_y(sx * RET_X, oled_cz, 0.85, face_in_y + 0.2, post_top + 1))   # M2 x 6 pilot
        posts = p if posts is None else posts.union(p)
    cup = cyl_y(0, buz_cz, BUZ_D / 2 + 1.2, face_in_y - 0.5, face_in_y + 5.0)
    cup = cup.cut(cyl_y(0, buz_cz, BUZ_D / 2 + 0.2, face_in_y - 1, face_in_y + 6))
    return frame.union(posts).union(cup)


def mouth_holes(buz_cz, y0, y1):
    """five little holes in a smile below the visor - the buzzer sings through them"""
    r_arc = 10.0
    cut = None
    for k in range(-2, 3):
        a = math.radians(k * 14.0)
        x = r_arc * math.sin(a)
        z = buz_cz - 2.5 + r_arc - r_arc * math.cos(a)
        c = cyl_y(x, z, 1.05, y0, y1)
        cut = c if cut is None else cut.union(c)
    return cut


def touch_pocket(top_in_z, cy):
    """recess for the TTP223 under the touch spot + a small rim to hold it"""
    w, d = TTP[0] + 0.6, TTP[1] + 0.6
    recess = box(-w / 2, w / 2, cy - d / 2, cy + d / 2, top_in_z - 2.0, top_in_z + 0.8)
    rim = box(-w / 2 - 1.0, w / 2 + 1.0, cy - d / 2 - 1.0, cy + d / 2, top_in_z - 2.2, top_in_z + 0.2)
    rim = rim.cut(box(-w / 2, w / 2, cy - d / 2, cy + d / 2 + 2, top_in_z - 3, top_in_z + 1))
    # 45 deg ramp in front of the rim, so it never starts in mid-air when the shell prints face-down
    yf = cy - d / 2 - 1.0
    ramp = (cq.Workplane(plane_x(-w / 2 - 1.0)).polyline([(yf + 0.01, top_in_z - 2.2), (yf + 0.01, top_in_z + 0.2),
                                             (yf - 2.4, top_in_z + 0.2)]).close().extrude(w + 2.0))
    return recess, rim.union(ramp)


def floor_stops(floor_z):
    """ribs on the floor that stop the tray going forward and sideways (start at the face)"""
    y_front = FT - 0.3
    ribs = None
    for sx in (-1, 1):
        stop = box(sx * 17.0 - 1.2, sx * 17.0 + 1.2, y_front, TRAY_Y0 - CL, floor_z - 0.5, floor_z + 3.0)
        guide = box(sx * (TRAY_WING + CL), sx * (TRAY_WING + CL + 1.5), y_front, TRAY_Y0 + 10, floor_z - 0.5, floor_z + 3.0)
        for b in (stop, guide):
            ribs = b if ribs is None else ribs.union(b)
    return ribs


def tray():
    """the electronics tray: DS3231 + MPU6050 upright at the front, ESP32-C3 on rails at the back"""
    y0, y1 = TRAY_Y0, TRAY_Y0 + TRAY_L
    zb = 0.0
    outline = [(-TRAY_WING, y0), (TRAY_WING, y0), (TRAY_WING, y0 + 10), (TRAY_NARROW, y0 + 12),
               (TRAY_NARROW, y1), (-TRAY_NARROW, y1), (-TRAY_NARROW, y0 + 12), (-TRAY_WING, y0 + 10)]
    base = prism_z(outline, zb, zb + TRAY_BASE)
    base = base.edges("|Z").fillet(1.5)
    top = zb + TRAY_BASE
    # DS3231: long bottom edge in a groove, one short edge in a slot post (the other end has its pins)
    ds_y = y0 + 6.0
    groove_rib = box(-15, 15, ds_y - 1.8, ds_y + 1.8, top - 0.1, top + 2.5)
    groove = box(-16, 16, ds_y - 0.9, ds_y + 0.9, top + 0.8, top + 3)
    groove = groove.union(box(-16, 16, ds_y - 2.0, ds_y - 0.89, top + 1.5, top + 3))   # coin-cell side kept low
    # MPU6050: upright, both short edges in slot posts, pins point back under the ESP32
    mpu_y = y0 + 12.0
    mposts = None
    for sx in (-1, 1):
        p = box(sx * (MPU_W / 2 - 1.2), sx * (MPU_W / 2 + 1.8), mpu_y - 2.2, mpu_y + 2.2, top - 0.1, top + 13)
        p = p.cut(box(sx * (MPU_W / 2 - 1.3), sx * (MPU_W / 2 + 0.35), mpu_y - 0.9, mpu_y + 0.9, top + 0.4, top + 14))
        mposts = p if mposts is None else mposts.union(p)
    # ESP32-C3: rails with a blind slot (the end of the slot is the stop)
    pcb_z = top + ESP_UNDER
    esp_front = y1 - ESP_L
    rails = None
    for sx in (-1, 1):
        r = box(sx * (ESP_W / 2 + CL), sx * (ESP_W / 2 + CL + 1.8), esp_front - 2.5, y1, top - 0.1, pcb_z + ESP_T + 2.0)
        r = r.cut(box(sx * (ESP_W / 2 - 0.6), sx * (ESP_W / 2 + CL + 0.8), esp_front, y1 + 1, pcb_z - 0.1, pcb_z + ESP_T + 0.25))
        rails = r if rails is None else rails.union(r)
    # finger notch to pull the tray out, wire tie slots
    t = base.union(groove_rib).cut(groove).union(mposts).union(rails)
    t = t.cut(box(-4, 4, y1 - 3, y1 + 1, -1, top + 1).edges("|Z").fillet(1.0))
    for sx in (-1, 1):
        t = t.cut(box(sx * 11.0 - 1.0, sx * 11.0 + 1.0, y0 + 20, y0 + 24, -1, top + 1))
    return t


def tray_usb_z(floor_z):
    return floor_z + TRAY_BASE + ESP_UNDER + ESP_T + USB_ABOVE


# ============================== DESIGN A: MOMO CUBE ==============================
A_W, A_H, A_R = 68.0, 62.0, 12.0
A_BACK_IN = TRAY_Y0 + TRAY_L + 0.8          # inside face of the back cover
A_D = A_BACK_IN + T                         # total depth
A_OLED_Z = 38.0
A_BUZ_Z = 13.5
A_VISOR = (54.0, 34.0, 10.0)
A_TOUCH_Y = 14.0
A_EAR = (40.0, 30.0)                        # ear centre (Z, Y)


def a_shell():
    outer = cq.Workplane("XY").box(A_W, A_BACK_IN, A_H).edges("|Y").fillet(A_R)
    outer = outer.faces("<Y").edges().chamfer(2.5)
    outer = outer.faces(">Y").edges().chamfer(0.8)
    outer = outer.translate((0, A_BACK_IN / 2, A_H / 2))
    inner = cq.Workplane("XY").box(A_W - 2 * T, A_BACK_IN, A_H - 2 * T).edges("|Y").fillet(A_R - T)
    inner = inner.translate((0, FT + A_BACK_IN / 2, A_H / 2))
    s = outer.cut(inner)
    win_cz = A_OLED_Z + WIN_UP
    # visor recess, window, mouth
    s = s.cut(window_cut(0, win_cz, FT))
    s = s.cut(mouth_holes(A_BUZ_Z, -1, FT + 1))
    # inside the face
    s = s.union(face_inside_features(FT, A_OLED_Z, A_BUZ_Z))
    # touch spot: heart on top, TTP223 under it
    s = s.cut(prism_z(heart_pts(0, A_TOUCH_Y, 0.95), A_H - 0.8, A_H + 1))
    recess, rim = touch_pocket(A_H - T, A_TOUCH_Y)
    s = s.cut(recess).union(rim)
    # floor ribs for the tray
    s = s.union(floor_stops(T))
    # ears: shallow seat + two pin holes (1.75 mm filament pins) on each side
    ez, ey = A_EAR
    for sx in (-1, 1):
        x_out = sx * A_W / 2
        s = s.cut(cyl_x(ey, ez, 11.2, x_out - 0.6 if sx > 0 else x_out - 1, x_out + 1 if sx > 0 else x_out + 0.6))
        for dy in (-5.0, 5.0):
            s = s.cut(cyl_x(ey + dy, ez, 0.95, -A_W / 2 - 1, A_W / 2 + 1))
    # back cover: top hook recess + two screws from below
    s = s.cut(box(-8.5, 8.5, A_BACK_IN - 3.5, A_BACK_IN + 1, A_H - T - 0.01, A_H - T + 1.0))
    for sx in (-1, 1):
        s = s.cut(cyl_z(sx * 19.0, A_BACK_IN - 3.5, 1.2, -1, T + 1))           # M2 clearance
        s = s.cut(cyl_z(sx * 19.0, A_BACK_IN - 3.5, 2.3, -1, 1.4))             # pan head counterbore
    return s


def a_back():
    y0, y1 = A_BACK_IN, A_D
    plate = prism_y(rr_pts(A_W, A_H, A_R, 0, A_H / 2), y0, y1)
    plate = plate.faces(">Y").edges().chamfer(1.0)
    # lip inside the shell opening
    lw, lh, lr = A_W - 2 * T - 2 * CL, A_H - 2 * T - 2 * CL, A_R - T - CL
    lip = prism_y(rr_pts(lw, lh, lr, 0, A_H / 2), y0 - 3.0, y0 + 0.01)
    lip = lip.cut(prism_y(rr_pts(lw - 2.4, lh - 2.4, max(lr - 1.2, 0.5), 0, A_H / 2), y0 - 4, y0 + 1))
    lip = lip.cut(box(-TRAY_NARROW - 0.6, TRAY_NARROW + 0.6, y0 - 4, y0 + 1, 0, T + 4.0))
    p = plate.union(lip)
    # top hook (goes into the recess in the shell roof)
    p = p.union(box(-8, 8, y0 - 3.0, y0 + 0.01, A_H - T - CL - 1.2, A_H - T + 0.8))
    # screw blocks (screws come from below through the floor)
    for sx in (-1, 1):
        blk = box(sx * 19.0 - 3.5, sx * 19.0 + 3.5, y0 - 7.0, y0 + 0.01, T + 0.2, T + 8.0)
        blk = blk.cut(cyl_z(sx * 19.0, y0 - 3.5, 0.85, T, T + 7.0))
        p = p.union(blk)
    # tongue that keeps the tray in place
    p = p.union(box(-8, 8, y0 - 0.6, y0 + 0.01, T + 0.2, T + 1.4))
    # USB-C opening, vents, name
    uz = tray_usb_z(T)
    p = p.cut(prism_y(rr_pts(USB_OPEN[0], USB_OPEN[1], 3.2, 0, uz), y0 - 4, y1 + 1))
    for k in range(-3, 4):
        p = p.cut(prism_y(rr_pts(2.0, 14.0, 1.0, k * 5.0, 44.0), y0 - 4, y1 + 1))
    try:
        txt = (cq.Workplane(cq.Plane(origin=(0, y1, 28.0), xDir=(-1, 0, 0), normal=(0, 1, 0)))
               .text("MOMO", 7.0, -0.6, font="DejaVu Sans", kind="bold", halign="center", valign="center"))
        p = p.cut(txt)
    except Exception as e:  # no font: skip the name
        print("text skipped:", e)
    return p


def visor(w, h, r):
    """dark visor (print in black, front face down). Local origin = window centre.
    The plate sits ON the face (Y -1..0); a ring on its back drops into the face pocket and
    centres it, and its bore is one continuous 45 deg black bevel down to the glass."""
    plate = prism_y(rr_pts(w, h, r), -VISOR_T, 0.0)
    plate = plate.faces("<Y").edges().chamfer(0.4)
    ring = prism_y(rr_pts(WIN[0] + 3.3, WIN[1] + 3.3, 3.65, n=6), -0.01, VIS_RING - 0.05)
    v = plate.union(ring)
    def bore(y):                       # bore size shrinks 1 mm per side per mm of depth
        g = 2.0 - 2.0 * y
        return rr_pts(WIN[0] + g, WIN[1] + g, 2.0 + g / 2, n=6)
    return v.cut(loft_y([(-VISOR_T - 0.01, bore(-VISOR_T - 0.01)), (VIS_RING, bore(VIS_RING))]))


def a_ear():
    """headphone-style ear: disc + button, two pin holes on the back (print flat side down)"""
    e = cyl_z(0, 0, 11.0, 0, 4.5).edges(">Z").fillet(1.8)
    e = e.union(cyl_z(0, 0, 5.0, 4.0, 6.0).edges(">Z").fillet(0.8))
    for dy in (-5.0, 5.0):
        e = e.cut(cyl_z(0, dy, 0.95, -1, 3.0))
    return e


def dock_lift(tilt_deg=6.0):
    """height of Momo's front-bottom edge in the dock (the back-bottom edge is 2 mm above the desk)"""
    return 2.0 + (A_D + 0.4) * math.sin(math.radians(tilt_deg))


def a_dock(tilt_deg=6.0):
    """low dock: Momo leans back a little, lifts out easily for the tilt games"""
    W, D, H = A_W + 10, A_D + 10, 10.0
    dock = cq.Workplane("XY").box(W, D, H).edges("|Z").fillet(10).edges(">Z").fillet(2.0)
    dock = dock.translate((0, A_D / 2, H / 2))
    pocket = cq.Workplane("XY").box(A_W + 0.8, A_D + 0.8, 40).edges("|Z").fillet(2.0)
    pocket = pocket.translate((0, A_D / 2, 20))
    pocket = pocket.rotate((0, 0, 0), (1, 0, 0), -tilt_deg)
    pocket = pocket.translate((0, 0, dock_lift(tilt_deg)))
    dock = dock.cut(pocket)
    for sx in (-1, 1):
        for y in (4.0, A_D - 4.0):
            dock = dock.cut(cyl_z(sx * (A_W / 2 - 2), y, 4.2, -1, 1.0))
    return dock


# ============================== DESIGN B: MOMO POD ==============================
B_SECTIONS = [  # (y, width, height, bottom radius, top radius, bottom z)
    (0.0, 65.0, 59.0, 4.5, 20.5, 1.5),
    (1.5, 68.0, 62.0, 6.0, 22.0, 0.0),
    (18.0, 70.0, 64.0, 7.0, 24.0, 0.0),
    (38.0, 62.0, 56.0, 7.0, 21.0, 0.0),
]
B_BACK_IN = TRAY_Y0 + TRAY_L + 0.8
B_BACK = (B_BACK_IN, 50.0, 42.0, 6.0, 14.0, 0.0)
B_D = B_BACK_IN + T
B_OLED_Z = 37.0
B_BUZ_Z = 12.8
B_VISOR = (50.0, 32.0, 13.0)
B_TOUCH_Y = 14.0
B_SCREW_X = 16.7


def b_profile(sec, inset=0.0):
    y, w, h, rb, rt, z0 = sec
    w2, h2 = w - 2 * inset, h - 2 * inset
    cz = z0 + inset + h2 / 2
    return rr_pts(w2, h2, max(rt - inset, 1.0), 0, cz, n=10, rb=max(rb - inset, 1.0))


def surface_z(solid, x, y):
    """highest Z of a solid along a vertical line (x, y)"""
    probe = box(x - 0.05, x + 0.05, y - 0.05, y + 0.05, -1, 200)
    return probe.intersect(solid).val().BoundingBox().zmax


def b_shell():
    # front edge: a short straight chamfer; the body behind it: one smooth loft.
    # (a smooth loft through the tiny chamfer step would ripple the flat bottom)
    secs = B_SECTIONS[1:] + [B_BACK]
    chamfer = loft_y([(s[0], b_profile(s)) for s in B_SECTIONS[:2]], ruled=True)
    outer = loft_y([(s[0], b_profile(s)) for s in secs]).union(chamfer)
    inner_secs = [(FT, ) + tuple(B_SECTIONS[1][1:])] + B_SECTIONS[2:] + [B_BACK]
    inner = loft_y([(s[0], b_profile(s, T)) for s in inner_secs] +
                   [(B_BACK[0] + 2.0, b_profile(B_BACK, T))])
    s = outer.cut(inner)
    win_cz = B_OLED_Z + WIN_UP
    s = s.cut(window_cut(0, win_cz, FT))
    s = s.cut(mouth_holes(B_BUZ_Z, -1, FT + 1))
    s = s.union(face_inside_features(FT, B_OLED_Z, B_BUZ_Z))
    # touch spot: a ring + dot on top, 0.7 mm deep, following the curved top
    skin = outer.cut(outer.translate((0, 0, -0.7)))
    mark = cyl_z(0, B_TOUCH_Y, 7.0, 40, 80).cut(cyl_z(0, B_TOUCH_Y, 5.4, 39, 81)).union(cyl_z(0, B_TOUCH_Y, 1.8, 40, 80))
    s = s.cut(mark.intersect(skin))
    in_top = surface_z(inner, 0, B_TOUCH_Y)
    recess, rim = touch_pocket(in_top, B_TOUCH_Y)
    s = s.cut(recess).union(rim)
    s = s.union(floor_stops(T))
    # antenna holes on top
    for sx in (-1, 1):
        s = s.cut(cyl_z(sx * 15.0, 30.0, 2.15, 40, 70))
    # back plate: top hook recess + two screws from below
    top_back_in = B_BACK[2] - T
    s = s.cut(box(-8.5, 8.5, B_BACK_IN - 3.5, B_BACK_IN + 1, top_back_in - 0.01, top_back_in + 1.0))
    for sx in (-1, 1):
        s = s.cut(cyl_z(sx * B_SCREW_X, B_BACK_IN - 3.5, 1.2, -1, T + 1))
        s = s.cut(cyl_z(sx * B_SCREW_X, B_BACK_IN - 3.5, 2.3, -1, 1.4))
        for x, y in ((20.0, 8.0), (18.0, B_BACK_IN - 10.3)):
            s = s.cut(cyl_z(sx * x, y, 4.2, -1, 0.8))         # rubber feet (8 mm bumpons)
    return s


def b_back():
    y0, y1 = B_BACK_IN, B_D
    _, w, h, rb, rt, _z = B_BACK
    plate = prism_y(rr_pts(w, h, rt, 0, h / 2, rb=rb), y0, y1)
    plate = plate.faces(">Y").edges().chamfer(1.0)
    lw, lh = w - 2 * T - 2 * CL, h - 2 * T - 2 * CL
    lip = prism_y(rr_pts(lw, lh, rt - T - CL, 0, h / 2, rb=max(rb - T - CL, 0.8)), y0 - 3.0, y0 + 0.01)
    lip = lip.cut(prism_y(rr_pts(lw - 2.4, lh - 2.4, rt - T - CL - 1.2, 0, h / 2, rb=0.6), y0 - 4, y0 + 1))
    lip = lip.cut(box(-TRAY_NARROW - 0.6, TRAY_NARROW + 0.6, y0 - 4, y0 + 1, 0, T + 4.0))
    p = plate.union(lip)
    p = p.union(box(-8, 8, y0 - 3.0, y0 + 0.01, h - T - CL - 1.2, h - T + 0.8))
    for sx in (-1, 1):
        blk = box(sx * B_SCREW_X - 2.8, sx * B_SCREW_X + 2.8, y0 - 7.0, y0 + 0.01, T + 0.2, T + 7.5)
        blk = blk.cut(cyl_z(sx * B_SCREW_X, y0 - 3.5, 0.85, T, T + 7.0))
        p = p.union(blk)
    p = p.union(box(-8, 8, y0 - 0.6, y0 + 0.01, T + 0.2, T + 1.4))
    uz = tray_usb_z(T)
    p = p.cut(prism_y(rr_pts(USB_OPEN[0], USB_OPEN[1], 3.2, 0, uz), y0 - 4, y1 + 1))
    for k in range(-2, 3):
        p = p.cut(prism_y(rr_pts(2.0, 10.0, 1.0, k * 5.0, 30.0), y0 - 4, y1 + 1))
    return p


def b_antenna():
    """antenna with a little bulb (print bulb-down: the flat top of the bulb is on the bed)"""
    bulb = cq.Workplane("XY").sphere(4.6).translate((0, 0, 4.6 - 1.2))
    bulb = bulb.cut(box(-6, 6, -6, 6, -3, 0))          # flat face on the bed
    stalk = cyl_z(0, 0, 2.0, 5.0, 5.0 + 9.0)
    stalk = stalk.edges(">Z").chamfer(0.5)
    return bulb.union(stalk)


def oled_retainer():
    """bar that presses the OLED from behind; two M2 x 6 screws into the posts"""
    w = 2 * RET_X + 6.0
    bar = box(-w / 2, w / 2, 0, 2.4, -4.0, 4.0).edges("|Y").fillet(1.5)
    for sx in (-1, 1):
        bar = bar.cut(cyl_y(sx * RET_X, 0, 1.15, -1, 3.4))
        bar = bar.union(box(sx * (OLED_PCB[0] / 2 - 2.8), sx * (OLED_PCB[0] / 2 - 0.4), -0.5, 0.01, -4.0, 4.0))
    return bar


# ============================== BUILD ==============================
PARTS = {
    # name: (builder, colour, print note)
    "A_cube_shell":   (a_shell,  "white",  "face down on the plate"),
    "A_cube_back":    (a_back,   "white",  "outside face down"),
    "A_cube_visor":   (lambda: visor(*A_VISOR), "black", "flat"),
    "A_cube_ear":     (a_ear,    "accent", "flat side down - print 2"),
    "A_cube_dock":    (a_dock,   "accent", "as it stands"),
    "B_pod_shell":    (b_shell,  "white",  "face down on the plate"),
    "B_pod_back":     (b_back,   "white",  "outside face down"),
    "B_pod_visor":    (lambda: visor(*B_VISOR), "black", "flat"),
    "B_pod_antenna":  (b_antenna, "accent", "bulb down - print 2"),
    "tray":           (tray,     "grey",   "flat (base down) - same tray for both"),
    "oled_retainer":  (oled_retainer, "grey", "flat - same for both"),
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
