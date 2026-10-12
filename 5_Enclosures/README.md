# Momo enclosures — three 3D-printable bodies for the Bambu Lab A1 mini

<p align="center">
  <img src="pictures/Momo_family.png" alt="Momo Cube on its dock, Hero Momo and Momo Pod" width="820">
</p>

Three complete bodies for the same Momo electronics. All print on the **Bambu Lab A1 mini** (180 × 180 × 180 mm) in **PLA with no supports**, and all use the same electronics tray, so you can move Momo from one body to another in a few minutes.

**New in Momo 2.2: C — Hero Momo**, the body for the cordless Momo: it has room for the 500 mAh battery and the USB-C charger board, two USB-C sockets (CHARGE and CODE), the power switch and the back button — and a mini-superhero costume. The Cube and the Pod stay as they were (USB-powered Momo, no battery).

| | **C — Hero Momo** (new) |
|---|---|
| Look | Rounded cube (14 mm corners on top, 5 mm at the bottom), black **domino-mask visor**, a **star** touch spot on the head, **lightning bolts** on both sides, a smile of sound holes, a red **cape** with a big star window |
| Size | 70 × 61 × 72 mm with the cape (body 70 × 58 × 70 mm) |
| Inside | Electronics deck: the same slide-out tray rides on top; under it a **battery bay** (fits 500 mAh LiPos up to 7 × 30 × 42 mm) and a **slot for the TP4056 board** that lines its USB-C up with the CHARGE hole |
| Back | **CHARGE** USB-C (charger), **CODE** USB-C (ESP32), **power slide switch** with ON / POWER labels, **7 mm back button** right in the middle of the cape's star |
| Touch spot | Star on top, roof thinned to 0.8 mm over the TTP223 |
| Stands on | 4 rubber feet |
| Colours | Sky blue (or white) body, black mask, red cape — any colours you like |
| Printed parts | 6 (shell, back plate, mask visor, cape, tray, OLED retainer) |
| Filament | about 75 g PLA |
| Feels like | A little hero guarding your desk — the cape and mask make people smile |

| | **A — Momo Cube** | **B — Momo Pod** |
|---|---|---|
| Look | Soft cube with 12 mm round corners, coral "headphone" ears, coral desk dock | Rounded helmet that gets narrower towards the back, two mint antennas |
| Size | 79 × 59 × 62 mm (with ears), 70 mm tall on the dock | 70 × 59 × 68 mm (with antennas) |
| Touch spot | Heart on top | Ring + dot on top |
| Stands on | Dock that tilts it 6° back towards your eyes (4 rubber feet) | 4 rubber feet |
| Colours | White body, black visor, coral ears and dock | White body, black visor, mint antennas |
| Printed parts | 8 (shell, back, visor, 2 ears, dock, tray, retainer) | 7 (shell, back, visor, 2 antennas, tray, retainer) |
| Filament | about 70 g PLA (dock ~25 g of it) | about 45 g PLA |
| Feels like | A little desk gadget that belongs next to a keyboard | A friendly character you want to pick up |

> Bambu Studio shows the exact grams and print time when you slice. The numbers above are estimates from the part volumes.

---

## Contents

0. [Design C — Hero Momo: print and assembly](#design-c--hero-momo-print-and-assembly)
1. [Pictures](#pictures)
2. [What makes these bodies nice to live with](#what-makes-these-bodies-nice-to-live-with)
3. [Why these choices — and why not the others](#why-these-choices--and-why-not-the-others)
4. [What to print](#what-to-print)
5. [Print settings (Bambu Studio, A1 mini)](#print-settings-bambu-studio-a1-mini)
6. [What else you need](#what-else-you-need)
7. [Before you print everything: quick fit check](#before-you-print-everything-quick-fit-check)
8. [Assembly](#assembly)
9. [Changing the design](#changing-the-design)
10. [How the designs were checked](#how-the-designs-were-checked)
11. [Files](#files)

---

## Design C — Hero Momo: print and assembly

| Hero Momo | Back: cape, back button, CHARGE / CODE / switch | Cape off |
|---|---|---|
| ![](pictures/C_hero_front.png) | ![](pictures/C_hero_back.png) | ![](pictures/C_hero_back_no_cape.png) |
| **Back plate off: battery, charger, tray on the deck** | **Exploded (back)** | **Exploded (front): mask and cape** |
| ![](pictures/C_hero_inside.png) | ![](pictures/C_hero_exploded_back.png) | ![](pictures/C_hero_exploded_front.png) |
| **Side** | **Top: the star touch spot** | |
| ![](pictures/C_hero_side.png) | ![](pictures/C_hero_top.png) | |

Plates (exactly as they go on the A1 mini plate):

| C — blue (or white) | C — red | C — black |
|---|---|---|
| ![](pictures/C_Hero_plate1_blue.png) | ![](pictures/C_Hero_plate2_red.png) | ![](pictures/C_Hero_plate3_black.png) |

### What to print (folder `C_Hero_Momo`)

| Plate file | Colour | Parts | Print orientation |
|---|---|---|---|
| `C_Hero_plate1_blue.3mf` | Sky blue or white | shell, back plate, tray, OLED retainer | shell face down, back plate outside face down |
| `C_Hero_plate2_red.3mf` | Red | cape | upside down: the collar flat on the plate, the cape standing up |
| `C_Hero_plate3_black.3mf` | Black | mask visor | front face down |

Same print settings as below (0.20 mm, 3 walls, 15 % gyroid, **no supports**). The cape is a thin standing wall: print it at normal speed with the part cooling fan on; a brim is not needed.

### Extra parts for Hero Momo

| Part | Qty | Used for |
|---|---|---|
| M2 × 8 mm self-tapping screws | 2 | Back plate (from underneath) |
| M2 × 6 mm self-tapping screws | 2 | OLED retainer |
| 500 mAh LiPo (502035 / 502030), TP4056 USB-C board with protection | 1 each | See `HARDWARE_GUIDE.md` |
| SS12D00 slide switch with a **4–5 mm lever** (G4/G5) | 1 | Power |
| 7 mm momentary push button (PBS-110 type) | 1 | Back button |
| Rubber bumpers 8 mm, foam tape, hot glue or super glue | | Feet, battery, charger board, switch |

### Hero sizes you can change (`source/momo_hero.py`)

| Parameter | Default | Meaning |
|---|---|---|
| `BAT` | 32 × 7 mm | Battery bay width × height (length = up to the deck end, 44 mm) |
| `TP_W`, `TP_L`, `TP_X` | 17.5, 28.0, 16.5 mm | TP4056 board size and position (USB-C lines up with the CHARGE hole) |
| `BTN_HOLE`, `BTN_Z` | 7.2, 48.5 mm | Back button hole and height (centre of the cape's star) |
| `SW_BODY`, `SW_SLOT` | 8.8 × 3.8 × 3.8, 4.4 × 2.2 mm | Slide switch pocket and lever slot |
| `STAR_R`, `STAR_r` | 15.5, 7.8 mm | Cape star window (fits a button collar up to 12 mm) |
| `CAPE_HEM_Z` | 31.5 mm | Lowest point of the cape (stays above the CODE socket) |

### Assembly (about 45 minutes after the wiring is soldered)

1. **Mask.** Press the black mask visor onto the face — its ring finds the pocket by itself. 3–4 drops of super glue from behind.
2. **Screen.** OLED into its frame (glass to the window, ribbon down), retainer across the back, 2 × M2×6. Snug, not tight.
3. **Buzzer** into the cup behind the smile (a dot of glue). **TTP223** into the pocket under the star, touch pad up, foam tape.
4. **Battery and charger** (looking into the open back): the battery lies flat on the floor in the **right** bay under the deck, its wires towards you. The TP4056 board goes into the **left** slot, components up, USB-C towards you, until it touches the 45° stop. A dab of hot glue or foam tape under it. Lead all power wires up through the open space behind the deck.
5. **Tray.** DS3231 upright in the groove, MPU6050 upright between its posts, ESP32-C3 into the rails (USB-C at the back). Slide the tray in **on top of the deck** until it stops at the ribs.
6. **Back plate.** Push the slide switch into its pocket from the inside (lever through the slot, a drop of glue on the body's sides). Screw the back button into the 7 mm hole (nut inside). Hook the plate's tab under the roof, swing it down, 2 × **M2×8** from underneath.
7. **Test:** switch ON → Momo starts from the battery. Plug a cable into **CHARGE** → the charging screen. **CODE** is for uploading.
8. **Cape.** Push its two pegs into the holes on the head (a little firm on purpose). The star window frames the back button.
9. **Feet:** 4 rubber bumpers in the round recesses.

To open Hero Momo: lift the cape off, remove the 2 screws underneath, lift the back plate off the hook, slide the tray out. The battery and charger slide out of the back too.

---

## Pictures

| A — Momo Cube | A — on its dock | A — exploded (front) |
|---|---|---|
| ![](pictures/A_cube_front.png) | ![](pictures/A_cube_on_dock.png) | ![](pictures/A_cube_exploded_front.png) |
| **A — back (USB-C, vents)** | **A — back cover off** | **A — tray slides out** |
| ![](pictures/A_cube_back.png) | ![](pictures/A_cube_inside.png) | ![](pictures/A_cube_exploded_back.png) |

| B — Momo Pod | B — side | B — exploded (front) |
|---|---|---|
| ![](pictures/B_pod_front.png) | ![](pictures/B_pod_side.png) | ![](pictures/B_pod_exploded_front.png) |
| **B — back** | **B — back cover off** | **B — tray slides out** |
| ![](pictures/B_pod_back.png) | ![](pictures/B_pod_inside.png) | ![](pictures/B_pod_exploded_back.png) |

Print plates (exactly as they go on the A1 mini plate):

| A — white | A — coral | A — black |
|---|---|---|
| ![](pictures/A_Cube_plate1_white.png) | ![](pictures/A_Cube_plate2_coral.png) | ![](pictures/A_Cube_plate3_black.png) |
| **B — white** | **B — mint** | **B — black** |
| ![](pictures/B_Pod_plate1_white.png) | ![](pictures/B_Pod_plate2_mint.png) | ![](pictures/B_Pod_plate3_black.png) |

The pictures are rendered from the real print files (with simple stand-ins for the electronics), not drawn by hand.

---

## What makes these bodies nice to live with

| Feature | Why it matters to the person using Momo |
|---|---|
| **Black visor with a 45° bevel** | The OLED's own black glass, its white frame and the window edge all disappear. Only the glowing eyes are visible, so the face looks like it floats. |
| **Self-centring visor** | A ring on the back of the visor drops into a pocket in the face. It can only sit in one place, so it is always straight — no lining up by eye. |
| **Smile = speaker** | Five holes in a smile under the visor, with the buzzer sealed in a cup right behind them. The sound comes out of Momo's "mouth", louder and clearer than through a closed wall. |
| **Marked touch spot** | The heart (Cube) or ring (Pod) sits exactly above the TTP223. The roof is thinned to about 1.6–2.3 mm there, so a light pet always works. |
| **Slide-out tray** | ESP32-C3, DS3231 and MPU6050 sit on one tray that slides in from the back and stops by itself. Wire it on the desk, slide it in. No fiddling inside a small box. |
| **No visible screws** | The back cover hooks into the roof at the top and is held by 2 screws from underneath. From the front and sides, Momo is one smooth piece. |
| **USB-C lined up for you** | The tray rails hold the ESP32-C3 at the exact height of the hole in the back cover, so the cable goes straight in, even for thick plugs (13.4 × 7.6 mm opening). |
| **Vents at the back** | The ESP32-C3 gets warm with Wi-Fi on. The vents let that heat out, so it does not warm up the clock and motion sensors. |
| **Tilt works the right way** | The MPU6050 stands upright with its long side left–right, so "tilt left/right" in the games and music controls matches what Momo's firmware expects. |
| **Does not slide when you tap it** | Rubber feet (Pod, and under the Cube's dock). |
| **Dock (Cube)** | Leans Momo back 6°, so the screen faces your eyes when you sit at a desk. Momo lifts straight out for the tilt and shake games and drops back in. |
| **Same tray for both** | Build once; change bodies later without unsoldering anything. |

---

## Why these choices — and why not the others

### Material: PLA

| Option | Verdict |
|---|---|
| **PLA (chosen)** | Prints the most accurately on the A1 mini, which is an open printer without a heated chamber. Sharp details (visor bevel, smile holes, heart), many colours, matte versions hide layer lines. |
| PETG | Takes more heat, but strings and prints less crisp small details. Worth it only if Momo will sit in a hot car or in direct summer sun. |
| ABS / ASA | Warp and crack on an open printer like the A1 mini, and give off fumes. Not needed: the electronics use well under 1 W. |
| TPU | Too soft for a shell. (Fine for home-made feet, though.) |
| Resin | Needs a resin printer and washing/curing. The A1 mini is an FDM printer. |

PLA softens at about 55–60 °C, so keep Momo out of a closed car in summer.

### Printing without supports

| Option | Verdict |
|---|---|
| **Designed to print without supports (chosen)** | Each part has one flat side for the plate, and every shape grows upwards at 45° or less. No support marks on the face, no picking support out of small holes, less filament, shorter prints. |
| Supports | Leave rough marks, and inside a closed shell they are hard or impossible to remove. |

How this was done: the shells print face-down (the face copies the plate's finish), the covers print outside-face-down, and everything inside the shell grows from the face or a wall. Features that would start in mid-air (for example the rim around the touch sensor) have a 45° ramp under them.

### Visor as a separate black part

| Option | Verdict |
|---|---|
| **Separate black visor (chosen)** | Deep black around the eyes on any A1 mini, with or without the AMS lite. A small, quick print. |
| Multi-colour print with the AMS lite | Works only if you have the AMS lite, purges a lot of filament for a small black area, and the colour would only be on the outside layer. |
| Painted face | Hard to get clean edges at this size, and paint chips over time. |
| Visor sunk into a pocket in the face | Looks flush, but the shell prints face-down, so the bottom of the pocket would have to bridge across a 54 mm hole and would sag. The raised visor with a centring ring avoids that and still looks deliberate. |

### Electronics on a tray

| Option | Verdict |
|---|---|
| **Slide-out tray (chosen)** | Wire and test everything outside the shell. Modules are held at the right height and angle. The same tray fits both bodies. |
| Glue modules to the inside walls | Hard to reach, hard to repair, and the round corners leave little flat space. |
| One custom PCB | The cleanest result in the long run, but it needs PCB design and ordering. Good for a "Momo 3" (see the roadmap in the main README). |

### Holding things together

| Option | Verdict |
|---|---|
| **Top hook + 2 screws underneath (chosen)** | Opens and closes many times without wearing out. Nothing visible from the front. |
| Snap-fits only | PLA is stiff. Snaps either crack after a few openings or hold too loosely. |
| Heat-set inserts | Very strong, but need a soldering iron tip and add parts. M2 self-tapping screws in PLA are plenty for a back cover that is opened now and then. |
| Glue the cover | Then you can never get back in to change a module. |

### Hero Momo: why it is built this way

| Choice | Why | Why not the others |
|---|---|---|
| **Battery under a deck** | The battery and charger lie flat and stay put; the tray slides in on top exactly as in the Cube and Pod, so wiring and repairs work the same | Battery on the tray: heavy, stresses the wires every time you open Momo. Battery in a separate base: bigger, and a cable between the two parts |
| **Charger slot with a 45° stop** | The board's USB-C lands exactly on the CHARGE hole; the 45° stop prints without supports | Glueing the board by eye: the socket ends up behind the wall |
| **Two USB-C sockets** | CHARGE always charges; CODE always reaches the ESP32 for uploads and the Serial Monitor | One socket for both needs a USB switch or a power-path chip |
| **Back button in the cape's star** | The hero emblem is the button — easy to find by touch, and it looks like a design detail, not an afterthought | A button under the cape: hidden. A button on the side: Momo slides when you press it |
| **Cape on two pegs** | Lifts off in a second to open the back; prints upside down with no supports | A cape glued to the back plate: you could not open Momo |
| **Mask as the visor** | The same self-centring visor idea, shaped as a domino mask, so the hero look costs no extra part | A painted mask: messy edges at this size |
| **Slide switch pocket on the back plate** | Off means off; the plate is thinned to 1.6 mm there so a 4–5 mm lever sticks out about 2.4 mm | A switch on the side wall: the wires would cross the tray |
| **Original costume** | A mask, a cape, a star and bolts are the general idea of a superhero, so the design is yours to share | Logos, suits and faces of famous heroes belong to their owners |

### Shape

| Option | Verdict |
|---|---|
| **Cube (A)** | Clean and simple, sits well next to a monitor and keyboard. The ears give it character without making it toy-like. |
| **Pod (B)** | Softer and more like a character. The narrower back makes it look small from the side and keeps the face as the biggest part. |
| **Hero (C)** | A cube with soft top corners and a flat bottom: the flat bottom gives the battery bay its width, the round top makes it friendly, and the costume gives it a story. |
| Sphere / egg | Rolls, and a perfect sphere cannot print without supports. |
| A copy of an existing desk robot | Not ours to copy. Both designs here are original. |

### CAD tool: CadQuery (Python)

| Option | Verdict |
|---|---|
| **CadQuery (chosen)** | The whole design is a Python script with named parameters. Change one number (for example your OLED's size) and every part updates, including the fit checks and plates. Free, works on Windows. |
| Fusion 360 | Excellent and more visual, but a cloud account, and the design is not a text file you can review and version on GitHub. |
| OpenSCAD | Also code, but no real fillets and no STEP export, so the soft rounded look is hard to get. |
| Onshape | Great in the browser; free plan makes your documents public. |

STEP files are included, so you can still open and change the parts in Fusion 360, Onshape, FreeCAD or SolidWorks.

---

## What to print

Open the **.3mf** files in Bambu Studio. Each one is a full plate, already placed and turned the right way. If Bambu Studio says the file is not from Bambu Lab, choose to load the geometry only.

### A — Momo Cube (folder `A_Momo_Cube`)

| Plate file | Colour | Parts |
|---|---|---|
| `A_Cube_plate1_white.3mf` | White | shell, back cover, tray, OLED retainer |
| `A_Cube_plate2_coral.3mf` | Coral / orange | dock, 2 ears |
| `A_Cube_plate3_black.3mf` | Black | visor |

### B — Momo Pod (folder `B_Momo_Pod`)

| Plate file | Colour | Parts |
|---|---|---|
| `B_Pod_plate1_white.3mf` | White | shell, back cover, tray, OLED retainer |
| `B_Pod_plate2_mint.3mf` | Mint / teal | 2 antennas |
| `B_Pod_plate3_black.3mf` | Black | visor |

The separate **STL** files in each `STL` folder are turned the right way for printing, if you prefer to place parts yourself. The tray and the retainer can be any colour — nobody sees them.

---

## Print settings (Bambu Studio, A1 mini)

| Setting | Value | Why |
|---|---|---|
| Printer / nozzle | Bambu Lab A1 mini, 0.4 mm | |
| Filament | Bambu PLA Basic or **PLA Matte** | Matte looks more like a finished product and hides layer lines |
| Process | **0.20 mm Standard** (0.16 mm Fine for the shell if you want it smoother) | |
| Wall loops | **3** | Walls are 2.4 mm; 3 loops make them solid |
| Sparse infill | **15 %, gyroid** | Only the dock really has infill |
| Supports | **Off** | Not needed anywhere |
| Brim | Off (turn it on for the antennas if your plate is not freshly cleaned) | |
| Seam | Aligned or Back | Keeps the seam off the face |
| Plate | Textured PEI → matte, slightly grainy face. Smooth PEI → shiny face | The face prints on the plate, so it copies the plate's finish |
| Elephant foot compensation | Leave the default (0.15 mm) | Keeps the visor ring and the face pocket at the right size |

Tips:
- Clean the plate with dish soap and water before the shell: the face is the most important surface and it is on the plate.
- Print the black visor on its own. It is small and quick, and you can reprint it in another colour (coral, mint, transparent) for a different look.

---

## What else you need

| Part | Qty per Momo | Used for |
|---|---|---|
| M2 × 6 mm self-tapping screws, pan head | 4 | 2 × back cover (from underneath), 2 × OLED retainer. Do not use longer screws for the retainer. |
| Rubber bumpers, 8 mm, self-adhesive | 4 | Under the Cube's dock, or under the Pod |
| 1.75 mm filament (offcut) | 4 pins, about 4.5 mm long | Cube ears only: they locate and hold the ears |
| Double-sided foam tape | a small piece | TTP223 in its pocket; optional for the DS3231 and MPU6050 on the tray |
| Super glue (CA), a few drops | | Visor, ears / antennas, buzzer in its cup |
| Thin wire (28–30 AWG silicone), 8–10 cm pieces | about 20 | Wiring (see the wiring table in the main README) |

Electronics are the same as in the main README: ESP32-C3 Super Mini, 1.3" SH1106 OLED (4 pins, I2C), TTP223, MPU6050 (GY-521), DS3231 module, 12 mm passive buzzer.

---

## Before you print everything: quick fit check

Modules from different sellers are not all the same size. The design uses these sizes (mm):

| Module | Size used | Where to change it (`source/momo_enclosures.py`) |
|---|---|---|
| OLED board | 35.4 × 33.5, board 1.6 thick, glass 1.8 thick | `OLED_PCB`, `OLED_PCB_T`, `OLED_GLASS_T` |
| Visible screen window | 31.5 × 18.0 | `WIN` |
| OLED screw holes (centre to centre ÷ 2) | 22.3 | `RET_X` |
| ESP32-C3 Super Mini | 18.0 × 22.6, 1.0 thick | `ESP_W`, `ESP_L`, `ESP_T` |
| MPU6050 (GY-521) | 21.2 × 16.0 | `MPU_W`, `MPU_H` |
| DS3231 module | 38.0 × 22.0 | `DS_W`, `DS_H` |
| Passive buzzer | Ø12.0, 9.5 tall | `BUZ_D`, `BUZ_L` |
| TTP223 module | 15.0 × 11.0 | `TTP` |

1. Measure your modules with a ruler or caliper.
2. Print the **tray** and the **black visor** first (two small, quick prints). Check that the ESP32-C3, MPU6050 and DS3231 slide into the tray, and that the visor's ring fits the OLED window.
3. If something is off, change the number, run `python make_all.py`, and print again. Then print the shell.

---

## Assembly

About 30 minutes, after the wiring is soldered.

1. **Visor.** Push the visor into the face — the ring on its back finds the pocket by itself. Fix it with 3–4 small drops of super glue on the back (or thin double-sided tape).
2. **Screen.** From inside, lay the OLED in its frame: glass towards the window, the ribbon cable at the bottom (the frame has a gap there). Put the retainer across the back of the board and screw it to the two posts with 2 × M2×6. Snug, not tight — it presses on glass.
3. **Buzzer.** Push it into the round cup behind the smile, pins towards the back. A dot of glue keeps it there.
4. **Touch sensor.** Stick the TTP223 into the pocket under the heart/ring, touch pad facing up, with foam tape.
5. **Wiring.** Wire everything as in the main README. Leave 8–10 cm of wire between the face parts (OLED, buzzer, touch) and the tray, so the tray can slide out while everything stays connected.
6. **Tray.**
   - DS3231: stand it in the groove at the front of the tray, coin cell facing forward (towards the face).
   - MPU6050: stand it between its two slotted posts, long side left–right, pin header at the bottom.
   - ESP32-C3: slide it into the rails from the back, components up, USB-C at the back, until it stops.
7. **Test before closing.** Plug in USB-C and check the face, touch, sound and tilt (menu → games → Catch). If tilt left/right feels reversed, turn the MPU6050 around so its other side faces the front.
8. **Tray in.** Slide the tray in from the back until it stops at the ribs on the floor.
9. **Back cover.** Hook the tab at its top into the slot under the roof, swing it down, and screw it from underneath with 2 × M2×6.
10. **Ears or antennas.**
    - Cube: glue two 4.5 mm pieces of filament into the holes of each ear, then press the ears onto the round seats on the sides.
    - Pod: put a drop of glue on each antenna stalk and push it into the hole in the top.
11. **Feet.** Stick the rubber bumpers into the round recesses (under the Pod, or under the Cube's dock).

To open Momo later: remove the 2 screws underneath, lift the back cover off the hook, and slide the tray out.

---

## Changing the design

Everything is generated by Python from the files in `source/`.

```
pip install cadquery trimesh manifold3d shapely rtree mapbox_earcut vtk pillow
cd source
python make_all.py
```

`make_all.py` runs, in order:

| Script | What it does |
|---|---|
| `momo_enclosures.py` | Builds the 11 parts of the Cube, the Pod and the shared tray / retainer → `out/*.step` and `out/*.stl` |
| `momo_hero.py` | Builds the 4 Hero Momo parts (shell, back plate, mask, cape) |
| `checks.py` | Checks bed size, overhangs and collisions (see below) |
| `plates.py` | Places the parts on A1 mini plates → `plates/*.3mf` and `stl_print_ready/*.stl` |
| `render.py` | Makes the pictures → `renders/*.png` |

It takes about a minute on a normal PC. Useful numbers at the top of `momo_enclosures.py`:

| Parameter | Default | Meaning |
|---|---|---|
| `CL` | 0.25 mm | Clearance per side for sliding fits. Use 0.3 if the tray is tight on your printer. |
| `T` / `FT` | 2.4 / 2.6 mm | Wall / face thickness |
| `A_W, A_H, A_R` | 68, 62, 12 mm | Cube width, height, corner radius |
| `B_SECTIONS`, `B_BACK` | | Pod shape: width, height and corner radii at each depth |
| `A_VISOR`, `B_VISOR` | 54×34 r10, 50×32 r13 | Visor size and corner radius |
| `A_EAR` | Z 40, Y 30 | Ear position |
| `dock_lift(tilt_deg=6)` | 6° | How far the dock leans Momo back |

---

## How the designs were checked

`checks.py` runs these checks on the exported files (results in `source/checks_report.txt`):

| Check | Result |
|---|---|
| Every part is a closed solid (watertight) | 15 / 15 |
| Every part fits the A1 mini (180 × 180 × 180 mm) | 15 / 15; the largest footprint is the dock (78 × 68 mm), the tallest parts are the shells (56 mm) |
| Plates: all parts inside the plate, at least 3 mm apart | 9 / 9 plates |
| Overhangs steeper than 45° in the print position | Only small ledges, 2.4 mm or less (tops of small holes, screw counterbores, the 45° charger stop, the edges of the cape's star window) |
| Bridges | Longest 8.4 mm (rubber-foot recesses under the dock); everything else 5.2 mm or less |
| Collisions with everything assembled (shells, covers, tray, visors, ears/antennas, dock, cape + stand-ins for OLED, buzzer, TTP223, DS3231 with coin cell, MPU6050, ESP32-C3 with USB-C, and for Hero Momo the LiPo, the TP4056 board with its USB-C, the slide switch and the back button) | 0 mm³ (rounding below 0.05 mm³). The retainer presses 0.1 mm into the OLED board on purpose — that is what holds it. |

What was **not** done here: printing them. These checks catch design mistakes (parts that do not fit, overhangs that would fail), but real printers and real modules vary by a few tenths of a millimetre — that is what the quick fit check above is for.

---

## Files

```
5_Enclosures/
├── README.md                    ← this file
├── A_Momo_Cube/
│   ├── A_Cube_plate1_white.3mf  ← open in Bambu Studio and print
│   ├── A_Cube_plate2_coral.3mf
│   ├── A_Cube_plate3_black.3mf
│   ├── STL/                     ← single parts, turned for printing
│   └── STEP/                    ← single parts for editing in any CAD program
├── B_Momo_Pod/                  ← the same for the Pod
├── C_Hero_Momo/                 ← the same for Hero Momo (new in 2.2)
├── pictures/                    ← all renders and plate pictures
└── source/                      ← the Python design (parametric) + checks report
```
