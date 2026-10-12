// =====================================================================
//  hero.h  -  Momo 2.2 hero look: Momo as its own little superhero
//  (a cape, a mask and a star emblem - an original character).
//  Water reminder: Hero Momo flies in, lands, drinks and powers up.
//  Stretch reminder: hero training - power pose, reach, bends, punches,
//  squat and a super jump.
//  Choose "Classic" in the app to get the 2.1 animations back.
// =====================================================================

// Cape behind the buddy: shoulders at (sx, sy), flutters with time,
// streams sideways when flying (stream: -1 left .. 1 right)
void drawCape(float sx, float sy, float stream, unsigned long t) {
  float w1 = sinf(t / 140.0f) * 2.0f, w2 = sinf(t / 140.0f + 1.8f) * 2.0f;
  int x0 = (int)lroundf(sx - 4), x1 = (int)lroundf(sx + 4), y0 = (int)lroundf(sy + 1);
  float dx = stream * 12.0f, dy = 15.0f - fabsf(stream) * 7.0f;
  int bl = (int)lroundf(sx - 9 + dx + w1), br = (int)lroundf(sx + 9 + dx + w2);
  int by = (int)lroundf(sy + dy), bm = (int)lroundf(sx + dx + (w1 + w2) / 2);
  // outline with a wavy hem; dotted inside so the body stays easy to see
  display.drawLine(x0, y0, bl, by, INK);
  display.drawLine(x1, y0, br, by, INK);
  display.drawLine(bl, by, bm, by - 2 + (int)w1, INK);
  display.drawLine(bm, by - 2 + (int)w1, br, by, INK);
  for (int y = y0 + 2; y < by - 1; y += 2) {
    float k = (float)(y - y0) / max(1, by - y0);
    int a = (int)lroundf(x0 + (bl - x0) * k), b = (int)lroundf(x1 + (br - x1) * k);
    for (int x = a + 2; x < b - 1; x += 3) display.drawPixel(x + (y / 2) % 2, y, INK);
  }
}

// Mask on the buddy's head (black band with two bright eyes, and a tie)
void drawHeadMask(float hcx, float hcy, unsigned long t) {
  int x = (int)lroundf(hcx), y = (int)lroundf(hcy);
  display.fillRect(x - 6, y - 3, 13, 4, ERASE);
  bool blink = (t % 2600) < 140;
  display.fillRect(x - 3, y - 2 + (blink ? 1 : 0), 2, blink ? 1 : 2, INK);
  display.fillRect(x + 2, y - 2 + (blink ? 1 : 0), 2, blink ? 1 : 2, INK);
  int w = (int)(sinf(t / 150.0f) * 1.5f);
  display.drawLine(x + 7, y - 2, x + 10, y - 3 + w, INK);
  display.drawLine(x + 7, y - 1, x + 10, y + 1 + w, INK);
}

// The whole hero: cape, buddy body (screens.h), masked head
void drawHeroBuddy(int cx, int footY, float lean, float lUp, float lLow, float rUp, float rLow, float knee, int lift,
                   float stream, unsigned long now) {
  const float D2R = 0.0174533f;
  float thigh = (8 + knee / 2) * D2R, shin = (8 - knee / 3) * D2R;
  float legH = cosf(thigh) * 11 + cosf(shin) * 11;
  float hx = cx, hy = footY - legH - lift;
  float tl = (180 - lean) * D2R;
  float sx = hx + sinf(tl) * 9, sy = hy + cosf(tl) * 9;
  drawCape(sx, sy, stream, now);
  drawBuddy(cx, footY, lean, lUp, lLow, rUp, rLow, knee, lift, now);
  float hcx = hx + sinf(tl) * 21, hcy = hy + cosf(tl) * 21;
  drawHeadMask(hcx, hcy, now);
  display.fillCircle((int)lroundf(sx), (int)lroundf(sy) + 2, 1, ERASE);   // star emblem spot on the chest
}

// A small hero star with a twinkle
void heroStar(int cx, int cy, int r, unsigned long t) {
  starShape(cx, cy, r + (int)((t / 200) % 2), INK);
}

// Hydration meter: a power cell with a water drop, fill 0..1
void drawPowerMeter(int x, int y, float level, bool blinkEmpty, unsigned long t) {
  display.drawRoundRect(x, y, 12, 30, 3, INK);
  display.fillRect(x + 4, y - 2, 4, 2, INK);
  int h = (int)lroundf(26 * constrain(level, 0.0f, 1.0f));
  if (h > 0) display.fillRect(x + 2, y + 28 - h, 8, h, INK);
  if (level > 0.15f)                                     // bubbles going up
    for (int k = 0; k < 2; k++) {
      int by = y + 27 - (int)((t / 60 + k * 11) % (unsigned long)max(2, h));
      display.drawPixel(x + 4 + k * 3, by, ERASE);
    }
  if (blinkEmpty && level < 0.05f && (t / 300) % 2) display.drawRect(x + 2, y + 24, 8, 4, INK);
  dropShape(x + 6, y + 36, 2, INK);
}

const float HIPS_UP = 50, HIPS_LOW = -60;       // hands on hips (the power pose)

void drawHeroWater(unsigned long now) {
  if (!remindDone) {
    unsigned long t = now - screenStart;
    if (t < 900) {                                       // flies in, cape streaming
      float k = easeInOut(t / 900.0f);
      int x = (int)lroundf(-12 + 52 * k), foot = (int)lroundf(30 + 31 * k);
      drawHeroBuddy(x, foot, 18 * (1 - k), 165, 175, 165, 175, 0, 0, -1.0f + k * 0.6f, now);
      for (int s = 0; s < 3; s++) display.drawFastHLine(x - 26 + s * 3, foot - 30 + s * 6, 10, INK);   // speed lines
    } else if (t < 1400) {                               // lands with a little squat
      float k = (t - 900) / 500.0f;
      float knee = sinf(k * 3.1416f) * 50;
      drawHeroBuddy(40, 61, 0, HIPS_UP + 60 * (1 - k), HIPS_LOW, HIPS_UP + 60 * (1 - k), HIPS_LOW, knee, 0, 0, now);
      for (int s = -1; s <= 1; s += 2) display.drawLine(40 + s * 8, 62, 40 + s * 14, 58, INK);       // dust
    } else {                                             // power pose, waiting for water
      drawHeroBuddy(40, 61, 0, HIPS_UP, HIPS_LOW, HIPS_UP, HIPS_LOW, 0, 0, 0, now);
      int bob = (int)((now / 250) % 2);
      display.drawCircle(64, 12 + bob, 8, INK);           // thought bubble with a water drop
      display.fillCircle(56, 22 + bob, 1, INK);
      dropShape(64, 9 + bob, 3, INK);
    }
    drawPowerMeter(100, 8, 0, t >= 1400, now);
    drawGlassRow(st.drinks, cfg.water.goal, 54, false, now);
    return;
  }
  unsigned long since = now - remindDoneAt;
  float level = since < 1800 ? since / 1800.0f : 1.0f;
  bool goal = st.drinks >= cfg.water.goal;
  if (since < 1800) {                                    // drinks from a bottle
    float tip = min(1.0f, since / 400.0f);
    float rUp = 70 + 45 * tip, rLow = 120 + 45 * tip;   // the right arm brings the bottle up
    drawHeroBuddy(40, 61, 0, HIPS_UP, HIPS_LOW, rUp, rLow, 0, 0, 0, now);
    const float D2R = 0.0174533f;                        // where the right hand is (same maths as drawBuddy)
    float hy = 61 - cosf(8 * D2R) * 22, sy = hy - 9;
    float ex = 40 + sinf(rUp * D2R) * 10, ey = sy + cosf(rUp * D2R) * 10;
    float hx = ex + sinf(rLow * D2R) * 10, hyy = ey + cosf(rLow * D2R) * 10;
    float mx = 41, my = hy - 21 + 4;                     // Momo's mouth, under the mask
    float ang = atan2f(mx - hx, -(my - hyy));            // bottle points from the hand to the mouth
    fillRotRect(hx, hyy, ang, 0, 8, 2.5f, INK);          // body
    fillRotRect(hx, hyy, ang, 3, 5, 1.2f, ERASE);        // label
    fillRotRect(hx, hyy, ang, 8, 11, 1.2f, INK);         // neck
    if ((since / 120) % 2) display.drawPixel(37, (int)my + 2, INK);   // gulp
  } else {                                               // powered up: arms up, star burst
    unsigned long k = since - 1800;
    int lift = k < 600 ? (int)(sinf(k / 600.0f * 3.1416f) * 6) : 0;
    drawHeroBuddy(40, 61, 0, 160, 175, 160, 175, 0, lift, 0, now);
    for (int i = 0; i < 8; i++) {                        // burst
      float ang = i * 0.785f;
      int r0 = 20 + (int)((k / 40) % 8);
      display.drawLine(40 + (int)(cosf(ang) * r0), 28 + (int)(sinf(ang) * r0 * 0.7f),
                       40 + (int)(cosf(ang) * (r0 + 4)), 28 + (int)(sinf(ang) * (r0 + 4) * 0.7f), INK);
    }
    heroStar(70, 12, 5, now);
    if (goal) drawOverlay(O_CONFETTI, now, 0);
  }
  drawPowerMeter(100, 8, level, false, now);
  drawGlassRow(st.drinks, cfg.water.goal, 54, since >= 1800, now);
}

// Hero training routine (front view), with the cape
struct HPose { int8_t lean; int16_t lUp, lLow, rUp, rLow; uint8_t knee, lift; };
const HPose H_STAND = {0, 15, 10, 15, 10, 0, 0}, H_HIPS = {0, 50, -60, 50, -60, 0, 0}, H_UP = {0, 165, 175, 165, 175, 0, 0},
            H_BENDL = {-24, 125, 150, 125, 150, 0, 0}, H_BENDR = {24, 125, 150, 125, 150, 0, 0},
            H_PUNCHL = {0, 90, 90, 50, -60, 0, 0}, H_PUNCHR = {0, 50, -60, 90, 90, 0, 0},
            H_SQUAT = {0, 110, 120, 110, 120, 60, 0}, H_JUMP = {0, 165, 175, 165, 175, 0, 9};
struct HStep { const HPose *pose; uint16_t ms; };
const HStep HERO_ROUTINE[] = {
  {&H_STAND, 800}, {&H_HIPS, 1500}, {&H_UP, 1600}, {&H_BENDL, 1600}, {&H_UP, 800}, {&H_BENDR, 1600},
  {&H_PUNCHL, 900}, {&H_PUNCHR, 900}, {&H_PUNCHL, 900}, {&H_PUNCHR, 900}, {&H_SQUAT, 1500}, {&H_JUMP, 900}, {&H_HIPS, 1200}
};
const uint8_t HERO_ROUTINE_N = sizeof(HERO_ROUTINE) / sizeof(HERO_ROUTINE[0]);

void heroPoseAt(unsigned long t, float &lean, float &lUp, float &lLow, float &rUp, float &rLow, float &knee, float &lift) {
  unsigned long total = 0;
  for (uint8_t i = 0; i < HERO_ROUTINE_N; i++) total += HERO_ROUTINE[i].ms;
  t %= total;
  uint8_t i = 0;
  while (t >= HERO_ROUTINE[i].ms) { t -= HERO_ROUTINE[i].ms; i++; }
  const HPose &a = *HERO_ROUTINE[(i + HERO_ROUTINE_N - 1) % HERO_ROUTINE_N].pose, &b = *HERO_ROUTINE[i].pose;
  float k = easeInOut(t / 400.0f);
  lean = lerpf(a.lean, b.lean, k);
  lUp = lerpf(a.lUp, b.lUp, k); lLow = lerpf(a.lLow, b.lLow, k);
  rUp = lerpf(a.rUp, b.rUp, k); rLow = lerpf(a.rLow, b.rLow, k);
  knee = lerpf(a.knee, b.knee, k);
  lift = lerpf(a.lift, b.lift, k);
  if (b.lift > 0) lift = b.lift * sinf(min(1.0f, t / (float)HERO_ROUTINE[i].ms) * 3.1416f);   // up and down
}

void drawHeroStretch(unsigned long now) {
  uint8_t goal = (uint8_t)min(8, (int)cfg.stretch.goal);
  if (remindDone) {                                      // victory: jumping with arms up, stars
    unsigned long since = now - remindDoneAt;
    int lift = (int)(fabsf(sinf(since / 160.0f)) * 7);
    float w = sinf(since / 120.0f) * 10;
    drawHeroBuddy(40, 61, 0, 160 + w, 175, 160 - w, 175, 0, lift, 0, now);
    heroStar(70, 10, 5, now);
    heroStar(16, 16, 3, now + 100);
    drawMiniEyes(104, 14, F_CELEBRATE, now);
    drawOverlay(O_CONFETTI, now, 0);
  } else {
    float lean, lUp, lLow, rUp, rLow, knee, lift;
    unsigned long t = now - screenStart;
    heroPoseAt(t, lean, lUp, lLow, rUp, rLow, knee, lift);
    drawHeroBuddy(40, 61, lean, lUp, lLow, rUp, rLow, knee, (int)lift, 0, now);
    if (lUp == 90 || rUp == 90 || fabsf(lUp - 90) < 3 || fabsf(rUp - 90) < 3) {   // punch lines
      int side = fabsf(lUp - 90) < 3 ? -1 : 1;
      for (int k = 0; k < 3; k++) display.drawFastHLine(40 + side * (24 + k * 2), 33 + k * 3 - 3, 4, INK);
    }
    drawMiniEyes(104, 14, F_DETERMINED, now);
    int filled = (int)min(12UL, t / (20000UL / 12));      // a ring of dots fills up over 20 seconds
    for (int k = 0; k < 12; k++) {
      float a = k * 0.5236f - 1.5708f;
      int x = 104 + (int)lroundf(cosf(a) * 11), y = 48 + (int)lroundf(sinf(a) * 11);
      if (k < filled) display.fillCircle(x, y, 2, INK); else display.drawPixel(x, y, INK);
    }
  }
  for (uint8_t i = 0; i < goal; i++) {                   // stretches today: one dot each
    int x = 104 - ((int)goal - 1) * 3 + i * 6;
    if (i < st.stretches) display.fillCircle(x, 29, 2, INK); else display.drawCircle(x, 29, 2, INK);
  }
}

// Start-up: the hero star lands, then Momo's name
void drawHeroBoot(unsigned long now) {
  unsigned long t = now - screenStart;
  float k = min(1.0f, t / 600.0f);
  int r = (int)lroundf(4 + 14 * easeInOut(k));
  starShape(64, 20, r, INK);
  display.fillCircle(64, 21, max(1, r / 4), ERASE);
  if (t > 600 && t < 1100)
    for (int i = 0; i < 8; i++) {
      float a = i * 0.785f;
      int r0 = 20 + (int)((t - 600) / 25);
      display.drawLine(64 + (int)(cosf(a) * r0), 20 + (int)(sinf(a) * r0 * 0.8f),
                       64 + (int)(cosf(a) * (r0 + 5)), 20 + (int)(sinf(a) * (r0 + 5) * 0.8f), INK);
    }
  if (t > 450) {
    centerText(cfg.name, 42, 2);
    display.setTextSize(1);
    display.setCursor(104, 56);
    display.print("v" MOMO_VERSION);
  }
}
