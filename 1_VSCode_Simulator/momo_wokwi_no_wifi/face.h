// =====================================================================
//  face.h  -  Momo's 46 expressions
//  Each expression = two eye shapes + optional eye art, overlay, mouth
//  and motion. Momo glides smoothly from one expression to the next.
// =====================================================================

struct EyeP {
  int8_t x, y;      // offset from the normal eye position
  uint8_t w, h, r;  // size and corner roundness
  uint8_t lid;      // top eyelid (pixels covered)
  int8_t slant;     // + = angry (inner corner lower), - = sad (outer corner lower)
  uint8_t cut;      // happy "^" cut from below
  uint8_t lidB;     // bottom eyelid
};

enum EyeArt : uint8_t { A_NORMAL, A_HEARTS, A_STARS, A_X, A_SPIRAL, A_CLOSED, A_CHEVRON, A_GLASSES };
enum Overlay : uint8_t {
  O_NONE, O_SPARKLE, O_BLUSH, O_TEARS, O_SWEAT, O_QUESTION, O_EXCLAIM, O_DOTS, O_VEIN, O_STEAM,
  O_ZZZ, O_NOTES, O_HEART, O_DROP, O_GLITCH, O_CONFETTI
};
enum Mouth : uint8_t { M_NONE, M_SMILE, M_GRIN, M_FROWN, M_O, M_BIGO, M_SMIRK, M_TONGUE, M_WAVY, M_FLAT, M_KISS };
enum Motion : uint8_t { MO_NONE, MO_BOUNCE, MO_SHAKE, MO_TREMBLE, MO_BOB, MO_SWAY, MO_LOOK, MO_FLOAT, MO_GLITCH, MO_PULSE, MO_BREATH };

struct Expr { EyeP L, R; uint8_t art, ov1, ov2, mouth, motion; };

enum Face : uint8_t {
  F_NEUTRAL, F_HAPPY, F_JOY, F_LAUGH, F_LOVE, F_BLUSH, F_WINK, F_SLEEPY, F_ASLEEP, F_BORED,
  F_SAD, F_CRY, F_ANGRY, F_FURIOUS, F_SURPRISED, F_SHOCKED, F_SCARED, F_CONFUSED, F_CURIOUS, F_THINKING,
  F_SUSPICIOUS, F_ANNOYED, F_DIZZY, F_EXCITED, F_PROUD, F_RELAXED, F_FOCUSED, F_DETERMINED, F_COOL, F_MISCHIEVOUS,
  F_GRATEFUL, F_KISS, F_YAWN, F_SILLY, F_WORRIED, F_NERVOUS, F_EMBARRASSED, F_DISAPPOINTED, F_HOPEFUL, F_PLAYFUL,
  F_SNEAKY, F_GLITCH, F_MUSIC, F_THIRSTY, F_EFFORT, F_CELEBRATE, F_COUNT
};

const char *FACE_NAMES[F_COUNT] = {
  "Neutral", "Happy", "Joy", "Laugh", "Love", "Blush", "Wink", "Sleepy", "Asleep", "Bored",
  "Sad", "Cry", "Angry", "Furious", "Surprised", "Shocked", "Scared", "Confused", "Curious", "Thinking",
  "Suspicious", "Annoyed", "Dizzy", "Excited", "Proud", "Relaxed", "Focused", "Determined", "Cool", "Mischievous",
  "Grateful", "Kiss", "Yawn", "Silly", "Worried", "Nervous", "Embarrassed", "Disappointed", "Hopeful", "Playful",
  "Sneaky", "Glitch", "Music", "Thirsty", "Effort", "Celebrate"
};

#define EYE(x, y, w, h, r, lid, sl, cut, lb) {x, y, w, h, r, lid, sl, cut, lb}
#define E_N     EYE(0, 0, 30, 36, 8, 0, 0, 0, 0)
#define E_HAPPY EYE(0, -2, 30, 34, 8, 0, 0, 14, 0)
#define E_WINKC EYE(0, -2, 30, 34, 8, 0, 0, 27, 0)

const Expr EXPRS[F_COUNT] = {
  /* Neutral      */ {E_N, E_N, A_NORMAL, O_NONE, O_NONE, M_NONE, MO_BREATH},
  /* Happy        */ {E_HAPPY, E_HAPPY, A_NORMAL, O_NONE, O_NONE, M_NONE, MO_NONE},
  /* Joy          */ {EYE(0, -4, 32, 34, 9, 0, 0, 16, 0), EYE(0, -4, 32, 34, 9, 0, 0, 16, 0), A_NORMAL, O_SPARKLE, O_NONE, M_GRIN, MO_BOUNCE},
  /* Laugh        */ {EYE(0, -4, 32, 24, 8, 0, 0, 12, 0), EYE(0, -4, 32, 24, 8, 0, 0, 12, 0), A_NORMAL, O_NONE, O_NONE, M_BIGO, MO_BOUNCE},
  /* Love         */ {E_N, E_N, A_HEARTS, O_NONE, O_NONE, M_SMILE, MO_PULSE},
  /* Blush        */ {EYE(0, 4, 28, 30, 8, 4, 0, 0, 0), EYE(0, 4, 28, 30, 8, 4, 0, 0, 0), A_NORMAL, O_BLUSH, O_NONE, M_NONE, MO_NONE},
  /* Wink         */ {E_WINKC, E_N, A_NORMAL, O_NONE, O_NONE, M_SMIRK, MO_NONE},
  /* Sleepy       */ {EYE(0, 2, 30, 36, 8, 20, -3, 0, 0), EYE(0, 2, 30, 36, 8, 20, -3, 0, 0), A_NORMAL, O_NONE, O_NONE, M_NONE, MO_FLOAT},
  /* Asleep       */ {E_N, E_N, A_CLOSED, O_ZZZ, O_NONE, M_NONE, MO_BREATH},
  /* Bored        */ {EYE(6, 2, 30, 36, 8, 18, 0, 0, 4), EYE(6, 2, 30, 36, 8, 18, 0, 0, 4), A_NORMAL, O_NONE, O_NONE, M_FLAT, MO_NONE},
  /* Sad          */ {EYE(0, 4, 30, 32, 8, 10, -6, 0, 0), EYE(0, 4, 30, 32, 8, 10, -6, 0, 0), A_NORMAL, O_NONE, O_NONE, M_FROWN, MO_NONE},
  /* Cry          */ {EYE(0, 2, 30, 32, 8, 10, -6, 0, 0), EYE(0, 2, 30, 32, 8, 10, -6, 0, 0), A_NORMAL, O_TEARS, O_NONE, M_FROWN, MO_NONE},
  /* Angry        */ {EYE(0, 2, 30, 34, 6, 8, 8, 0, 0), EYE(0, 2, 30, 34, 6, 8, 8, 0, 0), A_NORMAL, O_NONE, O_NONE, M_FLAT, MO_NONE},
  /* Furious      */ {EYE(0, 2, 32, 34, 4, 10, 11, 0, 0), EYE(0, 2, 32, 34, 4, 10, 11, 0, 0), A_NORMAL, O_VEIN, O_STEAM, M_FROWN, MO_SHAKE},
  /* Surprised    */ {EYE(0, -3, 34, 42, 14, 0, 0, 0, 0), EYE(0, -3, 34, 42, 14, 0, 0, 0, 0), A_NORMAL, O_NONE, O_NONE, M_O, MO_NONE},
  /* Shocked      */ {EYE(0, -4, 24, 44, 10, 0, 0, 0, 0), EYE(0, -4, 24, 44, 10, 0, 0, 0, 0), A_NORMAL, O_EXCLAIM, O_NONE, M_BIGO, MO_TREMBLE},
  /* Scared       */ {EYE(0, -2, 22, 28, 8, 0, -3, 0, 0), EYE(0, -2, 22, 28, 8, 0, -3, 0, 0), A_NORMAL, O_SWEAT, O_NONE, M_WAVY, MO_TREMBLE},
  /* Confused     */ {EYE(0, 0, 30, 36, 8, 4, -4, 0, 0), EYE(0, 3, 24, 26, 8, 0, 0, 0, 0), A_NORMAL, O_QUESTION, O_NONE, M_WAVY, MO_NONE},
  /* Curious      */ {EYE(6, 1, 28, 32, 8, 6, 0, 0, 0), EYE(6, -3, 34, 42, 12, 0, 0, 0, 0), A_NORMAL, O_NONE, O_NONE, M_NONE, MO_NONE},
  /* Thinking     */ {EYE(8, -6, 28, 32, 8, 6, 0, 0, 0), EYE(8, -6, 28, 32, 8, 6, 0, 0, 0), A_NORMAL, O_DOTS, O_NONE, M_FLAT, MO_NONE},
  /* Suspicious   */ {EYE(-8, 2, 30, 36, 6, 18, 3, 0, 8), EYE(-8, 2, 30, 36, 6, 18, 3, 0, 8), A_NORMAL, O_NONE, O_NONE, M_FLAT, MO_NONE},
  /* Annoyed      */ {EYE(-6, -4, 30, 36, 6, 17, 0, 0, 0), EYE(-6, -4, 30, 36, 6, 17, 0, 0, 0), A_NORMAL, O_NONE, O_NONE, M_FLAT, MO_NONE},
  /* Dizzy        */ {E_N, E_N, A_SPIRAL, O_NONE, O_NONE, M_WAVY, MO_SWAY},
  /* Excited      */ {E_N, E_N, A_STARS, O_NONE, O_NONE, M_GRIN, MO_BOUNCE},
  /* Proud        */ {EYE(0, -6, 30, 32, 8, 0, 0, 13, 0), EYE(0, -6, 30, 32, 8, 0, 0, 13, 0), A_NORMAL, O_SPARKLE, O_NONE, M_SMIRK, MO_NONE},
  /* Relaxed      */ {EYE(0, 2, 30, 34, 10, 14, 0, 8, 0), EYE(0, 2, 30, 34, 10, 14, 0, 8, 0), A_NORMAL, O_NONE, O_NONE, M_SMILE, MO_FLOAT},
  /* Focused      */ {EYE(0, 0, 32, 16, 6, 0, 2, 0, 0), EYE(0, 0, 32, 16, 6, 0, 2, 0, 0), A_NORMAL, O_NONE, O_NONE, M_FLAT, MO_NONE},
  /* Determined   */ {EYE(0, 0, 30, 34, 6, 8, 5, 0, 0), EYE(0, 0, 30, 34, 6, 8, 5, 0, 0), A_NORMAL, O_SPARKLE, O_NONE, M_FLAT, MO_NONE},
  /* Cool         */ {E_N, E_N, A_GLASSES, O_NONE, O_NONE, M_SMIRK, MO_NONE},
  /* Mischievous  */ {EYE(0, 0, 30, 36, 8, 16, 5, 0, 0), E_HAPPY, A_NORMAL, O_NONE, O_NONE, M_SMIRK, MO_NONE},
  /* Grateful     */ {E_HAPPY, E_HAPPY, A_NORMAL, O_HEART, O_NONE, M_SMILE, MO_FLOAT},
  /* Kiss         */ {E_WINKC, E_N, A_NORMAL, O_HEART, O_NONE, M_KISS, MO_NONE},
  /* Yawn         */ {EYE(0, 2, 32, 10, 4, 0, 0, 0, 0), EYE(0, 2, 32, 10, 4, 0, 0, 0, 0), A_NORMAL, O_NONE, O_NONE, M_BIGO, MO_NONE},
  /* Silly        */ {EYE(8, 0, 26, 30, 8, 0, 0, 0, 0), EYE(-8, 0, 26, 30, 8, 0, 0, 0, 0), A_NORMAL, O_NONE, O_NONE, M_TONGUE, MO_SWAY},
  /* Worried      */ {EYE(0, 0, 26, 30, 8, 6, -5, 0, 0), EYE(0, 0, 26, 30, 8, 6, -5, 0, 0), A_NORMAL, O_NONE, O_NONE, M_WAVY, MO_NONE},
  /* Nervous      */ {EYE(2, 0, 26, 30, 8, 6, -5, 0, 0), EYE(2, 0, 26, 30, 8, 6, -5, 0, 0), A_NORMAL, O_SWEAT, O_NONE, M_WAVY, MO_TREMBLE},
  /* Embarrassed  */ {EYE(-8, 6, 28, 30, 8, 8, -2, 0, 0), EYE(-8, 6, 28, 30, 8, 8, -2, 0, 0), A_NORMAL, O_BLUSH, O_SWEAT, M_NONE, MO_NONE},
  /* Disappointed */ {EYE(0, 8, 30, 30, 8, 14, -4, 0, 0), EYE(0, 8, 30, 30, 8, 14, -4, 0, 0), A_NORMAL, O_NONE, O_NONE, M_NONE, MO_NONE},
  /* Hopeful      */ {EYE(0, -6, 34, 40, 14, 0, -2, 0, 0), EYE(0, -6, 34, 40, 14, 0, -2, 0, 0), A_NORMAL, O_SPARKLE, O_NONE, M_SMILE, MO_FLOAT},
  /* Playful      */ {E_HAPPY, EYE(0, -2, 30, 34, 8, 0, 0, 10, 0), A_NORMAL, O_NONE, O_NONE, M_TONGUE, MO_SWAY},
  /* Sneaky       */ {EYE(0, 2, 30, 36, 6, 20, 2, 0, 6), EYE(0, 2, 30, 36, 6, 20, 2, 0, 6), A_NORMAL, O_NONE, O_NONE, M_SMIRK, MO_LOOK},
  /* Glitch       */ {E_N, E_N, A_NORMAL, O_GLITCH, O_NONE, M_NONE, MO_GLITCH},
  /* Music        */ {E_HAPPY, E_HAPPY, A_NORMAL, O_NOTES, O_NONE, M_SMILE, MO_BOB},
  /* Thirsty      */ {EYE(0, 3, 30, 32, 8, 12, -3, 0, 0), EYE(0, 3, 30, 32, 8, 12, -3, 0, 0), A_NORMAL, O_DROP, O_NONE, M_TONGUE, MO_NONE},
  /* Effort       */ {E_N, E_N, A_CHEVRON, O_SWEAT, O_NONE, M_FLAT, MO_TREMBLE},
  /* Celebrate    */ {EYE(0, -4, 32, 34, 9, 0, 0, 16, 0), EYE(0, -4, 32, 34, 9, 0, 0, 16, 0), A_NORMAL, O_CONFETTI, O_SPARKLE, M_GRIN, MO_BOUNCE},
};

// ---------- animation state ----------
struct EyeF { float x, y, w, h, r, lid, slant, cut, lidB; };
EyeF curEye[2];
uint8_t faceNow = F_NEUTRAL;
bool faceFirst = true;
float gazeX = 0, gazeY = 0, gazeTX = 0, gazeTY = 0;
unsigned long nextBlink = 0, blinkStart = 0, nextGaze = 0;
bool blinkTwice = false;
float faceDim = 1.0f;   // unused on monochrome screens, kept for future colour screens

void eyeToF(const EyeP &p, EyeF &f) {
  f.x = p.x; f.y = p.y; f.w = p.w; f.h = p.h; f.r = p.r;
  f.lid = p.lid; f.slant = p.slant; f.cut = p.cut; f.lidB = p.lidB;
}

void setFace(uint8_t id) {
  if (id >= F_COUNT) id = F_NEUTRAL;
  if (faceFirst || EXPRS[id].art != EXPRS[faceNow].art) {   // new eye art: no tweening
    eyeToF(EXPRS[id].L, curEye[0]);
    eyeToF(EXPRS[id].R, curEye[1]);
    faceFirst = false;
  }
  faceNow = id;
}

// Eyes start closed and open smoothly (used when Momo wakes up)
void openEyesFromClosed() {
  for (int i = 0; i < 2; i++) curEye[i].lid = curEye[i].h;
}

void lookAt(float dx, float dy) { gazeTX = dx; gazeTY = dy; }

// Pseudo-random from a number (stable per frame, no state)
uint32_t hash32(uint32_t x) { x ^= x >> 16; x *= 0x7feb352d; x ^= x >> 15; x *= 0x846ca68b; x ^= x >> 16; return x; }

// ---------- drawing helpers ----------
void heartShape(int cx, int cy, int s, uint16_t c) {
  display.fillCircle(cx - s / 2, cy, s / 2 + 1, c);
  display.fillCircle(cx + s / 2, cy, s / 2 + 1, c);
  display.fillTriangle(cx - s - 1, cy + 1, cx + s + 1, cy + 1, cx, cy + s + 4, c);
}

void sparkleShape(int cx, int cy, int r, uint16_t c) {
  display.fillTriangle(cx - 1, cy, cx + 1, cy, cx, cy - r, c);
  display.fillTriangle(cx - 1, cy, cx + 1, cy, cx, cy + r, c);
  display.fillTriangle(cx, cy - 1, cx, cy + 1, cx - r, cy, c);
  display.fillTriangle(cx, cy - 1, cx, cy + 1, cx + r, cy, c);
}

void starShape(int cx, int cy, int r, uint16_t c) {
  // 5-point star from 5 triangles + centre
  float a0 = -1.5708f;
  for (int i = 0; i < 5; i++) {
    float a = a0 + i * 1.2566f;
    float b1 = a - 0.6283f, b2 = a + 0.6283f;
    int ri = r * 4 / 10;
    display.fillTriangle(cx + (int)(cosf(a) * r), cy + (int)(sinf(a) * r),
                         cx + (int)(cosf(b1) * ri), cy + (int)(sinf(b1) * ri),
                         cx + (int)(cosf(b2) * ri), cy + (int)(sinf(b2) * ri), c);
  }
  display.fillCircle(cx, cy, r * 4 / 10 + 1, c);
}

void dropShape(int cx, int cy, int s, uint16_t c) {
  display.fillCircle(cx, cy + s, s, c);
  display.fillTriangle(cx - s, cy + s, cx + s, cy + s, cx, cy - s, c);
}

void noteShape(int x, int y, uint16_t c) {   // a small musical note
  display.fillCircle(x, y + 6, 2, c);
  display.drawFastVLine(x + 2, y, 7, c);
  display.drawLine(x + 2, y, x + 5, y + 2, c);
}

void thickLine(int x0, int y0, int x1, int y1, uint16_t c) {
  display.drawLine(x0, y0, x1, y1, c);
  display.drawLine(x0 + 1, y0, x1 + 1, y1, c);
  display.drawLine(x0, y0 + 1, x1, y1 + 1, c);
}

// ---------- the hero mask (Momo 2.2, hero look) ----------
// Momo's own little superhero mask around the eyes: an outline with a soft
// dotted fill and two small ties at the sides that flutter.
const int8_t MASK_PTS[][2] = {
  {-58, -10}, {-48, -20}, {-35, -25}, {-20, -25}, {-9, -20}, {-4, -14}, {4, -14}, {9, -20}, {20, -25}, {35, -25},
  {48, -20}, {58, -10}, {51, 3}, {42, 16}, {29, 22}, {16, 21}, {8, 14}, {3, 9}, {-3, 9}, {-8, 14}, {-16, 21},
  {-29, 22}, {-42, 16}, {-51, 3}
};
const uint8_t MASK_N = sizeof(MASK_PTS) / sizeof(MASK_PTS[0]);

bool heroLook() { return x22.look == 1; }

// scale in 1/100 (100 = full face), centre (cx, cy)
void drawHeroMask(float cx, float cy, int scale, bool fill, unsigned long t) {
  int px[MASK_N], py[MASK_N];
  for (uint8_t i = 0; i < MASK_N; i++) {
    px[i] = (int)lroundf(cx + MASK_PTS[i][0] * scale / 100.0f);
    py[i] = (int)lroundf(cy + MASK_PTS[i][1] * scale / 100.0f);
  }
  if (fill) {                                  // dotted fill, row by row
    int ymin = 64, ymax = -1;
    for (uint8_t i = 0; i < MASK_N; i++) { ymin = min(ymin, py[i]); ymax = max(ymax, py[i]); }
    for (int y = max(0, ymin); y <= min(63, ymax); y++) {
      int xs[8], nx = 0;
      for (uint8_t i = 0; i < MASK_N && nx < 8; i++) {
        int x0 = px[i], y0 = py[i], x1 = px[(i + 1) % MASK_N], y1 = py[(i + 1) % MASK_N];
        if ((y0 <= y && y1 > y) || (y1 <= y && y0 > y)) xs[nx++] = x0 + (y - y0) * (x1 - x0) / (y1 - y0);
      }
      for (int a = 1; a < nx; a++) for (int b = a; b > 0 && xs[b] < xs[b - 1]; b--) { int k = xs[b]; xs[b] = xs[b - 1]; xs[b - 1] = k; }
      if ((y % 3) != 0) continue;
      for (int k = 0; k + 1 < nx; k += 2)
        for (int x = max(0, xs[k] + 2); x <= min(127, xs[k + 1] - 2); x++)
          if (((x + y) % 3) == 0) display.drawPixel(x, y, INK);
    }
  }
  for (uint8_t i = 0; i < MASK_N; i++) {       // outline
    int j = (i + 1) % MASK_N;
    if (scale >= 70) thickLine(px[i], py[i], px[j], py[j], INK);
    else display.drawLine(px[i], py[i], px[j], py[j], INK);
  }
  if (scale >= 70) {                           // the mask's ties, fluttering
    float w = sinf(t / 160.0f) * 2.0f;
    for (int side = -1; side <= 1; side += 2) {
      int tx = px[side < 0 ? 0 : 11], ty = py[side < 0 ? 0 : 11];
      display.drawLine(tx, ty, tx + side * 4, ty + 5 + (int)w, INK);
      display.drawLine(tx, ty + 1, tx + side * 3, ty + 9 - (int)w, INK);
    }
  }
}

// One normal eye with eyelids and happy cut
void drawEyeShape(float cx, float cy, const EyeF &e, bool isLeft) {
  int w = (int)lroundf(e.w), h = (int)lroundf(e.h);
  if (w < 2 || h < 2) return;
  int x0 = (int)lroundf(cx - w / 2.0f), y0 = (int)lroundf(cy - h / 2.0f);
  int r = (int)lroundf(e.r);
  r = min(r, min(w, h) / 2 - 1);
  if (r > 0) display.fillRoundRect(x0, y0, w, h, r, INK);
  else display.fillRect(x0, y0, w, h, INK);
  int lid = (int)lroundf(e.lid), sl = (int)lroundf(e.slant);
  if (lid > 0 || sl != 0) {
    int inner = constrain(lid + sl, 0, h);
    int outer = constrain(lid - sl / 2, 0, h);
    int yl = y0 + (isLeft ? outer : inner);
    int yr = y0 + (isLeft ? inner : outer);
    display.fillTriangle(x0 - 1, y0 - 1, x0 + w, y0 - 1, x0 + w, yr, ERASE);
    display.fillTriangle(x0 - 1, y0 - 1, x0 + w, yr, x0 - 1, yl, ERASE);
  }
  int lb = (int)lroundf(e.lidB);
  if (lb > 0) display.fillRect(x0 - 1, y0 + h - lb, w + 2, lb + 1, ERASE);
  int cut = (int)lroundf(e.cut);
  if (cut > 0) {
    int rc = w * 7 / 10 + 2;
    display.fillCircle(x0 + w / 2, y0 + h - cut + rc, rc, ERASE);
  }
}

void drawArtEye(uint8_t art, float cx, float cy, bool isLeft, unsigned long t) {
  int x = (int)lroundf(cx), y = (int)lroundf(cy);
  switch (art) {
    case A_HEARTS: {
      int s = 11 + (int)((t / 180) % 2) * 2;
      heartShape(x, y - 6, s, INK);
      break;
    }
    case A_STARS: {
      int r = 15 + (int)((t / 150) % 2) * 2;
      starShape(x, y, r, INK);
      break;
    }
    case A_X:
      for (int d = -1; d <= 1; d++) {
        display.drawLine(x - 11 + d, y - 11, x + 11 + d, y + 11, INK);
        display.drawLine(x - 11 + d, y + 11, x + 11 + d, y - 11, INK);
      }
      break;
    case A_SPIRAL: {
      float rot = (t % 1200) / 1200.0f * 6.2832f * (isLeft ? 1 : -1);
      float px = x, py = y;
      for (float a = 0; a < 14.0f; a += 0.35f) {
        float rr = 1.0f + a * 1.05f;
        float nx = x + cosf(a + rot) * rr, ny = y + sinf(a + rot) * rr;
        if (a > 0) thickLine((int)px, (int)py, (int)nx, (int)ny, INK);
        px = nx; py = ny;
      }
      break;
    }
    case A_CLOSED: {   // sleeping eye: a soft downward curve
      int prevX = x - 14, prevY = y + 2;
      for (int dx = -14; dx <= 14; dx += 2) {
        int dy = 2 + (196 - dx * dx) / 40;
        thickLine(prevX, prevY, x + dx, y + dy, INK);
        prevX = x + dx; prevY = y + dy;
      }
      break;
    }
    case A_CHEVRON:   // > <  squeezing hard
      if (isLeft) { thickLine(x - 10, y - 9, x + 8, y, INK); thickLine(x + 8, y, x - 10, y + 9, INK); }
      else { thickLine(x + 10, y - 9, x - 8, y, INK); thickLine(x - 8, y, x + 10, y + 9, INK); }
      break;
    case A_GLASSES:
      display.fillRoundRect(x - 17, y - 9, 34, 20, 5, INK);
      display.drawLine(x - 9, y - 6, x - 3, y + 6, ERASE);
      display.drawLine(x - 6, y - 6, x, y + 6, ERASE);
      if (isLeft) display.fillRect(x + 17, y - 6, 15, 3, INK);   // bridge
      break;
    default:
      break;
  }
}

void drawOverlay(uint8_t ov, unsigned long t, int eyeBottom) {
  switch (ov) {
    case O_SPARKLE: {
      int s = (t / 200) % 2 ? 4 : 2;
      sparkleShape(114, 9, s + 1, INK);
      sparkleShape(12, 14, 6 - s, INK);
      break;
    }
    case O_BLUSH:
      for (int side = 0; side < 2; side++) {
        int bx = side ? 80 : 32;
        for (int k = 0; k < 3; k++) display.drawLine(bx + k * 5, eyeBottom + 6, bx + k * 5 + 3, eyeBottom + 2, INK);
      }
      break;
    case O_TEARS: {
      int fall = (int)((t / 25) % 18);
      dropShape(54, eyeBottom + fall - 2, 2, INK);
      dropShape(74, eyeBottom + ((fall + 9) % 18) - 2, 2, INK);
      break;
    }
    case O_SWEAT: {
      int slide = (int)((t / 120) % 8);
      dropShape(112, 6 + slide, 3, INK);
      display.drawPixel(111, 11 + slide, ERASE);
      break;
    }
    case O_QUESTION: {
      int bob = (int)((t / 300) % 2);
      display.setTextSize(2);
      display.setTextColor(INK);
      display.setCursor(112, 1 + bob);
      display.print("?");
      break;
    }
    case O_EXCLAIM:
      display.setTextSize(2);
      display.setTextColor(INK);
      display.setCursor(114, 1);
      display.print("!");
      break;
    case O_DOTS: {
      int n = 1 + (int)((t / 400) % 3);
      for (int i = 0; i < n; i++) display.fillCircle(104 + i * 7, 6, 2, INK);
      break;
    }
    case O_VEIN: {   // anger mark
      int x = 112, y = 6;
      display.drawLine(x - 5, y - 1, x - 1, y - 1, INK); display.drawLine(x - 1, y - 1, x - 1, y - 5, INK);
      display.drawLine(x + 1, y - 5, x + 1, y - 1, INK); display.drawLine(x + 1, y - 1, x + 5, y - 1, INK);
      display.drawLine(x - 5, y + 1, x - 1, y + 1, INK); display.drawLine(x - 1, y + 1, x - 1, y + 5, INK);
      display.drawLine(x + 1, y + 5, x + 1, y + 1, INK); display.drawLine(x + 1, y + 1, x + 5, y + 1, INK);
      break;
    }
    case O_STEAM:
      for (int side = 0; side < 2; side++) {
        int sx = side ? 120 : 6;
        int rise = (int)((t / 90) % 6);
        for (int k = 0; k < 3; k++) {
          int y = 14 - k * 4 - rise;
          if (y > 0) display.drawPixel(sx + ((k + rise) % 2 ? 1 : -1), y, INK);
          if (y > 1) display.drawPixel(sx, y - 1, INK);
        }
      }
      break;
    case O_ZZZ: {
      int rise = (int)((t / 120) % 20);
      display.setTextColor(INK);
      display.setTextSize(1);
      display.setCursor(100, 22 - rise / 2);
      display.print("z");
      display.setTextSize(2);
      display.setCursor(108, 8 - rise / 2);
      display.print("Z");
      break;
    }
    case O_NOTES: {
      int rise = (int)((t / 60) % 30);
      noteShape(108, 30 - rise, INK);
      noteShape(12, 30 - ((rise + 15) % 30), INK);
      break;
    }
    case O_HEART: {
      int rise = (int)((t / 80) % 16);
      heartShape(112, 16 - rise / 2, 4, INK);
      break;
    }
    case O_DROP:
      dropShape(64, 1, 3, INK);
      display.drawPixel(63, 5, ERASE);
      break;
    case O_GLITCH: {
      uint32_t h = hash32((uint32_t)(t / 90));
      for (int k = 0; k < 4; k++) {
        int y = (int)((h >> (k * 6)) % 52) + 6;
        int x = (int)((h >> (k * 5 + 3)) % 60);
        display.fillRect(x, y, 30 + (int)(h % 40), 1 + (int)((h >> 9) % 2), (k % 2) ? ERASE : INK);
      }
      break;
    }
    case O_CONFETTI:
      for (int k = 0; k < 14; k++) {
        uint32_t h = hash32((uint32_t)k * 977u);
        int x = (int)(h % 128);
        int y = (int)(((h >> 8) % 64 + t / (20 + (h % 15))) % 64);
        if ((h >> 4) % 2) display.fillRect(x, y, 2, 2, INK);
        else display.drawPixel(x, y, INK);
      }
      break;
    default:
      break;
  }
}

void drawMouth(uint8_t m, int my, unsigned long t) {
  int cx = 64;
  switch (m) {
    case M_SMILE:
      for (int dx = -7; dx <= 7; dx++) display.drawPixel(cx + dx, my + (49 - dx * dx) / 14, INK);
      for (int dx = -6; dx <= 6; dx++) display.drawPixel(cx + dx, my + 1 + (36 - dx * dx) / 12, INK);
      break;
    case M_GRIN:
      display.fillCircle(cx, my - 1, 8, INK);
      display.fillRect(cx - 9, my - 10, 19, 9, ERASE);
      break;
    case M_FROWN:
      for (int dx = -7; dx <= 7; dx++) display.drawPixel(cx + dx, my + 3 - (49 - dx * dx) / 14, INK);
      for (int dx = -6; dx <= 6; dx++) display.drawPixel(cx + dx, my + 4 - (36 - dx * dx) / 12, INK);
      break;
    case M_O:
      display.drawCircle(cx, my + 1, 3, INK);
      break;
    case M_BIGO: {
      int r = 4 + (int)((t / 300) % 2);
      display.fillCircle(cx, my + 1, r, INK);
      display.fillCircle(cx, my + 1, r - 2, ERASE);
      break;
    }
    case M_SMIRK:
      display.drawLine(cx - 6, my + 2, cx + 6, my - 1, INK);
      display.drawLine(cx - 6, my + 3, cx + 6, my, INK);
      break;
    case M_TONGUE:
      for (int dx = -6; dx <= 6; dx++) display.drawPixel(cx + dx, my + (36 - dx * dx) / 12, INK);
      display.fillCircle(cx + 2, my + 3, 3, INK);
      display.drawPixel(cx + 2, my + 3, ERASE);
      break;
    case M_WAVY:
      for (int dx = -8; dx <= 8; dx++) display.drawPixel(cx + dx, my + ((dx + 8) / 3 % 2 ? 1 : 0), INK);
      break;
    case M_FLAT:
      display.drawFastHLine(cx - 6, my + 1, 13, INK);
      break;
    case M_KISS:
      display.drawCircle(cx, my + 1, 2, INK);
      display.drawPixel(cx + 4, my - 1, INK);
      break;
    default:
      break;
  }
}

// Motion offset for the whole face
void motionOffset(uint8_t mo, unsigned long t, float &dx, float &dy) {
  dx = 0; dy = 0;
  switch (mo) {
    case MO_BOUNCE:  dy = -fabsf(sinf(t / 110.0f)) * 4; break;
    case MO_SHAKE:   dx = ((t / 45) % 2) ? 2 : -2; break;
    case MO_TREMBLE: dx = (float)((int)(hash32((uint32_t)(t / 40)) % 3) - 1); break;
    case MO_BOB:     dy = sinf(t / 230.0f) * 3; break;
    case MO_SWAY:    dx = sinf(t / 380.0f) * 5; break;
    case MO_LOOK:    dx = sinf(t / 650.0f) * 10; break;
    case MO_FLOAT:   dy = sinf(t / 850.0f) * 2; break;
    case MO_GLITCH:  if ((t / 60) % 7 == 0) dx = (float)((int)(hash32((uint32_t)(t / 60)) % 13) - 6); break;
    case MO_BREATH:  dy = sinf(t / 1400.0f) * 1.0f; break;
    default: break;
  }
}

bool faceCanBlink(uint8_t id) {
  const Expr &e = EXPRS[id];
  return e.art == A_NORMAL && id != F_YAWN && id != F_FOCUSED && id != F_WINK && id != F_KISS;
}

// Draws the current face. Call once per frame on a cleared screen.
void drawEyes(unsigned long t) {
  const Expr &e = EXPRS[faceNow];
  // smooth towards the target shape
  const EyeP *tp[2] = {&e.L, &e.R};
  for (int i = 0; i < 2; i++) {
    EyeF tg;
    eyeToF(*tp[i], tg);
    float k = 0.32f;
    curEye[i].x += (tg.x - curEye[i].x) * k;
    curEye[i].y += (tg.y - curEye[i].y) * k;
    curEye[i].w += (tg.w - curEye[i].w) * k;
    curEye[i].h += (tg.h - curEye[i].h) * k;
    curEye[i].r += (tg.r - curEye[i].r) * k;
    curEye[i].lid += (tg.lid - curEye[i].lid) * 0.22f;
    curEye[i].slant += (tg.slant - curEye[i].slant) * k;
    curEye[i].cut += (tg.cut - curEye[i].cut) * k;
    curEye[i].lidB += (tg.lidB - curEye[i].lidB) * k;
  }
  gazeX += (gazeTX - gazeX) * 0.25f;
  gazeY += (gazeTY - gazeY) * 0.25f;

  // blinking
  float blink = 0;
  if (faceCanBlink(faceNow)) {
    if (t >= nextBlink) {
      blinkStart = t;
      blinkTwice = (hash32((uint32_t)t) % 5) == 0;
      nextBlink = t + 2500 + hash32((uint32_t)t + 7) % 3500;
    }
    unsigned long bt = t - blinkStart;
    if (bt < 140) blink = bt < 70 ? bt / 70.0f : (140 - bt) / 70.0f;
    else if (blinkTwice && bt >= 220 && bt < 360) { unsigned long b2 = bt - 220; blink = b2 < 70 ? b2 / 70.0f : (140 - b2) / 70.0f; }
  }

  float mx, my;
  motionOffset(e.motion, t, mx, my);
  bool useGaze = (e.art == A_NORMAL);
  bool hero = heroLook();
  if (hero) drawHeroMask(64 + mx + gazeX * 0.4f, 30 + my + gazeY * 0.4f, 100, true, t);
  int eyeBottom = 0;
  for (int i = 0; i < 2; i++) {
    float baseX = (i == 0) ? 40.0f : 88.0f;
    float cx = baseX + curEye[i].x + mx + (useGaze ? gazeX : 0);
    float cy = 30.0f + curEye[i].y + my + (useGaze ? gazeY : 0);
    if (e.art == A_NORMAL) {
      EyeF drawn = curEye[i];
      if (blink > 0) drawn.lid = max(drawn.lid, drawn.h * blink);
      if (hero) {                                // dark ring so the eye stands out from the mask
        int w = (int)lroundf(drawn.w), h = (int)lroundf(drawn.h);
        display.fillRoundRect((int)lroundf(cx - w / 2.0f) - 2, (int)lroundf(cy - h / 2.0f) - 2, w + 4, h + 4,
                              max(2, (int)lroundf(drawn.r) + 2), ERASE);
      }
      drawEyeShape(cx, cy, drawn, i == 0);
      eyeBottom = max(eyeBottom, (int)(cy + drawn.h / 2));
    } else {
      if (hero) display.fillCircle((int)lroundf(cx), (int)lroundf(cy), 17, ERASE);
      drawArtEye(e.art, cx, cy, i == 0, t);
      eyeBottom = max(eyeBottom, (int)cy + 14);
    }
  }
  drawOverlay(e.ov1, t, eyeBottom);
  drawOverlay(e.ov2, t, eyeBottom);
  if (e.mouth != M_NONE) {
    int mouthY = min(58, max(eyeBottom + 5, 52));
    drawMouth(e.mouth, mouthY + (int)my, t);
  }
}

// Small eyes for other screens (music, water, games...)
void drawMiniEyes(int cx, int cy, uint8_t mood, unsigned long t) {
  int bob = 0;
  if (mood == F_MUSIC) bob = (int)(sinf(t / 230.0f) * 2);
  if (heroLook()) drawHeroMask(cx, cy + 1 + bob, 40, false, t);
  for (int side = -1; side <= 1; side += 2) {
    int x = cx + side * 9 - 5, y = cy - 6 + bob;
    switch (mood) {
      case F_HAPPY: case F_MUSIC: case F_CELEBRATE:
        display.fillRoundRect(x, y, 10, 12, 3, INK);
        display.fillCircle(x + 5, y + 15, 8, ERASE);
        break;
      case F_SLEEPY: case F_THIRSTY:
        display.fillRoundRect(x, y, 10, 12, 3, INK);
        display.fillRect(x, y, 10, 6, ERASE);
        break;
      case F_EFFORT:
        if (side < 0) { display.drawLine(x + 1, y + 2, x + 8, y + 6, INK); display.drawLine(x + 8, y + 6, x + 1, y + 10, INK); }
        else { display.drawLine(x + 8, y + 2, x + 1, y + 6, INK); display.drawLine(x + 1, y + 6, x + 8, y + 10, INK); }
        break;
      case F_SURPRISED:
        display.fillRoundRect(x - 1, y - 2, 12, 15, 5, INK);
        break;
      case F_LAUGH: case F_EXCITED: case F_JOY: case F_PROUD: case F_GRATEFUL:   // happy arcs
        display.fillRoundRect(x, y, 10, 12, 3, INK);
        display.fillCircle(x + 5, y + 14, 7, ERASE);
        break;
      case F_SAD: case F_DISAPPOINTED: case F_WORRIED:                            // drooping outer corner
        display.fillRoundRect(x, y + 2, 10, 10, 3, INK);
        if (side < 0) display.fillTriangle(x, y + 1, x + 10, y + 1, x, y + 7, ERASE);
        else display.fillTriangle(x, y + 1, x + 10, y + 1, x + 10, y + 7, ERASE);
        break;
      case F_ANNOYED: case F_MISCHIEVOUS:                                         // half-closed, sly
        display.fillRoundRect(x, y, 10, 12, 3, INK);
        display.fillRect(x, y, 10, 5, ERASE);
        if (side < 0) display.fillTriangle(x, y + 5, x + 10, y + 5, x + 10, y + 8, ERASE);
        else display.fillTriangle(x, y + 5, x + 10, y + 5, x, y + 8, ERASE);
        break;
      case F_CONFUSED:                                                             // one big, one small
        if (side < 0) display.fillRoundRect(x + 2, y + 3, 7, 8, 2, INK);
        else display.fillRoundRect(x - 1, y - 1, 12, 14, 4, INK);
        break;
      case F_LOVE:
        heartShape(x + 5, y + 6, 3, INK);
        break;
      default:
        display.fillRoundRect(x, y, 10, 12, 3, INK);
        break;
    }
  }
}
