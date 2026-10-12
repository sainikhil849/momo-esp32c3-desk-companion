"""Off-screen renders of the Momo bodies with VTK."""
import sys
import numpy as np
import vtk
from vtk.util.numpy_support import numpy_to_vtk, numpy_to_vtkIdTypeArray
import assembly as A


def to_polydata(mesh):
    pts = vtk.vtkPoints()
    pts.SetData(numpy_to_vtk(np.ascontiguousarray(mesh.vertices, dtype=np.float64), deep=True))
    offsets = np.arange(0, 3 * len(mesh.faces) + 1, 3, dtype=np.int64)
    conn = np.ascontiguousarray(mesh.faces, dtype=np.int64).ravel()
    cells = vtk.vtkCellArray()
    cells.SetData(numpy_to_vtkIdTypeArray(offsets, deep=True), numpy_to_vtkIdTypeArray(conn, deep=True))
    pd = vtk.vtkPolyData()
    pd.SetPoints(pts)
    pd.SetPolys(cells)
    n = vtk.vtkPolyDataNormals()
    n.SetInputData(pd)
    n.SetFeatureAngle(32)
    n.SplittingOn()
    n.ConsistencyOn()
    n.Update()
    return n.GetOutput()


def render(parts, out, cam_pos, focal=None, size=(1600, 1200), view_up=(0, 0, 1), zoom=1.0,
           bg=((0.97, 0.97, 0.98), (0.84, 0.86, 0.90)), clip=None, ortho=False, ground=True):
    ren = vtk.vtkRenderer()
    ren.GradientBackgroundOn()
    ren.SetBackground(*bg[1])
    ren.SetBackground2(*bg[0])
    bounds = None
    for mesh, tf, col in parts:
        m = mesh.copy()
        m.apply_transform(tf)
        pd = to_polydata(m)
        if clip is not None:
            pl = vtk.vtkPlane()
            pl.SetOrigin(*clip[0])
            pl.SetNormal(*clip[1])
            cl = vtk.vtkClipPolyData()
            cl.SetInputData(pd)
            cl.SetClipFunction(pl)
            cl.Update()
            pd = cl.GetOutput()
        mapper = vtk.vtkPolyDataMapper()
        mapper.SetInputData(pd)
        act = vtk.vtkActor()
        act.SetMapper(mapper)
        p = act.GetProperty()
        c = A.COL.get(col, col) if isinstance(col, str) else col
        p.SetColor(*c)
        p.SetAmbient(0.22 if col == "white" else 0.15)
        p.SetDiffuse(0.80)
        p.SetSpecular(0.25 if col not in ("glass", "black") else 0.5)
        if not isinstance(col, str):
            p.SetSpecular(0.05)
        p.SetSpecularPower(28)
        if col == "screen":
            p.SetAmbient(1.0)
            p.SetDiffuse(0.0)
            p.LightingOff()
        if clip is not None:
            act.GetProperty().BackfaceCullingOff()
        ren.AddActor(act)
        b = m.bounds
        bounds = b if bounds is None else np.array([np.minimum(bounds[0], b[0]), np.maximum(bounds[1], b[1])])
    if ground:   # soft contact shadow: a few stacked see-through discs
        zmin = bounds[0][2]
        cx, cyy = (bounds[0][0] + bounds[1][0]) / 2, (bounds[0][1] + bounds[1][1]) / 2
        rx, ry = (bounds[1][0] - bounds[0][0]) / 2, (bounds[1][1] - bounds[0][1]) / 2
        for i, k in enumerate((1.25, 1.12, 1.0, 0.9)):
            d = vtk.vtkDiskSource()
            d.SetInnerRadius(0)
            d.SetOuterRadius(1.0)
            d.SetCircumferentialResolution(96)
            tf = vtk.vtkTransform()
            tf.Translate(cx, cyy + 4, zmin - 0.3 + i * 0.02)
            tf.Scale(rx * k, ry * k, 1)
            tp = vtk.vtkTransformPolyDataFilter()
            tp.SetInputConnection(d.GetOutputPort())
            tp.SetTransform(tf)
            pm = vtk.vtkPolyDataMapper()
            pm.SetInputConnection(tp.GetOutputPort())
            pa = vtk.vtkActor()
            pa.SetMapper(pm)
            pa.GetProperty().SetColor(0.35, 0.37, 0.42)
            pa.GetProperty().SetOpacity(0.07)
            pa.GetProperty().LightingOff()
            ren.AddActor(pa)
    lk = vtk.vtkLightKit()
    lk.SetKeyLightIntensity(0.80)
    lk.SetKeyLightElevation(28)
    lk.SetKeyLightAzimuth(-18)
    lk.SetKeyToFillRatio(2.2)
    lk.SetKeyToHeadRatio(2.6)
    lk.SetFillLightWarmth(0.55)
    lk.AddLightsToRenderer(ren)
    rw = vtk.vtkRenderWindow()
    rw.SetOffScreenRendering(1)
    rw.SetSize(*size)
    rw.SetMultiSamples(8)
    rw.AddRenderer(ren)
    cam = ren.GetActiveCamera()
    if focal is None:
        focal = ((bounds[0] + bounds[1]) / 2).tolist()
    cam.SetFocalPoint(*focal)
    cam.SetPosition(*cam_pos)
    cam.SetViewUp(*view_up)
    cam.SetViewAngle(24)
    if ortho:
        cam.ParallelProjectionOn()
    ren.ResetCameraClippingRange()
    cam.Zoom(zoom)
    rw.Render()
    w2i = vtk.vtkWindowToImageFilter()
    w2i.SetInput(rw)
    w2i.SetInputBufferTypeToRGB()
    w2i.ReadFrontBufferOff()
    w2i.Update()
    wr = vtk.vtkPNGWriter()
    wr.SetFileName(out)
    wr.SetInputConnection(w2i.GetOutputPort())
    wr.Write()
    print("wrote", out)


def without(parts, *names):
    return [p for p in parts if p[0].metadata.get("name", "") not in names]


def shifted(parts, dx):
    return [(m, A.T(dx, 0, 0) @ t, c) for m, t, c in parts]


if __name__ == "__main__":
    import os
    os.makedirs("renders", exist_ok=True)
    which = sys.argv[1] if len(sys.argv) > 1 else "all"
    front = (-150, -230, 150)
    if which in ("a", "all"):
        render(A.assembly_a(), "renders/A_cube_front.png", front)
        render(A.assembly_a(docked=True), "renders/A_cube_on_dock.png", (150, -240, 140))
        render(A.assembly_a(), "renders/A_cube_back.png", (170, 300, 150))
        render(without(A.assembly_a(), "A_cube_back"), "renders/A_cube_inside.png", (150, 260, 170))
        render(A.assembly_a(28), "renders/A_cube_exploded_back.png", (-240, 230, 190), zoom=0.95)
        render(A.assembly_a(0, front=28), "renders/A_cube_exploded_front.png", (-290, -170, 120), zoom=0.9)
        render(A.assembly_a(), "renders/A_cube_top.png", (0, -60, 300), view_up=(0, 1, 0))
    if which in ("b", "all"):
        render(A.assembly_b(), "renders/B_pod_front.png", front)
        render(A.assembly_b(), "renders/B_pod_side.png", (-300, 28, 40))
        render(A.assembly_b(), "renders/B_pod_back.png", (170, 300, 150))
        render(without(A.assembly_b(), "B_pod_back"), "renders/B_pod_inside.png", (150, 260, 170))
        render(A.assembly_b(28), "renders/B_pod_exploded_back.png", (-240, 230, 190), zoom=0.95)
        render(A.assembly_b(0, front=28), "renders/B_pod_exploded_front.png", (-290, -170, 120), zoom=0.9)
    if which in ("c", "all"):
        render(A.assembly_c(), "renders/C_hero_front.png", front)
        render(A.assembly_c(), "renders/C_hero_side.png", (-300, 28, 40))
        render(A.assembly_c(), "renders/C_hero_back.png", (170, 300, 150))
        render(A.assembly_c(cape=False), "renders/C_hero_back_no_cape.png", (120, 320, 90))
        render(A.assembly_c(cape=False, back=False), "renders/C_hero_inside.png", (150, 260, 170))
        render(A.assembly_c(28), "renders/C_hero_exploded_back.png", (-240, 230, 190), zoom=0.95)
        render(A.assembly_c(0, front=28), "renders/C_hero_exploded_front.png", (-290, -170, 120), zoom=0.9)
        render(A.assembly_c(), "renders/C_hero_top.png", (0, -60, 300), view_up=(0, 1, 0))
    if which in ("family", "all"):
        fam = shifted(A.assembly_a(docked=True), -92) + shifted(A.assembly_c(), 0) + shifted(A.assembly_b(), 92)
        render(fam, "renders/Momo_family.png", (-70, -420, 170), size=(1920, 1080))
