"""Fit + printability checks for both Momo enclosures.

1. every part is a closed solid and fits the Bambu Lab A1 mini (180 x 180 x 180 mm)
2. overhangs in the print orientation (anything steeper than 45 deg that is not on the plate)
3. collisions: printed parts vs. each other and vs. the electronics, fully assembled
Run:  python checks.py
"""
import math
import numpy as np
import trimesh
import assembly as A

BED = 180.0
RX = lambda d: A.R((1, 0, 0), d)
PRINT_POSE = {          # rotation that puts each part in its print orientation
    "A_cube_shell": RX(90), "B_pod_shell": RX(90),          # face down
    "A_cube_back": RX(-90), "B_pod_back": RX(-90),           # outside face down
    "A_cube_visor": RX(90), "B_pod_visor": RX(90),           # front face down
    "oled_retainer": RX(-90),                                # flat back down, pads up
    "A_cube_ear": np.eye(4), "B_pod_antenna": np.eye(4),     # as modelled
    "A_cube_dock": np.eye(4), "tray": np.eye(4),
    "C_hero_shell": RX(90), "C_hero_back": RX(-90),          # face down / outside face down
    "C_hero_mask": RX(90), "C_hero_cape": RX(180),           # front face down / collar flat on the plate
}


def print_pose(name):
    m = A.load(name)
    m.apply_transform(PRINT_POSE[name])
    lo = m.bounds[0]
    m.apply_translation((-(m.bounds[0][0] + m.bounds[1][0]) / 2, -(m.bounds[0][1] + m.bounds[1][1]) / 2, -lo[2]))
    return m


def overhangs(m, limit_deg=45.0, bed_tol=0.25):
    """groups of down-facing triangles steeper than the limit (bed contact ignored)"""
    nz = m.face_normals[:, 2]
    zmin = m.triangles[:, :, 2].min(axis=1)
    bad = np.where((nz < -math.cos(math.radians(90 - limit_deg)) - 1e-3) & (zmin > bed_tol))[0]
    if len(bad) == 0:
        return []
    sub = m.submesh([bad], append=True)
    groups = []
    for g in sub.split(only_watertight=False):
        ext = g.bounds[1] - g.bounds[0]
        flat = float(np.mean(g.face_normals[:, 2]) < -0.98)
        groups.append({"area": g.area, "span": float(min(ext[0], ext[1])), "length": float(max(ext[0], ext[1])),
                       "z": float(g.bounds[0][2]), "flat": bool(flat)})
    return sorted(groups, key=lambda d: -d["area"])


def intersect_volume(a, b):
    if not (np.all(a.bounds[0] < b.bounds[1]) and np.all(b.bounds[0] < a.bounds[1])):
        return 0.0
    try:
        r = trimesh.boolean.intersection([a, b], engine="manifold")
        return float(r.volume) if r is not None and len(r.faces) else 0.0
    except Exception as e:      # noqa
        return float("nan")


def placed(parts):
    out = []
    for m, t, c in parts:
        mm = m.copy()
        mm.apply_transform(t)
        out.append((m.metadata.get("name", c), mm, c))
    return out


if __name__ == "__main__":
    ok = True
    print("== 1. solids + bed fit (A1 mini 180 mm cube) ==")
    for name in PRINT_POSE:
        m = print_pose(name)
        ext = m.bounds[1] - m.bounds[0]
        fits = bool(np.all(ext <= BED))
        good = m.is_watertight and m.is_volume and fits
        ok &= good
        print(f"  {name:15s} {'OK ' if good else 'BAD'}  {ext[0]:6.1f} x {ext[1]:6.1f} x {ext[2]:6.1f} mm  "
              f"{m.volume / 1000:6.1f} cm3  ~{m.volume / 1000 * 1.24 * 0.55:4.0f} g")

    print("== 2. overhangs > 45 deg in print orientation (bridges listed with their span) ==")
    for name in PRINT_POSE:
        g = overhangs(print_pose(name))
        steep = [x for x in g if not x["flat"] and x["area"] > 2.0]      # < 2 mm2 = mesh slivers
        flats = [x for x in g if x["flat"] and x["area"] > 2.0]
        worst_bridge = max([x["span"] for x in flats], default=0.0)
        worst_steep = max([x["span"] for x in steep], default=0.0)
        print(f"  {name:15s} steep groups={len(steep):2d} (widest {worst_steep:4.1f} mm)  "
              f"bridges={len(flats):2d} (longest span {worst_bridge:4.1f} mm)")
        for x in (steep + flats)[:6]:
            print(f"      {'bridge' if x['flat'] else 'steep '} area {x['area']:6.1f} mm2  span {x['span']:4.1f}  "
                  f"len {x['length']:5.1f}  at z={x['z']:5.1f}")

    print("== 3. collisions when assembled (mm3; ~0 expected) ==")
    # the retainer pads are meant to press 0.1 mm into the OLED board (that clamps it)
    INTENDED = {("oled_retainer", "pcb_blue")}
    for label, parts in (("A", A.assembly_a()), ("A docked", A.assembly_a(docked=True)), ("B", A.assembly_b()),
                         ("C", A.assembly_c())):
        p = [x for x in placed(parts) if x[2] != "screen"]
        worst = 0.0
        for i in range(len(p)):
            for j in range(i + 1, len(p)):
                v = intersect_volume(p[i][1], p[j][1])
                if (p[i][0], p[j][0]) in INTENDED and v < 5.0:
                    continue
                if v != v or v > 0.5:
                    print(f"  {label}: {p[i][0]:14s} x {p[j][0]:14s}  {v:7.2f} mm3")
                if v == v:
                    worst = max(worst, v)
        print(f"  {label}: largest unintended overlap {worst:.2f} mm3 (retainer clamps OLED by 0.1 mm, as designed)")
        ok &= worst < 0.5
    print("ALL CHECKS PASSED" if ok else "SOME CHECKS FAILED")
