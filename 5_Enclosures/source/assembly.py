"""Assembly positions of every part + simple models of the electronics (for renders and fit checks)."""
import math
import numpy as np
import trimesh
import momo_enclosures as M
import momo_hero as H

COL = {
    "white": (0.95, 0.94, 0.91), "black": (0.10, 0.11, 0.13), "accent_a": (1.00, 0.48, 0.36),
    "accent_b": (0.36, 0.86, 0.80), "grey": (0.55, 0.57, 0.62), "pcb": (0.10, 0.36, 0.22),
    "pcb_blue": (0.12, 0.25, 0.55), "glass": (0.02, 0.02, 0.03), "metal": (0.75, 0.75, 0.78),
    "screen": (0.55, 0.98, 0.95), "buzzer": (0.08, 0.08, 0.08), "battery": (0.82, 0.82, 0.86),
    "hero_blue": (0.56, 0.76, 0.98), "hero_red": (0.90, 0.18, 0.22), "lipo": (0.80, 0.70, 0.30),
    "button_red": (0.85, 0.12, 0.15),
}


def load(name):
    m = trimesh.load(f"out/{name}.stl")
    m.metadata["name"] = name
    return m


def T(tx=0, ty=0, tz=0):
    return trimesh.transformations.translation_matrix((tx, ty, tz))


def R(axis, deg, point=(0, 0, 0)):
    return trimesh.transformations.rotation_matrix(math.radians(deg), axis, point)


def bx(x0, x1, y0, y1, z0, z1):
    b = trimesh.creation.box(extents=(abs(x1 - x0), abs(y1 - y0), abs(z1 - z0)))
    b.apply_translation(((x0 + x1) / 2, (y0 + y1) / 2, (z0 + z1) / 2))
    return b


def cy(axis, center, r, length, sections=48):
    c = trimesh.creation.cylinder(radius=r, height=length, sections=sections)
    if axis == "y":
        c.apply_transform(R((1, 0, 0), 90))
    elif axis == "x":
        c.apply_transform(R((0, 1, 0), 90))
    c.apply_translation(center)
    return c


def rrect_y(cx, cz, w, h, r, y0, y1):
    """rounded rectangle in the X-Z plane, thickness along Y from y0 to y1"""
    from shapely.geometry import box as sbox
    p = sbox(cx - w / 2 + r, cz - h / 2 + r, cx + w / 2 - r, cz + h / 2 - r).buffer(r, quad_segs=12)
    m = trimesh.creation.extrude_polygon(p, y1 - y0)
    m.apply_transform(R((1, 0, 0), 90))
    m.apply_translation((0, y1, 0))
    return m


def surface_hits(mesh, x, y):
    """Z of every surface a vertical line at (x, y) crosses, from the top down"""
    locs, _, _ = mesh.ray.intersects_location([[x, y, 300.0]], [[0, 0, -1.0]], multiple_hits=True)
    return sorted([p[2] for p in locs], reverse=True)


def pocket_top(shell, touch_y):
    """ceiling of the TTP223 pocket under the touch spot (first surface going up from inside)"""
    locs, _, _ = shell.ray.intersects_location([[3.0, touch_y, 20.0]], [[0, 0, 1.0]], multiple_hits=True)
    return min(p[2] for p in locs)


def electronics(floor_z, oled_cz, buz_cz, touch_top_in, touch_y):
    """simple shapes of the modules, where they sit inside Momo.
    returns (mesh, colour, group); group 'tray' rides on the tray, 'face' sits behind the face"""
    out = []
    ft = M.FT
    # OLED: glass (with a lit face drawn on it), PCB, 4-pin header at the top
    gz = oled_cz + 0.4
    out.append((bx(-17.25, 17.25, ft, ft + M.OLED_GLASS_T, gz - 11.5, gz + 11.5), "glass", "face"))
    wcz = oled_cz + M.WIN_UP
    for sx in (-1, 1):   # two eyes on the screen
        e = rrect_y(sx * 7.6, wcz, 9.0, 11.5, 3.0, ft - 0.05, ft + 0.2)
        out.append((e, "screen", "face"))
    out.append((bx(-17.7, 17.7, ft + M.OLED_GLASS_T, ft + M.OLED_GLASS_T + M.OLED_PCB_T, oled_cz - 16.75, oled_cz + 16.75), "pcb_blue", "face"))
    out.append((bx(-5.1, 5.1, ft + M.OLED_GLASS_T + M.OLED_PCB_T, ft + 6.0, oled_cz + 12.5, oled_cz + 15.0), "black", "face"))
    # buzzer
    out.append((cy("y", (0, ft + M.BUZ_L / 2 + 0.2, buz_cz), M.BUZ_D / 2, M.BUZ_L), "buzzer", "face"))
    # TTP223 under the touch spot
    out.append((bx(-7.5, 7.5, touch_y - 5.5, touch_y + 5.5, touch_top_in - 1.6, touch_top_in), "pcb_blue", "face"))
    # tray contents (tray base sits on the floor)
    top = floor_z + M.TRAY_BASE
    y0, y1 = M.TRAY_Y0, M.TRAY_Y0 + M.TRAY_L
    ds_y = y0 + 6.0
    out.append((bx(-19, 19, ds_y - 0.8, ds_y + 0.8, top + 0.9, top + 0.9 + M.DS_H), "pcb_blue", "tray"))
    out.append((cy("y", (2.0, ds_y - 0.8 - 2.6, top + 0.9 + 11), 10.0, 5.0), "battery", "tray"))
    out.append((bx(-3.8, 3.8, ds_y + 0.8, ds_y + 2.2, top + 8, top + 14), "black", "tray"))
    mpu_y = y0 + 12.0
    out.append((bx(-M.MPU_W / 2, M.MPU_W / 2, mpu_y - 0.8, mpu_y + 0.8, top + 0.5, top + 0.5 + M.MPU_H), "pcb_blue", "tray"))
    out.append((bx(-2, 2, mpu_y + 0.8, mpu_y + 1.7, top + 6, top + 10), "black", "tray"))
    pcb_z = top + M.ESP_UNDER
    out.append((bx(-9, 9, y1 - M.ESP_L, y1, pcb_z, pcb_z + M.ESP_T), "pcb", "tray"))
    out.append((bx(-4.5, 4.5, y1 - 7.0, y1 + 0.6, pcb_z + M.ESP_T, pcb_z + M.ESP_T + 3.2), "metal", "tray"))
    out.append((bx(-2.5, 2.5, y1 - 16, y1 - 11, pcb_z + M.ESP_T, pcb_z + M.ESP_T + 0.9), "black", "tray"))
    return out


def assembly_a(explode=0.0, docked=False, front=0.0):
    """explode: pull the back cover + tray out of the back; front: pull visor and ears off"""
    parts = []
    win_cz = M.A_OLED_Z + M.WIN_UP
    post_top = M.FT + M.OLED_GLASS_T + M.OLED_PCB_T + 0.4
    e, f = explode, front
    parts.append((load("A_cube_shell"), np.eye(4), "white"))
    parts.append((load("A_cube_back"), T(0, 1.6 * e, 0), "white"))
    parts.append((load("A_cube_visor"), T(0, -1.5 * f, win_cz), "black"))
    parts.append((load("oled_retainer"), T(0, post_top + 0.6 * e, M.A_OLED_Z), "grey"))
    parts.append((load("tray"), T(0, 1.2 * e, M.T), "grey"))
    ez, ey = M.A_EAR
    parts.append((load("A_cube_ear"), T(M.A_W / 2 - 0.6 + 0.6 * f, ey, ez) @ R((0, 1, 0), 90), "accent_a"))
    parts.append((load("A_cube_ear"), T(-M.A_W / 2 + 0.6 - 0.6 * f, ey, ez) @ R((0, 1, 0), -90), "accent_a"))
    for m, c, g in electronics(M.T, M.A_OLED_Z, M.A_BUZ_Z, pocket_top(parts[0][0], M.A_TOUCH_Y), M.A_TOUCH_Y):
        parts.append((m, T(0, 1.2 * e, 0) if g == "tray" else np.eye(4), c))
    if docked:
        Mx = T(0, 0, M.dock_lift()) @ R((1, 0, 0), -6.0)
        parts = [(m, Mx @ t, c) for (m, t, c) in parts]
        parts.append((load("A_cube_dock"), np.eye(4), "accent_a"))
    return parts


def assembly_b(explode=0.0, front=0.0):
    parts = []
    win_cz = M.B_OLED_Z + M.WIN_UP
    post_top = M.FT + M.OLED_GLASS_T + M.OLED_PCB_T + 0.4
    e, f = explode, front
    parts.append((load("B_pod_shell"), np.eye(4), "white"))
    parts.append((load("B_pod_back"), T(0, 1.6 * e, 0), "white"))
    parts.append((load("B_pod_visor"), T(0, -1.5 * f, win_cz), "black"))
    parts.append((load("oled_retainer"), T(0, post_top + 0.6 * e, M.B_OLED_Z), "grey"))
    parts.append((load("tray"), T(0, 1.2 * e, M.T), "grey"))
    shell = parts[0][0]
    for sx in (-1, 1):
        zs = surface_hits(shell, sx * 15.0 + sx * 2.3, 30.0)[0]
        parts.append((load("B_pod_antenna"), T(sx * 15.0, 30.0, zs + 9.0 + 0.8 * f) @ R((1, 0, 0), 180), "accent_b"))
    for m, c, g in electronics(M.T, M.B_OLED_Z, M.B_BUZ_Z, pocket_top(shell, M.B_TOUCH_Y), M.B_TOUCH_Y):
        parts.append((m, T(0, 1.2 * e, 0) if g == "tray" else np.eye(4), c))
    return parts


def power_parts():
    """battery, charger board, slide switch and back button of Hero Momo (fixed to the shell / back plate)"""
    out = []
    t = M.T
    # 500 mAh LiPo (worst case 503035 with its protection board: 5.6 x 30.5 x 41 mm)
    out.append((bx(H.BAT_X0 + 0.6, H.BAT_X0 + 0.6 + 30.5, M.FT + 0.4, M.FT + 0.4 + 41.0, t + 0.05, t + 5.65), "lipo", "shell"))
    # TP4056 USB-C board: PCB on its ledges, USB-C socket at the back, chip + LEDs
    pz = t + H.TP_LEDGE
    out.append((bx(H.TP_X - H.TP_W / 2, H.TP_X + H.TP_W / 2, H.TP_FRONT, H.TP_BACK, pz, pz + H.TP_PCB), "pcb_blue", "shell"))
    out.append((bx(H.TP_X - 4.45, H.TP_X + 4.45, H.TP_BACK - 6.5, H.TP_BACK + H.TP_USB_OVER, pz + H.TP_PCB, pz + H.TP_PCB + 3.2), "metal", "shell"))
    out.append((bx(H.TP_X - 2.5, H.TP_X + 2.5, H.TP_FRONT + 8, H.TP_FRONT + 13, pz + H.TP_PCB, pz + H.TP_PCB + 1.2), "black", "shell"))
    # slide switch body (in its pocket) + lever through the slot
    y0 = H.C_BACK_IN
    out.append((bx(H.SW_X - 4.3, H.SW_X + 4.3, y0 + 0.8 - 3.5, y0 + 0.8, H.SW_Z - 1.75, H.SW_Z + 1.75), "metal", "back"))
    out.append((bx(H.SW_X - 0.75 + 1.0, H.SW_X + 0.75 + 1.0, y0 + 0.8, y0 + 0.8 + 4.0, H.SW_Z - 0.75, H.SW_Z + 0.75), "black", "back"))
    # 7 mm panel push button: cap + collar outside, body + nut inside
    out.append((cy("y", (0, H.C_D + 1.0, H.BTN_Z), 4.6, 2.0), "black", "back"))
    out.append((cy("y", (0, H.C_D + 3.4, H.BTN_Z), 2.6, 2.8), "button_red", "back"))
    out.append((cy("y", (0, y0 - 1.0, H.BTN_Z), 4.8, 2.0), "metal", "back"))
    out.append((cy("y", (0, y0 - 7.0, H.BTN_Z), 4.0, 10.0), "black", "back"))
    return out


def assembly_c(explode=0.0, front=0.0, cape=True, back=True):
    """Hero Momo. explode: back plate + tray out of the back; front: mask off, cape up"""
    parts = []
    win_cz = H.C_OLED_Z + M.WIN_UP
    post_top = M.FT + M.OLED_GLASS_T + M.OLED_PCB_T + 0.4
    e, f = explode, front
    parts.append((load("C_hero_shell"), np.eye(4), "hero_blue"))
    if back:
        parts.append((load("C_hero_back"), T(0, 1.6 * e, 0), "hero_blue"))
    parts.append((load("C_hero_mask"), T(0, -1.5 * f, win_cz), "black"))
    parts.append((load("oled_retainer"), T(0, post_top + 0.6 * e, H.C_OLED_Z), "grey"))
    parts.append((load("tray"), T(0, 1.2 * e, H.DECK_TOP), "grey"))
    if cape:
        parts.append((load("C_hero_cape"), T(0, 1.6 * e + 0.4 * f, 0.9 * f + 0.6 * e), "hero_red"))
    shell = parts[0][0]
    for m, c, g in electronics(H.DECK_TOP, H.C_OLED_Z, H.C_BUZ_Z, pocket_top(shell, H.C_TOUCH_Y), H.C_TOUCH_Y):
        parts.append((m, T(0, 1.2 * e, 0) if g == "tray" else np.eye(4), c))
    for m, c, g in power_parts():
        if g == "back" and not back:
            continue
        parts.append((m, T(0, 1.6 * e, 0) if g == "back" else T(0, 0.9 * e, 0), c))
    return parts
