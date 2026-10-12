"""Lay the parts out on Bambu Lab A1 mini plates (180 x 180 mm), print-oriented.

Writes  plates/*.3mf   (one file per plate, every part a separate object, already placed)
        stl_print_ready/*.stl  (each part on its own, rotated the way it should print)
Run:  python plates.py   (after momo_enclosures.py)
"""
import os
import numpy as np
import trimesh
import assembly as A
import checks as C

BED = 180.0
PLATES = {   # file name: (colour for the render, [(part, x, y, rotate_z_deg), ...])
    "A_Cube_plate1_white": ("white", [("A_cube_shell", 46, 128, 0), ("A_cube_back", 132, 128, 0),
                                      ("tray", 42, 48, 0), ("oled_retainer", 128, 40, 0)]),
    "A_Cube_plate2_coral": ("accent_a", [("A_cube_dock", 90, 112, 0), ("A_cube_ear", 62, 40, 0),
                                         ("A_cube_ear", 118, 40, 0)]),
    "A_Cube_plate3_black": ("black", [("A_cube_visor", 90, 90, 0)]),
    "B_Pod_plate1_white":  ("white", [("B_pod_shell", 47, 126, 0), ("B_pod_back", 130, 130, 0),
                                      ("tray", 42, 46, 0), ("oled_retainer", 128, 40, 0)]),
    "B_Pod_plate2_mint":   ("accent_b", [("B_pod_antenna", 78, 90, 0), ("B_pod_antenna", 102, 90, 0)]),
    "B_Pod_plate3_black":  ("black", [("B_pod_visor", 90, 90, 0)]),
    "C_Hero_plate1_blue":  ("hero_blue", [("C_hero_shell", 47, 128, 0), ("C_hero_back", 132, 128, 0),
                                          ("tray", 42, 46, 0), ("oled_retainer", 128, 40, 0)]),
    "C_Hero_plate2_red":   ("hero_red", [("C_hero_cape", 90, 90, 0)]),
    "C_Hero_plate3_black": ("black", [("C_hero_mask", 90, 90, 0)]),
}


def placed(name, x, y, rz):
    m = C.print_pose(name)
    m.apply_transform(A.R((0, 0, 1), rz))
    c = (m.bounds[0] + m.bounds[1]) / 2
    m.apply_translation((x - c[0], y - c[1], -m.bounds[0][2]))
    return m


def build():
    os.makedirs("plates", exist_ok=True)
    os.makedirs("stl_print_ready", exist_ok=True)
    for name in C.PRINT_POSE:
        C.print_pose(name).export(f"stl_print_ready/{name}.stl")
    layout = {}
    for plate, (col, items) in PLATES.items():
        scene = trimesh.Scene()
        meshes = []
        for k, (name, x, y, rz) in enumerate(items):
            m = placed(name, x, y, rz)
            lo, hi = m.bounds
            assert lo[0] >= 2 and lo[1] >= 2 and hi[0] <= BED - 2 and hi[1] <= BED - 2 and hi[2] <= BED, (plate, name, lo, hi)
            for other in meshes:          # no two parts touch (3 mm gap)
                olo, ohi = other.bounds
                gap = max(olo[0] - hi[0], lo[0] - ohi[0], olo[1] - hi[1], lo[1] - ohi[1])
                assert gap >= 3.0, (plate, name, gap)
            meshes.append(m)
            scene.add_geometry(m, node_name=f"{name}_{k + 1}", geom_name=f"{name}_{k + 1}")
        scene.export(f"plates/{plate}.3mf")
        layout[plate] = (col, meshes)
        print(f"plate {plate:22s} {len(meshes)} parts  OK")
    return layout


def render_plates(layout):
    import render as Rn
    os.makedirs("renders", exist_ok=True)
    for plate, (col, meshes) in layout.items():
        bed = trimesh.creation.box(extents=(BED, BED, 1.0))
        bed.apply_translation((BED / 2, BED / 2, -0.5))
        parts = [(bed, np.eye(4), (0.20, 0.21, 0.23))] + [(m, np.eye(4), col) for m in meshes]
        Rn.render(parts, f"renders/{plate}.png", (90, -175, 400), focal=(90, 92, 10), ground=False, size=(1200, 1000), zoom=0.8)


if __name__ == "__main__":
    render_plates(build())
