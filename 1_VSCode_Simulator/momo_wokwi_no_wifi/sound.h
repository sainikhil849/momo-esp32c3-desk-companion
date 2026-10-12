// =====================================================================
//  sound.h  -  Momo's voice: sound themes, volume, affection sounds,
//              alarm melodies and your own tune
//  The passive buzzer is driven by the chip's PWM timer (LEDC).
// =====================================================================

struct Note { uint16_t f; uint16_t ms; };       // f = 0 means a pause

enum Snd : uint8_t {
  S_PET, S_PET2, S_PURR, S_GIGGLE, S_LAUGH, S_LOVE, S_KISS,
  S_WAKE, S_GOODNIGHT, S_YAWN, S_NOTIFY, S_RING, S_ALERT, S_WATER, S_STRETCH, S_DONE,
  S_CLICK, S_BACK, S_DIZZY, S_HUG, S_GAME_START, S_POINT, S_GAME_OVER, S_NEW_BEST,
  S_ALARM_CLASSIC, S_ALARM_RISING, S_ALARM_SONG, S_TUNE, S_CHIME, S_SURPRISE, S_SAD,
  S_PAIR, S_ERROR, S_COUNT
};

enum SndCat : uint8_t { C_PETS, C_REMIND, C_NOTIFY, C_GAMES, C_ALARM, C_UI };

// Base melodies. The theme changes how they sound when they are played.
const Note T_PET[]       = {{1500, 60}, {2000, 70}};
const Note T_PET2[]      = {{1800, 50}, {1600, 50}, {2200, 80}};
const Note T_PURR[]      = {{420, 45}, {0, 15}, {440, 45}, {0, 15}, {420, 45}, {0, 15}, {460, 70}};
const Note T_GIGGLE[]    = {{1900, 45}, {0, 25}, {2100, 45}, {0, 25}, {2300, 45}, {0, 25}, {2600, 70}};
const Note T_LAUGH[]     = {{1600, 60}, {0, 30}, {1900, 60}, {0, 30}, {1600, 60}, {0, 30}, {2200, 60}, {0, 30}, {2600, 120}};
const Note T_LOVE[]      = {{1800, 90}, {2200, 90}, {2600, 90}, {0, 40}, {2600, 70}, {3100, 180}};
const Note T_KISS[]      = {{2400, 40}, {2900, 40}, {3400, 110}};
const Note T_WAKE[]      = {{900, 80}, {1200, 80}, {1500, 120}};
const Note T_GOODNIGHT[] = {{1300, 140}, {1100, 140}, {900, 260}};
const Note T_YAWN[]      = {{700, 120}, {600, 160}, {500, 220}};
const Note T_NOTIFY[]    = {{2200, 70}, {0, 40}, {2800, 90}};
const Note T_RING[]      = {{1600, 160}, {0, 80}, {1600, 160}};
const Note T_ALERT[]     = {{2000, 110}, {0, 70}, {2000, 110}};
const Note T_WATER[]     = {{1200, 70}, {1500, 70}, {1900, 70}, {0, 60}, {1500, 70}, {1900, 120}};
const Note T_STRETCH[]   = {{900, 100}, {1200, 100}, {1600, 100}, {2000, 200}};
const Note T_DONE[]      = {{1500, 100}, {2000, 100}, {2500, 220}};
const Note T_CLICK[]     = {{1100, 25}};
const Note T_BACK[]      = {{900, 30}};
const Note T_DIZZY[]     = {{900, 90}, {700, 90}, {500, 160}};
const Note T_HUG[]       = {{1200, 90}, {1600, 90}, {2000, 160}};
const Note T_GSTART[]    = {{1000, 70}, {1300, 70}, {1600, 70}, {2000, 140}};
const Note T_POINT[]     = {{2200, 35}};
const Note T_GOVER[]     = {{900, 140}, {700, 140}, {500, 300}};
const Note T_NEWBEST[]   = {{1300, 80}, {1600, 80}, {2000, 80}, {2600, 80}, {0, 50}, {2000, 80}, {2600, 260}};
const Note T_AL_CLASSIC[] = {{2000, 120}, {0, 80}, {2000, 120}, {0, 80}, {2000, 120}, {0, 80}, {2000, 120}};
const Note T_AL_RISING[]  = {{800, 120}, {1000, 120}, {1250, 120}, {1600, 120}, {2000, 120}, {2500, 200}};
const Note T_AL_SONG[]    = {{1319, 150}, {1568, 150}, {2093, 300}, {1760, 150}, {1568, 150}, {1319, 300}, {1047, 150}, {1175, 150}, {1319, 300}};
const Note T_CHIME[]      = {{1568, 120}, {1319, 220}};
const Note T_SURPRISE[]   = {{1200, 50}, {2400, 120}};
const Note T_SAD[]        = {{1000, 160}, {850, 160}, {700, 280}};
const Note T_PAIR[]       = {{1500, 80}, {0, 40}, {1500, 80}, {0, 40}, {2500, 160}};
const Note T_ERROR[]      = {{400, 180}, {0, 60}, {300, 260}};

struct SndInfo { const Note *notes; uint8_t n; SndCat cat; };
#define SND(t, c) {t, (uint8_t)(sizeof(t) / sizeof(t[0])), c}
const SndInfo SOUNDS[S_COUNT] = {
  SND(T_PET, C_PETS), SND(T_PET2, C_PETS), SND(T_PURR, C_PETS), SND(T_GIGGLE, C_PETS), SND(T_LAUGH, C_PETS),
  SND(T_LOVE, C_PETS), SND(T_KISS, C_PETS),
  SND(T_WAKE, C_UI), SND(T_GOODNIGHT, C_UI), SND(T_YAWN, C_PETS), SND(T_NOTIFY, C_NOTIFY), SND(T_RING, C_NOTIFY),
  SND(T_ALERT, C_NOTIFY), SND(T_WATER, C_REMIND), SND(T_STRETCH, C_REMIND), SND(T_DONE, C_REMIND),
  SND(T_CLICK, C_UI), SND(T_BACK, C_UI), SND(T_DIZZY, C_PETS), SND(T_HUG, C_NOTIFY), SND(T_GSTART, C_GAMES),
  SND(T_POINT, C_GAMES), SND(T_GOVER, C_GAMES), SND(T_NEWBEST, C_GAMES),
  SND(T_AL_CLASSIC, C_ALARM), SND(T_AL_RISING, C_ALARM), SND(T_AL_SONG, C_ALARM), {nullptr, 0, C_ALARM},
  SND(T_CHIME, C_UI), SND(T_SURPRISE, C_PETS), SND(T_SAD, C_PETS), SND(T_PAIR, C_UI), SND(T_ERROR, C_UI)
};

const char *SOUND_NAMES[S_COUNT] = {
  "pet", "pet2", "purr", "giggle", "laugh", "love", "kiss", "wake", "goodnight", "yawn", "notify", "ring",
  "alert", "water", "stretch", "done", "click", "back", "dizzy", "hug", "game_start", "point", "game_over",
  "new_best", "alarm_classic", "alarm_rising", "alarm_song", "tune", "chime", "surprise", "sad", "pair", "error"
};

const uint8_t TUNE_MAX = 40;
Note tuneBuf[TUNE_MAX];
uint8_t tuneLen = 0, tunePos = 0;
unsigned long tuneNext = 0;
bool tuneIsAlarm = false;
uint16_t tuneDuty = 0;

bool soundBlocked(SndCat cat);   // in schedule.h: quiet hours / night

// ---------- buzzer ----------
#if ESP_ARDUINO_VERSION_MAJOR >= 3
uint32_t buzzerFreq = 0;
bool buzzerOn = false;
unsigned long buzzerOffAt = 0;

void buzzerStart(uint16_t freq, uint16_t ms, uint16_t duty, unsigned long now) {
  if (buzzerFreq != freq) {
    if (buzzerFreq != 0) ledcDetach(PIN_BUZZER);
    buzzerFreq = ledcAttach(PIN_BUZZER, freq, 10) ? freq : 0;
  }
  if (buzzerFreq == 0) return;
  ledcWrite(PIN_BUZZER, duty);
  buzzerOn = true;
  buzzerOffAt = now + ms;
}

void buzzerUpdate(unsigned long now) {
  if (buzzerOn && (long)(now - buzzerOffAt) >= 0) {
    ledcWrite(PIN_BUZZER, 0);
    buzzerOn = false;
  }
}

void buzzerStop() {
  if (buzzerOn) ledcWrite(PIN_BUZZER, 0);
  buzzerOn = false;
}
#else
void buzzerStart(uint16_t freq, uint16_t ms, uint16_t duty, unsigned long now) { (void)duty; (void)now; tone(PIN_BUZZER, freq, ms); }
void buzzerUpdate(unsigned long now) { (void)now; }
void buzzerStop() { noTone(PIN_BUZZER); }
#endif

// Louder = wider pulses (10-bit PWM; 512 = 50% = loudest for a passive buzzer)
uint16_t volumeDuty(uint8_t vol) {
  switch (vol) {
    case 0: return 0;
    case 1: return 30;
    case 2: return 130;
    default: return 512;
  }
}

// ---------- your own tune: "C5/8 E5/8 G5/4 R/8 C6/2" ----------
// Note letter A-G, optional # or b, octave 1-8 (default 5); R = rest.
// /1 whole, /2 half, /4 quarter, /8 eighth (default), /16 sixteenth.
uint8_t parseTune(const char *s, Note *out, uint8_t maxNotes) {
  static const int8_t semis[7] = {9, 11, 0, 2, 4, 5, 7};   // A B C D E F G
  uint8_t n = 0;
  const char *p = s;
  while (p != nullptr && *p != '\0' && n < maxNotes) {
    while (*p == ' ' || *p == ',' || *p == '\t') p++;
    if (*p == '\0') break;
    char c = (char)toupper((unsigned char)*p);
    bool rest = (c == 'R' || c == '-');
    int semitone = 0, octave = 5;
    bool ok = rest || (c >= 'A' && c <= 'G');
    if (!ok) {                       // skip anything we do not understand
      while (*p != '\0' && *p != ' ' && *p != ',') p++;
      continue;
    }
    p++;
    if (!rest) {
      semitone = semis[c - 'A'];
      if (*p == '#') { semitone++; p++; }
      else if (*p == 'b') { semitone--; p++; }
      if (*p >= '1' && *p <= '8') { octave = *p - '0'; p++; }
    }
    int len = 8;
    if (*p == '/') {
      p++;
      int v = 0;
      while (*p >= '0' && *p <= '9') { v = v * 10 + (*p - '0'); p++; }
      if (v == 1 || v == 2 || v == 4 || v == 8 || v == 16) len = v;
    }
    while (*p != '\0' && *p != ' ' && *p != ',') p++;   // ignore extra characters
    uint16_t ms = (uint16_t)(1600 / len);               // whole note = 1.6 s
    if (rest) {
      out[n].f = 0;
    } else {
      int midi = (octave + 1) * 12 + semitone;
      float f = 440.0f * powf(2.0f, (midi - 69) / 12.0f);
      if (f < 100) f = 100;
      if (f > 8000) f = 8000;
      out[n].f = (uint16_t)f;
    }
    out[n].ms = ms;
    n++;
  }
  return n;
}

// ---------- Hero sounds (Momo 2.2) ----------
// Momo's own little-superhero voice: short fanfares and stingers written for
// Momo (original melodies). Used when the sound theme is "Hero" (4).
const Note HS_WAKE[]     = {{784, 90}, {1047, 90}, {1319, 90}, {1568, 260}};
const Note HS_GOODNIGHT[] = {{1568, 160}, {1319, 160}, {1047, 320}};
const Note HS_WATER[]    = {{1047, 60}, {0, 30}, {1047, 60}, {1568, 80}, {2093, 200}};
const Note HS_STRETCH[]  = {{784, 120}, {1175, 120}, {1568, 120}, {1175, 80}, {1568, 260}};
const Note HS_DONE[]     = {{1319, 80}, {1568, 80}, {0, 40}, {1568, 80}, {2093, 300}};
const Note HS_GSTART[]   = {{1047, 70}, {1319, 70}, {1568, 70}, {2093, 70}, {0, 40}, {2093, 180}};
const Note HS_NEWBEST[]  = {{1568, 80}, {2093, 80}, {2637, 80}, {0, 50}, {2349, 80}, {2637, 80}, {3136, 300}};
const Note HS_GOVER[]    = {{1568, 140}, {1397, 140}, {1175, 140}, {1047, 320}};
const Note HS_HUG[]      = {{1319, 90}, {1760, 90}, {2093, 90}, {2637, 220}};
const Note HS_PET[]      = {{1568, 50}, {2093, 80}};
const Note HS_PURR[]     = {{523, 60}, {0, 15}, {587, 60}, {0, 15}, {523, 60}, {0, 15}, {659, 90}};
const Note HS_NOTIFY[]   = {{2093, 60}, {0, 30}, {2637, 60}, {3136, 90}};
const Note HS_CHIME[]    = {{1568, 100}, {2093, 220}};
const Note HS_SAD[]      = {{1175, 160}, {1047, 160}, {880, 300}};
const Note HS_CHARGE[]   = {{1047, 60}, {1319, 60}, {1568, 60}, {2093, 60}, {2637, 140}};
const Note HS_LOWBATT[]  = {{1568, 120}, {1175, 120}, {880, 120}, {587, 260}};
const Note HS_SLEEP[]    = {{2093, 100}, {1568, 100}, {1047, 100}, {523, 300}};

struct HeroSnd { uint8_t id; const Note *notes; uint8_t n; };
#define HSND(id, t) {id, t, (uint8_t)(sizeof(t) / sizeof(t[0]))}
const HeroSnd HERO_SOUNDS[] = {
  HSND(S_WAKE, HS_WAKE), HSND(S_GOODNIGHT, HS_GOODNIGHT), HSND(S_WATER, HS_WATER), HSND(S_STRETCH, HS_STRETCH),
  HSND(S_DONE, HS_DONE), HSND(S_GAME_START, HS_GSTART), HSND(S_NEW_BEST, HS_NEWBEST), HSND(S_GAME_OVER, HS_GOVER),
  HSND(S_HUG, HS_HUG), HSND(S_PET, HS_PET), HSND(S_PURR, HS_PURR), HSND(S_NOTIFY, HS_NOTIFY), HSND(S_CHIME, HS_CHIME),
  HSND(S_SAD, HS_SAD)
};
const uint8_t HERO_SOUND_COUNT = (uint8_t)(sizeof(HERO_SOUNDS) / sizeof(HERO_SOUNDS[0]));

// ---------- themes ----------
// 0 cute: higher and quicker  1 robot: stepped, clipped beeps
// 2 soft: lower and gentler   3 retro: 8-bit arpeggios
// 4 hero: a little "ta-" pick-up note and a longer last note (fanfare style)
uint8_t themeNotes(const Note *src, uint8_t n, uint8_t theme, Note *out, uint8_t maxOut) {
  uint8_t k = 0;
  if (theme == 4) {
    uint8_t first = 0;
    while (first < n && src[first].f == 0) first++;
    if (first < n && k < maxOut) { out[k].f = (uint16_t)(src[first].f * 3 / 4); out[k].ms = 40; k++; }
    for (uint8_t i = 0; i < n && k < maxOut; i++) {
      out[k] = src[i];
      if (i + 1 == n && out[k].f != 0) out[k].ms = (uint16_t)min(600, (int)(out[k].ms * 3 / 2));
      k++;
    }
    return k;
  }
  for (uint8_t i = 0; i < n && k < maxOut; i++) {
    Note x = src[i];
    if (x.f == 0) { out[k++] = x; continue; }
    switch (theme) {
      case 1: {   // robot
        uint16_t f = (uint16_t)((x.f / 200) * 200 + 200);
        uint16_t on = (uint16_t)max(20, (int)(x.ms * 6 / 10));
        out[k].f = f; out[k].ms = on; k++;
        if (k < maxOut) { out[k].f = 0; out[k].ms = (uint16_t)max(10, (int)(x.ms - on)); k++; }
        break;
      }
      case 2:     // soft
        out[k].f = (uint16_t)(x.f * 3 / 4); out[k].ms = (uint16_t)(x.ms * 13 / 10); k++;
        break;
      case 3: {   // retro arpeggio: note, third, fifth
        uint16_t part = (uint16_t)max(15, x.ms / 3);
        const uint16_t mul[3] = {100, 126, 150};
        for (uint8_t j = 0; j < 3 && k < maxOut; j++) { out[k].f = (uint16_t)((uint32_t)x.f * mul[j] / 100); out[k].ms = part; k++; }
        break;
      }
      default:    // cute
        out[k].f = (uint16_t)min(6000, (int)(x.f * 5 / 4)); out[k].ms = (uint16_t)max(20, (int)(x.ms * 8 / 10)); k++;
        break;
    }
  }
  return k;
}

bool categoryOn(SndCat cat) {
  switch (cat) {
    case C_PETS:   return cfg.sndPets;
    case C_REMIND: return cfg.sndReminders;
    case C_NOTIFY: return cfg.sndNotify;
    case C_GAMES:  return cfg.sndGames;
    default:       return true;
  }
}

void startNotes(const Note *notes, uint8_t n, bool alarm, uint16_t duty) {
  tuneLen = n;
  tunePos = 0;
  tuneNext = 0;
  tuneIsAlarm = alarm;
  tuneDuty = duty;
  if (notes != tuneBuf) memcpy(tuneBuf, notes, sizeof(Note) * n);
}

// Play one of Momo's sounds, following the theme, volume, quiet hours and toggles.
// Returns true if it will be heard.
bool playSound(Snd s) {
  if (s >= S_COUNT) return false;
  const SndInfo &info = SOUNDS[s];
  bool alarm = (info.cat == C_ALARM);
  if (!alarm) {
    if (cfg.volume == 0 || !categoryOn(info.cat) || soundBlocked(info.cat)) return false;
    if (tuneIsAlarm && tunePos < tuneLen) return false;   // never cut an alarm short
  }
  uint8_t vol = alarm ? (uint8_t)max(1, (int)cfg.volume) : cfg.volume;
  Note tmp[TUNE_MAX];
  uint8_t n;
  n = 0;
  if (s == S_TUNE) n = parseTune(cfg.tune, tmp, TUNE_MAX);
  else if (alarm) { n = (uint8_t)min((int)info.n, (int)TUNE_MAX); memcpy(tmp, info.notes, sizeof(Note) * n); }   // alarms keep their melody
  else {
    if (cfg.theme == 4)
      for (uint8_t i = 0; i < HERO_SOUND_COUNT; i++)
        if (HERO_SOUNDS[i].id == s) { n = HERO_SOUNDS[i].n; memcpy(tmp, HERO_SOUNDS[i].notes, sizeof(Note) * n); break; }
    if (n == 0) n = themeNotes(info.notes, info.n, cfg.theme, tmp, TUNE_MAX);
  }
  if (n == 0) return false;
  startNotes(tmp, n, alarm, volumeDuty(vol));
  return true;
}

// ---------- a sound for every emotion (46 faces, each one different) ----------
// Index = face number (see FACE_NAMES in face.h). Played when Momo reacts.
const Note FS_NEUTRAL[]      = {{1200, 50}, {1400, 60}};
const Note FS_HAPPY[]        = {{1500, 60}, {2000, 70}};
const Note FS_JOY[]          = {{1600, 50}, {2000, 50}, {2400, 50}, {3000, 110}};
const Note FS_LAUGH[]        = {{1900, 45}, {0, 25}, {2100, 45}, {0, 25}, {2300, 45}, {0, 25}, {2600, 70}};
const Note FS_LOVE[]         = {{1800, 90}, {2200, 90}, {2600, 90}, {0, 40}, {2600, 70}, {3100, 180}};
const Note FS_BLUSH[]        = {{1800, 50}, {1600, 50}, {2200, 80}};
const Note FS_WINK[]         = {{2000, 40}, {0, 40}, {2800, 80}};
const Note FS_SLEEPY[]       = {{900, 160}, {750, 220}};
const Note FS_ASLEEP[]       = {{500, 200}, {0, 120}, {450, 260}};
const Note FS_BORED[]        = {{700, 180}, {700, 180}, {650, 260}};
const Note FS_SAD[]          = {{1000, 160}, {850, 160}, {700, 280}};
const Note FS_CRY[]          = {{1300, 120}, {1200, 120}, {1300, 120}, {1100, 300}};
const Note FS_ANGRY[]        = {{300, 90}, {0, 30}, {300, 90}, {0, 30}, {260, 180}};
const Note FS_FURIOUS[]      = {{220, 60}, {330, 60}, {220, 60}, {330, 60}, {200, 220}};
const Note FS_SURPRISED[]    = {{1200, 50}, {2400, 120}};
const Note FS_SHOCKED[]      = {{800, 40}, {3000, 200}};
const Note FS_SCARED[]       = {{2600, 40}, {2500, 40}, {2600, 40}, {2500, 40}, {2600, 40}, {2400, 120}};
const Note FS_CONFUSED[]     = {{1000, 90}, {1300, 90}, {1000, 90}, {1500, 150}};
const Note FS_CURIOUS[]      = {{1100, 70}, {1500, 140}};
const Note FS_THINKING[]     = {{900, 100}, {0, 60}, {950, 100}, {0, 60}, {1000, 160}};
const Note FS_SUSPICIOUS[]   = {{700, 120}, {650, 200}};
const Note FS_ANNOYED[]      = {{600, 80}, {550, 80}, {500, 160}};
const Note FS_DIZZY[]        = {{900, 90}, {700, 90}, {500, 160}};
const Note FS_EXCITED[]      = {{1500, 40}, {1800, 40}, {2100, 40}, {2400, 40}, {2700, 40}, {3200, 100}};
const Note FS_PROUD[]        = {{1300, 100}, {1600, 100}, {2000, 250}};
const Note FS_RELAXED[]      = {{420, 45}, {0, 15}, {440, 45}, {0, 15}, {420, 45}, {0, 15}, {460, 70}};
const Note FS_FOCUSED[]      = {{1500, 30}, {0, 70}, {1500, 30}};
const Note FS_DETERMINED[]   = {{800, 80}, {1000, 80}, {1200, 160}};
const Note FS_COOL[]         = {{1000, 60}, {0, 30}, {1500, 180}};
const Note FS_MISCHIEVOUS[]  = {{1700, 50}, {1400, 50}, {1900, 50}, {1500, 90}};
const Note FS_GRATEFUL[]     = {{1400, 90}, {1800, 90}, {2200, 140}};
const Note FS_KISS[]         = {{2400, 40}, {2900, 40}, {3400, 110}};
const Note FS_YAWN[]         = {{700, 120}, {600, 160}, {500, 220}};
const Note FS_SILLY[]        = {{800, 50}, {1600, 50}, {800, 50}, {1600, 50}, {1200, 100}};
const Note FS_WORRIED[]      = {{1100, 120}, {1050, 120}, {1000, 200}};
const Note FS_NERVOUS[]      = {{1400, 30}, {0, 30}, {1350, 30}, {0, 30}, {1400, 30}, {0, 30}, {1300, 90}};
const Note FS_EMBARRASSED[]  = {{1600, 60}, {1300, 60}, {1000, 120}};
const Note FS_DISAPPOINTED[] = {{900, 150}, {0, 50}, {650, 300}};
const Note FS_HOPEFUL[]      = {{1000, 100}, {1300, 100}, {1700, 180}};
const Note FS_PLAYFUL[]      = {{1500, 50}, {1100, 50}, {1500, 50}, {1900, 100}};
const Note FS_SNEAKY[]       = {{500, 60}, {0, 80}, {550, 60}, {0, 80}, {600, 60}};
const Note FS_GLITCH[]       = {{3000, 20}, {400, 20}, {2500, 20}, {0, 20}, {3500, 20}, {600, 40}, {2000, 60}};
const Note FS_MUSIC[]        = {{1319, 100}, {1568, 100}, {1760, 100}, {1568, 150}};
const Note FS_THIRSTY[]      = {{1200, 70}, {0, 40}, {1200, 70}, {900, 160}};
const Note FS_EFFORT[]       = {{400, 100}, {450, 100}, {500, 100}, {600, 200}};
const Note FS_CELEBRATE[]    = {{1300, 80}, {1600, 80}, {2000, 80}, {2600, 80}, {0, 40}, {2000, 80}, {2600, 240}};

// Hero versions: every emotion has its own little heroic sound (also all different)
const Note HF_NEUTRAL[]      = {{1319, 50}, {1568, 70}};
const Note HF_HAPPY[]        = {{1568, 60}, {2093, 90}};
const Note HF_JOY[]          = {{1047, 50}, {1568, 50}, {2093, 50}, {2637, 140}};
const Note HF_LAUGH[]        = {{2093, 40}, {0, 20}, {2349, 40}, {0, 20}, {2637, 40}, {0, 20}, {2349, 40}, {0, 20}, {2637, 90}};
const Note HF_LOVE[]         = {{1568, 100}, {1976, 100}, {2349, 100}, {0, 40}, {2349, 80}, {2637, 220}};
const Note HF_BLUSH[]        = {{2093, 50}, {1760, 50}, {2349, 90}};
const Note HF_WINK[]         = {{2349, 40}, {0, 40}, {3136, 90}};
const Note HF_SLEEPY[]       = {{1047, 160}, {880, 240}};
const Note HF_ASLEEP[]       = {{587, 200}, {0, 120}, {523, 280}};
const Note HF_BORED[]        = {{784, 180}, {740, 180}, {698, 280}};
const Note HF_SAD[]          = {{1175, 160}, {988, 160}, {880, 300}};
const Note HF_CRY[]          = {{1397, 110}, {1319, 110}, {1397, 110}, {1175, 320}};
const Note HF_ANGRY[]        = {{392, 80}, {0, 30}, {392, 80}, {0, 30}, {349, 200}};
const Note HF_FURIOUS[]      = {{262, 60}, {392, 60}, {262, 60}, {392, 60}, {247, 240}};
const Note HF_SURPRISED[]    = {{1319, 50}, {2637, 130}};
const Note HF_SHOCKED[]      = {{988, 40}, {3136, 220}};
const Note HF_SCARED[]       = {{2794, 40}, {2637, 40}, {2794, 40}, {2637, 40}, {2794, 40}, {2349, 140}};
const Note HF_CONFUSED[]     = {{1175, 90}, {1397, 90}, {1175, 90}, {1568, 160}};
const Note HF_CURIOUS[]      = {{1319, 70}, {1760, 150}};
const Note HF_THINKING[]     = {{1047, 100}, {0, 60}, {1109, 100}, {0, 60}, {1175, 170}};
const Note HF_SUSPICIOUS[]   = {{784, 120}, {740, 220}};
const Note HF_ANNOYED[]      = {{698, 80}, {659, 80}, {587, 170}};
const Note HF_DIZZY[]        = {{1047, 90}, {831, 90}, {659, 90}, {523, 170}};
const Note HF_EXCITED[]      = {{1568, 40}, {1760, 40}, {1976, 40}, {2349, 40}, {2637, 40}, {3136, 120}};
const Note HF_PROUD[]        = {{1047, 100}, {1568, 100}, {2093, 280}};
const Note HF_RELAXED[]      = {{523, 50}, {0, 15}, {587, 50}, {0, 15}, {659, 50}, {0, 15}, {587, 90}};
const Note HF_FOCUSED[]      = {{1760, 30}, {0, 70}, {1760, 30}, {0, 70}, {2093, 40}};
const Note HF_DETERMINED[]   = {{784, 80}, {1047, 80}, {1568, 180}};
const Note HF_COOL[]         = {{1175, 60}, {0, 30}, {1760, 200}};
const Note HF_MISCHIEVOUS[]  = {{1976, 50}, {1568, 50}, {2093, 50}, {1760, 90}};
const Note HF_GRATEFUL[]     = {{1568, 90}, {1976, 90}, {2349, 150}};
const Note HF_KISS[]         = {{2637, 40}, {3136, 40}, {3520, 110}};
const Note HF_YAWN[]         = {{880, 120}, {740, 160}, {587, 230}};
const Note HF_SILLY[]        = {{880, 50}, {1760, 50}, {880, 50}, {1760, 50}, {1319, 110}};
const Note HF_WORRIED[]      = {{1245, 120}, {1175, 120}, {1109, 200}};
const Note HF_NERVOUS[]      = {{1568, 30}, {0, 30}, {1480, 30}, {0, 30}, {1568, 30}, {0, 30}, {1397, 90}};
const Note HF_EMBARRASSED[]  = {{1760, 60}, {1397, 60}, {1175, 130}};
const Note HF_DISAPPOINTED[] = {{1047, 150}, {0, 50}, {740, 300}};
const Note HF_HOPEFUL[]      = {{1175, 100}, {1480, 100}, {1760, 190}};
const Note HF_PLAYFUL[]      = {{1760, 50}, {1319, 50}, {1760, 50}, {2093, 110}};
const Note HF_SNEAKY[]       = {{587, 60}, {0, 80}, {659, 60}, {0, 80}, {740, 60}, {0, 60}, {784, 40}};
const Note HF_GLITCH[]       = {{3136, 20}, {440, 20}, {2637, 20}, {0, 20}, {3520, 20}, {659, 40}, {2093, 60}};
const Note HF_MUSIC[]        = {{1568, 100}, {1760, 100}, {2093, 100}, {1760, 150}};
const Note HF_THIRSTY[]      = {{1319, 70}, {0, 40}, {1319, 70}, {988, 170}};
const Note HF_EFFORT[]       = {{440, 100}, {494, 100}, {587, 100}, {784, 220}};
const Note HF_CELEBRATE[]    = {{1047, 80}, {1319, 80}, {1568, 80}, {2093, 80}, {0, 40}, {1568, 80}, {2093, 260}};

struct FaceSnd { const Note *notes; uint8_t n; };
#define FSND(t) {t, (uint8_t)(sizeof(t) / sizeof(t[0]))}
const FaceSnd FACE_SOUNDS[] = {
  FSND(FS_NEUTRAL), FSND(FS_HAPPY), FSND(FS_JOY), FSND(FS_LAUGH), FSND(FS_LOVE), FSND(FS_BLUSH), FSND(FS_WINK),
  FSND(FS_SLEEPY), FSND(FS_ASLEEP), FSND(FS_BORED), FSND(FS_SAD), FSND(FS_CRY), FSND(FS_ANGRY), FSND(FS_FURIOUS),
  FSND(FS_SURPRISED), FSND(FS_SHOCKED), FSND(FS_SCARED), FSND(FS_CONFUSED), FSND(FS_CURIOUS), FSND(FS_THINKING),
  FSND(FS_SUSPICIOUS), FSND(FS_ANNOYED), FSND(FS_DIZZY), FSND(FS_EXCITED), FSND(FS_PROUD), FSND(FS_RELAXED),
  FSND(FS_FOCUSED), FSND(FS_DETERMINED), FSND(FS_COOL), FSND(FS_MISCHIEVOUS), FSND(FS_GRATEFUL), FSND(FS_KISS),
  FSND(FS_YAWN), FSND(FS_SILLY), FSND(FS_WORRIED), FSND(FS_NERVOUS), FSND(FS_EMBARRASSED), FSND(FS_DISAPPOINTED),
  FSND(FS_HOPEFUL), FSND(FS_PLAYFUL), FSND(FS_SNEAKY), FSND(FS_GLITCH), FSND(FS_MUSIC), FSND(FS_THIRSTY),
  FSND(FS_EFFORT), FSND(FS_CELEBRATE)
};
const uint8_t FACE_SOUND_COUNT = (uint8_t)(sizeof(FACE_SOUNDS) / sizeof(FACE_SOUNDS[0]));
const FaceSnd HERO_FACE_SOUNDS[] = {
  FSND(HF_NEUTRAL), FSND(HF_HAPPY), FSND(HF_JOY), FSND(HF_LAUGH), FSND(HF_LOVE), FSND(HF_BLUSH), FSND(HF_WINK),
  FSND(HF_SLEEPY), FSND(HF_ASLEEP), FSND(HF_BORED), FSND(HF_SAD), FSND(HF_CRY), FSND(HF_ANGRY), FSND(HF_FURIOUS),
  FSND(HF_SURPRISED), FSND(HF_SHOCKED), FSND(HF_SCARED), FSND(HF_CONFUSED), FSND(HF_CURIOUS), FSND(HF_THINKING),
  FSND(HF_SUSPICIOUS), FSND(HF_ANNOYED), FSND(HF_DIZZY), FSND(HF_EXCITED), FSND(HF_PROUD), FSND(HF_RELAXED),
  FSND(HF_FOCUSED), FSND(HF_DETERMINED), FSND(HF_COOL), FSND(HF_MISCHIEVOUS), FSND(HF_GRATEFUL), FSND(HF_KISS),
  FSND(HF_YAWN), FSND(HF_SILLY), FSND(HF_WORRIED), FSND(HF_NERVOUS), FSND(HF_EMBARRASSED), FSND(HF_DISAPPOINTED),
  FSND(HF_HOPEFUL), FSND(HF_PLAYFUL), FSND(HF_SNEAKY), FSND(HF_GLITCH), FSND(HF_MUSIC), FSND(HF_THIRSTY),
  FSND(HF_EFFORT), FSND(HF_CELEBRATE)
};
const uint8_t HERO_FACE_SOUND_COUNT = (uint8_t)(sizeof(HERO_FACE_SOUNDS) / sizeof(HERO_FACE_SOUNDS[0]));

// Play the sound of one emotion (theme, volume, quiet hours and the
// "affection sounds" switch apply). Returns true if it will be heard.
bool playFaceSound(uint8_t face) {
  if (face >= FACE_SOUND_COUNT) return false;
  if (cfg.volume == 0 || !categoryOn(C_PETS) || soundBlocked(C_PETS)) return false;
  if (tuneIsAlarm && tunePos < tuneLen) return false;
  Note tmp[TUNE_MAX];
  uint8_t n;
  if (cfg.theme == 4 && face < HERO_FACE_SOUND_COUNT) {   // hero: its own melody for every emotion
    n = HERO_FACE_SOUNDS[face].n;
    memcpy(tmp, HERO_FACE_SOUNDS[face].notes, sizeof(Note) * n);
  } else {
    n = themeNotes(FACE_SOUNDS[face].notes, FACE_SOUNDS[face].n, cfg.theme, tmp, TUNE_MAX);
  }
  if (n == 0) return false;
  startNotes(tmp, n, false, volumeDuty(cfg.volume));
  return true;
}

// Battery, sleep and charging sounds (2.2): follow volume and quiet hours like the others
bool playNotes(const Note *notes, uint8_t n, SndCat cat) {
  if (cfg.volume == 0 || !categoryOn(cat) || soundBlocked(cat)) return false;
  if (tuneIsAlarm && tunePos < tuneLen) return false;
  startNotes(notes, n, false, volumeDuty(cfg.volume));
  return true;
}
#define PLAY_NOTES(t, cat) playNotes(t, (uint8_t)(sizeof(t) / sizeof(t[0])), cat)

// One short tone for the games (Simon, Copy the move, Rhythm, Rock paper scissors)
bool playGameTone(uint16_t freq, uint16_t ms) {
  if (cfg.volume == 0 || !categoryOn(C_GAMES) || soundBlocked(C_GAMES)) return false;
  if (tuneIsAlarm && tunePos < tuneLen) return false;
  Note one[1] = {{freq, ms}};
  startNotes(one, 1, false, volumeDuty(cfg.volume));
  return true;
}

// Your own reminders ring with this (like an alarm: also in quiet hours)
const Note T_MY_REMINDER[] = {{1568, 90}, {2093, 90}, {2637, 160}, {0, 80}, {1568, 90}, {2093, 90}, {2637, 160}};
void playMyReminder() {
  startNotes(T_MY_REMINDER, (uint8_t)(sizeof(T_MY_REMINDER) / sizeof(T_MY_REMINDER[0])), true, volumeDuty((uint8_t)max(1, (int)cfg.volume)));
}

// Alarm melody by number (0 classic, 1 rising, 2 Momo song, 3 your own tune)
Snd alarmSound(uint8_t which) {
  switch (which) {
    case 0: return S_ALARM_CLASSIC;
    case 1: return S_ALARM_RISING;
    case 3: return S_TUNE;
    default: return S_ALARM_SONG;
  }
}

void playAlarm(uint8_t which, uint8_t loudness) {
  Snd s = alarmSound(which);
  Note tmp[TUNE_MAX];
  uint8_t n;
  if (s == S_TUNE) n = parseTune(cfg.tune, tmp, TUNE_MAX);
  else { n = SOUNDS[s].n; memcpy(tmp, SOUNDS[s].notes, sizeof(Note) * n); }
  if (n == 0) { n = SOUNDS[S_ALARM_CLASSIC].n; memcpy(tmp, SOUNDS[S_ALARM_CLASSIC].notes, sizeof(Note) * n); }
  startNotes(tmp, n, true, volumeDuty(loudness));
}

void stopSound() {
  tuneLen = tunePos = 0;
  tuneIsAlarm = false;
  buzzerStop();
}

bool soundBusy() { return tunePos < tuneLen; }

void updateSound(unsigned long now) {
  buzzerUpdate(now);
  if (tunePos >= tuneLen) return;
  if (tuneNext != 0 && (long)(now - tuneNext) < 0) return;
  const Note &n = tuneBuf[tunePos++];
  if (n.f > 0 && tuneDuty > 0) buzzerStart(n.f, n.ms, tuneDuty, now);
  tuneNext = now + n.ms + 12;
  if (tunePos >= tuneLen) tuneIsAlarm = false;
}
