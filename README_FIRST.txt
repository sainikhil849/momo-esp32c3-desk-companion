MOMO 2.2 - START HERE
=====================

What is in this folder
  1_VSCode_Simulator   Try Momo 2 on your PC in VS Code (Wokwi). The firmware
                       is already built - just open the folder and press Start.
                       momo_wokwi          = full version (with Wi-Fi + app)
                       momo_wokwi_no_wifi  = backup version without Wi-Fi
  2_Robot_ArduinoIDE   The code for the real robot (open momo/momo.ino in
                       Arduino IDE) + setup guides for the phone apps.
  3_Phone_App          Source of the Momo app. Momo already carries the app
                       inside it - you only need this folder to change the app.
  4_Pictures           Hero screens, animated weather, all 46 expressions,
                       the phone app, the bodies and the wiring diagram.
  5_Enclosures         Three 3D-printable bodies for the Bambu Lab A1 mini:
                       Hero Momo (new - with the battery), Momo Cube and
                       Momo Pod. Open the .3mf plates in Bambu Studio.
                       Guide: 5_Enclosures/README.md
  README.md            Full description (best viewed on GitHub).
  HARDWARE_GUIDE.md    What to buy, prices in Hyderabad, wiring diagram,
                       soldering step by step, battery safety.
  TEST_REPORT.txt      What was tested and the results.

Do it in this order
  A. Try it on your PC:   1_VSCode_Simulator/momo_wokwi/HOW_TO_RUN.txt
  B. Build the hardware:  HARDWARE_GUIDE.md (test on a breadboard first)
     Put the code on it:  2_Robot_ArduinoIDE/HOW_TO_UPLOAD.txt
  C. Connect your phone:  2_Robot_ArduinoIDE/PHONE_SETUP.txt
  D. Print the body:      5_Enclosures/README.md (print the tray + mask
                          first as a fit check, then the rest)

What is new in Momo 2.2 (everything else is unchanged)
  - Battery: 500 mAh LiPo + a USB-C charger board (TP4056). Estimated 4-5 hours
    without a cable; validate on the finished hardware. Battery icon, charging
    screen, "Full!", a low-battery warning at 15 %, and Momo sleeps by itself
    before the battery is empty.
    Two USB-C sockets on the back: CHARGE (charging) and CODE (uploading).
    IMPORTANT: verify the TP4056 PROG resistor and the battery maker's maximum
    charge current before charging. A 3 kOhm PROG resistor gives about 400 mA
    on a standard TP4056; see HARDWARE_GUIDE.md, section 3, check 6.
  - Power switch: a slide switch turns Momo fully off (it still charges).
  - Back button on Momo's back: does everything the head does. Hold it
    3 seconds = sleep; press it = wake up. Momo also wakes up by itself just
    before an alarm or reminder.
  - Back is a gesture: DOUBLE-TAP (head or button) = back, on every screen.
    In Jump and Flappy: hold = back. The app has a Back button too.
  - Head rub: keep your hand on Momo's head - it closes its eyes and purrs.
    (Let go after about 1 second instead = menu, as before.)
  - Mini superhero Momo: a little mask on every face, a hero with a cape for
    water and stretch breaks, hero sounds. App > More > Momo's look switches
    back to Classic in one tap.
  - Animated weather (sun, clouds, rain, storm, snow, fog, wind). App > More >
    Weather: refresh every 10 min ... 3 hours, from Auto / Phone / Internet,
    Update now, last update.
  - Hero Momo body: battery bay, mask visor, star touch spot, red cape with
    the back button in its star.
  - Uploading 2.2 keeps your settings, alarms, reminders, phones, pets,
    scores and Wi-Fi.

What was new in Momo 2.1
  - Your own reminders: type anything in the app (for example "Book ticket"
    at 4:30 PM), once on a date, every day, or on chosen days (up to 10).
    Momo shows your words big and rings until you tap it; hold = again in
    5 minutes. They ring even in quiet hours and at night.
  - Water reminder is an animation now: a bottle pours into the glass, then
    Momo drinks it through a straw. Stretch reminder: a little Momo buddy
    does the routine (arms up, bend left/right, arm circles, squat).
    No text on these screens - small cups / dots show today's count.
  - Every one of the 46 emotions has its own sound. Momo plays it when it
    reacts (pets, games, the gallery, the app's face buttons); quiet idle
    faces stay silent.
  - 4 new games you play with Momo: Simon (repeat tap / tilt moves),
    Copy me (tap, tilt or shake when Momo says), Rhythm (tap the beat
    back), Rock paper scissors (tilt left / tap / tilt right).
  - Your settings, pets and scores from Momo 2.0 are kept when you upload 2.1.

What was new in Momo 2
  - All 46 expressions: Menu > 46 expressions (tap = next), or tap any of
    them in the Momo app. Momo also uses them by itself.
  - Momo follows your day: wakes up (Good morning!), gets sleepy in the
    evening, sleeps at night (screen dimmed, no sounds). Surprise faces and
    games happen only while Momo is awake, in the hours you choose.
  - Alarms: 8 alarms in the Momo app (days, label, sound, snooze) + the
    alarms you set in the Chronos app. Tap = snooze, hold = stop.
  - Water and stretch reminders at exact times (up to 8 each) and/or every
    X minutes between two times, with a filling glass and a stretching buddy.
  - Sounds: 4 themes (cute, robot, soft, retro), volume, sound on/off per
    type, quiet hours, affection sounds (chirp, purr, giggle, laugh, love,
    kiss), and your own tune for alarms.
  - Games you play: Jump, Flappy, Catch (tilt Momo), Reaction. Best scores
    are saved. Games never pop up by themselves - only Momo plays Pong alone
    when it is bored, in play hours, and you can switch that off.
  - Music from your phone: song title + animation, tap = play/pause,
    tilt left/right = previous/next.
  - Weather from the Chronos app or the internet, with animated icons.
  - Wi-Fi + the Momo app on your phone (no install, opens in Chrome),
    secured: only a phone that saw the 6-digit code on Momo's screen can
    control it. Backup and restore of all settings.
