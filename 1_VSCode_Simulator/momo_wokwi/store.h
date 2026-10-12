// =====================================================================
//  store.h  -  Momo's settings, scores and paired phones
//  Saved in the ESP32's flash (Preferences), so they survive power-off.
//  The Momo app changes them; backup/restore uses JSON.
//  The Wi-Fi password is stored here only and is never sent to the app.
// =====================================================================

struct HM { uint8_t h, m; };                       // a time of day

struct AlarmCfg {
  bool on;
  uint8_t h, m;
  uint8_t days;        // bit0 = Sunday ... bit6 = Saturday, 0 = ring once
  uint8_t sound;       // 0 classic, 1 rising, 2 Momo song, 3 your own tune
  uint8_t snooze;      // minutes
  char label[16];
};

struct ReminderCfg {
  bool on;
  uint8_t days;        // which days (bit0 = Sunday)
  uint8_t n;           // how many exact times are used (0..8)
  HM t[8];             // exact times
  bool every;          // also remind every X minutes...
  uint16_t everyMin;
  HM from, to;         // ...between these times
  uint8_t goal;        // daily goal (glasses / stretches)
};

struct Settings {
  uint32_t magic;
  uint16_t ver;
  char name[16];
  int32_t utcOffset;   // seconds (India = 19800)
  bool clock24;
  HM wake, sleep;      // Momo's day
  bool quietOn;
  HM quietFrom, quietTo;
  uint8_t theme;       // 0 cute, 1 robot, 2 soft, 3 retro
  uint8_t volume;      // 0 off, 1 low, 2 medium, 3 high
  bool sndPets, sndReminders, sndNotify, sndGames, chime;
  char tune[100];      // your own tune, e.g. "C5/8 E5/8 G5/4"
  bool selfPlay;       // Momo plays a game by itself when bored
  HM playFrom, playTo;
  uint8_t playGap;     // minutes between self-played games
  uint8_t lively;      // 0 calm, 1 normal, 2 lively
  uint8_t boredMin, napMin;
  uint8_t focusMin, breakMin;
  bool tiltFlip;       // swap tilt left/right for music
  AlarmCfg alarm[8];
  ReminderCfg water, stretch;
  float lat, lon;
  char city[24];
  bool weatherOn, issOn, hugsOn;
};

struct Stats {
  uint32_t magic;
  uint32_t pets;
  uint8_t affection;   // 0..100
  uint16_t best[4];    // jump, flappy, catch, reaction (ms, 0 = none)
  uint16_t dayKey;     // day the counters below belong to
  uint8_t drinks, stretches;
};

struct PairSlot {
  bool used;
  uint8_t hash[32];    // SHA-256 of the phone's secret token
  char device[24];
  uint32_t when;       // minutes since 2024 when it was paired
};

// Your own reminders ("Book ticket" at 16:30). Saved apart from the settings,
// so adding them did not reset anything.
struct NoteCfg {
  bool used, on;
  uint8_t h, m;
  uint8_t days;        // 0 = once on the date below, else bit0 = Sunday ... bit6 = Saturday
  uint16_t year;       // date for a one-time reminder
  uint8_t month, day;
  char text[32];
};
const uint8_t MAX_NOTES = 10;

// Momo 2.2 settings. Saved apart from the others (like the reminders), so
// upgrading from 2.0 / 2.1 keeps every alarm, reminder and score.
struct Extra22 {
  uint32_t magic;
  uint8_t look;        // 0 classic, 1 hero (Momo's own mini-superhero look)
  uint8_t wxEvery;     // weather refresh, minutes (10..180)
  uint8_t wxSource;    // 0 auto (phone first, else internet), 1 phone only, 2 internet only
  bool rub;            // keep a hand on the head = rub (purr) instead of the menu
  bool lowBattAlert;   // warn when the battery is low
  uint8_t spare[11];
};
const uint32_t X22_MAGIC = 0x4D4F2232;   // "MO" + 0x22

const uint32_t CFG_MAGIC = 0x4D4F4D32;   // "MOM2"
const uint16_t CFG_VER = 3;
const uint8_t MAX_PHONES = 3;

Settings cfg;
Stats st;
PairSlot phones[MAX_PHONES];
char wifiSsid[33] = "";
char wifiPass[65] = "";       // private: never printed, never sent to the app
NoteCfg notes[MAX_NOTES];
uint16_t best2[4];            // best scores of the newer games: Simon, Copy the move, Rhythm, Rock paper scissors
Extra22 x22;                  // 2.2: look, weather refresh, rub
bool x22Fresh = false;        // true on the first start after upgrading to 2.2
Preferences prefs;
bool cfgDirty = false, stDirty = false;
unsigned long cfgDirtyAt = 0, stDirtyAt = 0;

void copyText(char *dst, size_t size, const char *src);
void copyClean(char *dst, size_t size, const char *src);

HM hm(uint8_t h, uint8_t m) { HM t; t.h = h; t.m = m; return t; }

void defaultSettings() {
  memset(&cfg, 0, sizeof(cfg));
  cfg.magic = CFG_MAGIC;
  cfg.ver = CFG_VER;
  copyText(cfg.name, sizeof(cfg.name), "Momo");
  cfg.utcOffset = 19800;
  cfg.clock24 = false;
  cfg.wake = hm(7, 0);
  cfg.sleep = hm(22, 30);
  cfg.quietOn = false;
  cfg.quietFrom = hm(13, 0);
  cfg.quietTo = hm(14, 0);
  cfg.theme = 0;
  cfg.volume = 2;
  cfg.sndPets = cfg.sndReminders = cfg.sndNotify = cfg.sndGames = true;
  cfg.chime = false;
  copyText(cfg.tune, sizeof(cfg.tune), "C5/8 E5/8 G5/8 C6/4 R/8 G5/8 C6/2");
  cfg.selfPlay = true;
  cfg.playFrom = hm(10, 0);
  cfg.playTo = hm(20, 0);
  cfg.playGap = 30;
  cfg.lively = 1;
  cfg.boredMin = 5;
  cfg.napMin = 20;
  cfg.focusMin = 25;
  cfg.breakMin = 5;
  cfg.tiltFlip = false;
  for (int i = 0; i < 8; i++) {
    cfg.alarm[i].on = false;
    cfg.alarm[i].h = 7;
    cfg.alarm[i].m = 0;
    cfg.alarm[i].days = 0x3E;          // Monday..Friday
    cfg.alarm[i].sound = 2;
    cfg.alarm[i].snooze = 5;
    copyText(cfg.alarm[i].label, sizeof(cfg.alarm[i].label), "Wake up");
  }
  cfg.water.on = true;
  cfg.water.days = 0x7F;
  cfg.water.n = 5;
  cfg.water.t[0] = hm(10, 0); cfg.water.t[1] = hm(12, 0); cfg.water.t[2] = hm(14, 0);
  cfg.water.t[3] = hm(16, 0); cfg.water.t[4] = hm(18, 0);
  cfg.water.every = false;
  cfg.water.everyMin = 45;
  cfg.water.from = hm(9, 0);
  cfg.water.to = hm(19, 0);
  cfg.water.goal = 8;
  cfg.stretch.on = true;
  cfg.stretch.days = 0x7F;
  cfg.stretch.n = 3;
  cfg.stretch.t[0] = hm(11, 0); cfg.stretch.t[1] = hm(15, 0); cfg.stretch.t[2] = hm(17, 30);
  cfg.stretch.every = false;
  cfg.stretch.everyMin = 60;
  cfg.stretch.from = hm(10, 0);
  cfg.stretch.to = hm(18, 0);
  cfg.stretch.goal = 4;
  cfg.lat = 28.61f;
  cfg.lon = 77.21f;
  copyText(cfg.city, sizeof(cfg.city), "New Delhi");
  cfg.weatherOn = true;
  cfg.issOn = true;
  cfg.hugsOn = true;
}

void defaultExtra() {
  memset(&x22, 0, sizeof(x22));
  x22.magic = X22_MAGIC;
  x22.look = 1;          // the hero look
  x22.wxEvery = 30;
  x22.wxSource = 0;
  x22.rub = true;
  x22.lowBattAlert = true;
}

void defaultStats() {
  memset(&st, 0, sizeof(st));
  st.magic = CFG_MAGIC;
  st.affection = 50;
}

void loadStore() {
  prefs.begin("momo2", false);
  bool ok = prefs.getBytesLength("cfg") == sizeof(Settings) && prefs.getBytes("cfg", &cfg, sizeof(Settings)) == sizeof(Settings) &&
            cfg.magic == CFG_MAGIC && cfg.ver == CFG_VER;
  if (!ok) defaultSettings();
  ok = prefs.getBytesLength("stats") == sizeof(Stats) && prefs.getBytes("stats", &st, sizeof(Stats)) == sizeof(Stats) && st.magic == CFG_MAGIC;
  if (!ok) defaultStats();
  if (prefs.getBytesLength("phones") == sizeof(phones)) prefs.getBytes("phones", phones, sizeof(phones));
  else memset(phones, 0, sizeof(phones));
  memset(wifiSsid, 0, sizeof(wifiSsid));
  memset(wifiPass, 0, sizeof(wifiPass));
  prefs.getString("ssid", wifiSsid, sizeof(wifiSsid));
  prefs.getString("pass", wifiPass, sizeof(wifiPass));
  if (prefs.getBytesLength("notes") == sizeof(notes)) prefs.getBytes("notes", notes, sizeof(notes));
  else memset(notes, 0, sizeof(notes));
  if (prefs.getBytesLength("best2") == sizeof(best2)) prefs.getBytes("best2", best2, sizeof(best2));
  else memset(best2, 0, sizeof(best2));
  bool okx = prefs.getBytesLength("x22") == sizeof(x22) && prefs.getBytes("x22", &x22, sizeof(x22)) == sizeof(x22) &&
             x22.magic == X22_MAGIC;
  if (!okx) {
    defaultExtra();
    cfg.theme = 4;       // first start of 2.2: hero sounds go with the hero look
    x22Fresh = true;
  }
  if (x22.wxEvery < 10 || x22.wxEvery > 180) x22.wxEvery = 30;
  if (x22.wxSource > 2) x22.wxSource = 0;
  if (x22.look > 1) x22.look = 1;
}

void saveExtra() { prefs.putBytes("x22", &x22, sizeof(x22)); }

// Best score of any game (the first four live in Stats, the newer four in best2)
uint16_t &bestScore(uint8_t game) { return game < 4 ? st.best[game] : best2[(game - 4) & 3]; }

void markCfg(unsigned long now) { cfgDirty = true; cfgDirtyAt = now; }
void markStats(unsigned long now) { stDirty = true; stDirtyAt = now; }

void savePhones() { prefs.putBytes("phones", phones, sizeof(phones)); }
void saveWifi() { prefs.putString("ssid", wifiSsid); prefs.putString("pass", wifiPass); }
void saveNotes() { prefs.putBytes("notes", notes, sizeof(notes)); }

// Write changes to flash, at most every few seconds (protects the flash)
void saveStore(unsigned long now, bool force) {
  if (cfgDirty && (force || now - cfgDirtyAt > 2000)) {
    prefs.putBytes("cfg", &cfg, sizeof(Settings));
    saveExtra();
    cfgDirty = false;
  }
  if (stDirty && (force || now - stDirtyAt > 10000)) {
    prefs.putBytes("stats", &st, sizeof(Stats));
    prefs.putBytes("best2", best2, sizeof(best2));
    stDirty = false;
  }
}

// ---------------- JSON (for the app, backup and restore) ----------------

void hmText(HM t, char *out) { snprintf(out, 6, "%02u:%02u", (unsigned)(t.h % 24), (unsigned)(t.m % 60)); }

bool parseHM(const char *s, HM &out) {
  if (s == nullptr) return false;
  int h = -1, m = -1;
  if (sscanf(s, "%d:%d", &h, &m) != 2) return false;
  if (h < 0 || h > 23 || m < 0 || m > 59) return false;
  out.h = (uint8_t)h;
  out.m = (uint8_t)m;
  return true;
}

void putHM(JsonObject o, const char *key, HM t) { char b[6]; hmText(t, b); o[key] = b; }

void getHM(JsonObjectConst o, const char *key, HM &t) {
  if (o[key].is<const char *>()) parseHM(o[key].as<const char *>(), t);
}

template <typename T> void getNum(JsonObjectConst o, const char *key, T &v, long lo, long hi) {
  if (!o[key].is<long>() && !o[key].is<double>()) return;
  long x = o[key].as<long>();
  if (x < lo) x = lo;
  if (x > hi) x = hi;
  v = (T)x;
}

void getBool(JsonObjectConst o, const char *key, bool &v) {
  if (o[key].is<bool>()) v = o[key].as<bool>();
}

void getText(JsonObjectConst o, const char *key, char *dst, size_t size) {
  if (o[key].is<const char *>()) copyText(dst, size, o[key].as<const char *>());
}

void reminderToJson(JsonObject o, const ReminderCfg &r) {
  o["on"] = r.on;
  o["days"] = r.days;
  JsonArray t = o["times"].to<JsonArray>();
  for (uint8_t i = 0; i < r.n && i < 8; i++) { char b[6]; hmText(r.t[i], b); t.add(b); }
  o["every"] = r.every;
  o["everyMin"] = r.everyMin;
  putHM(o, "from", r.from);
  putHM(o, "to", r.to);
  o["goal"] = r.goal;
}

void reminderFromJson(JsonObjectConst o, ReminderCfg &r) {
  getBool(o, "on", r.on);
  getNum(o, "days", r.days, 0, 127);
  if (o["times"].is<JsonArrayConst>()) {
    HM tmp[8];
    uint8_t n = 0;
    for (JsonVariantConst v : o["times"].as<JsonArrayConst>()) {
      if (n >= 8) break;
      HM t;
      if (v.is<const char *>() && parseHM(v.as<const char *>(), t)) tmp[n++] = t;
    }
    // keep them sorted, so the app and Momo agree on the order
    for (uint8_t i = 1; i < n; i++)
      for (uint8_t j = i; j > 0 && tmp[j].h * 60 + tmp[j].m < tmp[j - 1].h * 60 + tmp[j - 1].m; j--) {
        HM x = tmp[j]; tmp[j] = tmp[j - 1]; tmp[j - 1] = x;
      }
    r.n = n;
    for (uint8_t i = 0; i < n; i++) r.t[i] = tmp[i];
  }
  getBool(o, "every", r.every);
  getNum(o, "everyMin", r.everyMin, 10, 600);
  getHM(o, "from", r.from);
  getHM(o, "to", r.to);
  getNum(o, "goal", r.goal, 1, 30);
}

void settingsToJson(JsonObject o) {
  o["name"] = cfg.name;
  o["utcOffsetMin"] = cfg.utcOffset / 60;
  o["clock24"] = cfg.clock24;
  putHM(o, "wake", cfg.wake);
  putHM(o, "sleep", cfg.sleep);
  JsonObject q = o["quiet"].to<JsonObject>();
  q["on"] = cfg.quietOn;
  putHM(q, "from", cfg.quietFrom);
  putHM(q, "to", cfg.quietTo);
  o["look"] = x22.look;
  o["rub"] = x22.rub;
  o["lowBattAlert"] = x22.lowBattAlert;
  JsonObject s = o["sound"].to<JsonObject>();
  s["theme"] = cfg.theme;
  s["volume"] = cfg.volume;
  s["pets"] = cfg.sndPets;
  s["reminders"] = cfg.sndReminders;
  s["notify"] = cfg.sndNotify;
  s["games"] = cfg.sndGames;
  s["chime"] = cfg.chime;
  s["tune"] = cfg.tune;
  JsonObject p = o["play"].to<JsonObject>();
  p["self"] = cfg.selfPlay;
  putHM(p, "from", cfg.playFrom);
  putHM(p, "to", cfg.playTo);
  p["gap"] = cfg.playGap;
  p["lively"] = cfg.lively;
  p["boredMin"] = cfg.boredMin;
  p["napMin"] = cfg.napMin;
  p["focusMin"] = cfg.focusMin;
  p["breakMin"] = cfg.breakMin;
  p["tiltFlip"] = cfg.tiltFlip;
  JsonArray a = o["alarms"].to<JsonArray>();
  for (int i = 0; i < 8; i++) {
    JsonObject x = a.add<JsonObject>();
    x["on"] = cfg.alarm[i].on;
    char b[6];
    hmText(hm(cfg.alarm[i].h, cfg.alarm[i].m), b);
    x["time"] = b;
    x["days"] = cfg.alarm[i].days;
    x["sound"] = cfg.alarm[i].sound;
    x["snooze"] = cfg.alarm[i].snooze;
    x["label"] = cfg.alarm[i].label;
  }
  reminderToJson(o["water"].to<JsonObject>(), cfg.water);
  reminderToJson(o["stretch"].to<JsonObject>(), cfg.stretch);
  JsonObject w = o["weather"].to<JsonObject>();
  w["on"] = cfg.weatherOn;
  w["city"] = cfg.city;
  w["lat"] = cfg.lat;
  w["lon"] = cfg.lon;
  w["every"] = x22.wxEvery;
  w["source"] = x22.wxSource;
  o["iss"] = cfg.issOn;
  o["hugs"] = cfg.hugsOn;
  JsonArray nl = o["notes"].to<JsonArray>();
  for (int i = 0; i < MAX_NOTES; i++) {
    if (!notes[i].used) continue;
    JsonObject x = nl.add<JsonObject>();
    x["on"] = notes[i].on;
    x["text"] = notes[i].text;
    char b[20];
    hmText(hm(notes[i].h, notes[i].m), b);
    x["time"] = b;
    x["days"] = notes[i].days;
    snprintf(b, sizeof(b), "%04u-%02u-%02u", (unsigned)(notes[i].year % 10000), (unsigned)(notes[i].month % 100), (unsigned)(notes[i].day % 100));
    x["date"] = b;
  }
}

// Your own reminders from the app: the list replaces the old one (max 10)
void notesFromJson(JsonArrayConst a) {
  NoteCfg tmp[MAX_NOTES];
  memset(tmp, 0, sizeof(tmp));
  uint8_t n = 0;
  for (JsonVariantConst v : a) {
    if (n >= MAX_NOTES) break;
    if (!v.is<JsonObjectConst>()) continue;
    JsonObjectConst x = v.as<JsonObjectConst>();
    NoteCfg &c = tmp[n];
    HM t;
    if (!x["time"].is<const char *>() || !parseHM(x["time"].as<const char *>(), t)) continue;
    c.used = true;
    c.h = t.h;
    c.m = t.m;
    c.on = x["on"] | true;
    getNum(x, "days", c.days, 0, 127);
    copyClean(c.text, sizeof(c.text), x["text"] | "");
    if (c.text[0] == '\0') copyText(c.text, sizeof(c.text), "Reminder");
    int y = 0, mo = 0, d = 0;
    const char *ds = x["date"] | "";
    if (sscanf(ds, "%d-%d-%d", &y, &mo, &d) == 3 && y >= 2024 && y <= 2099 && mo >= 1 && mo <= 12 && d >= 1 && d <= 31) {
      c.year = (uint16_t)y; c.month = (uint8_t)mo; c.day = (uint8_t)d;
    } else if (c.days == 0) {
      c.on = false;   // a one-time reminder needs a date
    }
    n++;
  }
  memcpy(notes, tmp, sizeof(notes));
  saveNotes();
}

// Applies only the keys that are present, with every value checked
void settingsFromJson(JsonObjectConst o) {
  getText(o, "name", cfg.name, sizeof(cfg.name));
  if (cfg.name[0] == '\0') copyText(cfg.name, sizeof(cfg.name), "Momo");
  if (o["utcOffsetMin"].is<long>()) {
    long v = o["utcOffsetMin"].as<long>();
    if (v >= -720 && v <= 840) cfg.utcOffset = (int32_t)(v * 60);
  }
  getBool(o, "clock24", cfg.clock24);
  getNum(o, "look", x22.look, 0, 1);
  getBool(o, "rub", x22.rub);
  getBool(o, "lowBattAlert", x22.lowBattAlert);
  getHM(o, "wake", cfg.wake);
  getHM(o, "sleep", cfg.sleep);
  if (o["quiet"].is<JsonObjectConst>()) {
    JsonObjectConst q = o["quiet"].as<JsonObjectConst>();
    getBool(q, "on", cfg.quietOn);
    getHM(q, "from", cfg.quietFrom);
    getHM(q, "to", cfg.quietTo);
  }
  if (o["sound"].is<JsonObjectConst>()) {
    JsonObjectConst s = o["sound"].as<JsonObjectConst>();
    getNum(s, "theme", cfg.theme, 0, 4);
    getNum(s, "volume", cfg.volume, 0, 3);
    getBool(s, "pets", cfg.sndPets);
    getBool(s, "reminders", cfg.sndReminders);
    getBool(s, "notify", cfg.sndNotify);
    getBool(s, "games", cfg.sndGames);
    getBool(s, "chime", cfg.chime);
    getText(s, "tune", cfg.tune, sizeof(cfg.tune));
  }
  if (o["play"].is<JsonObjectConst>()) {
    JsonObjectConst p = o["play"].as<JsonObjectConst>();
    getBool(p, "self", cfg.selfPlay);
    getHM(p, "from", cfg.playFrom);
    getHM(p, "to", cfg.playTo);
    getNum(p, "gap", cfg.playGap, 5, 240);
    getNum(p, "lively", cfg.lively, 0, 2);
    getNum(p, "boredMin", cfg.boredMin, 1, 60);
    getNum(p, "napMin", cfg.napMin, 2, 120);
    getNum(p, "focusMin", cfg.focusMin, 5, 120);
    getNum(p, "breakMin", cfg.breakMin, 1, 60);
    getBool(p, "tiltFlip", cfg.tiltFlip);
  }
  if (o["alarms"].is<JsonArrayConst>()) {
    int i = 0;
    for (JsonVariantConst v : o["alarms"].as<JsonArrayConst>()) {
      if (i >= 8) break;
      if (v.is<JsonObjectConst>()) {
        JsonObjectConst x = v.as<JsonObjectConst>();
        AlarmCfg &al = cfg.alarm[i];
        getBool(x, "on", al.on);
        HM t = hm(al.h, al.m);
        getHM(x, "time", t);
        al.h = t.h;
        al.m = t.m;
        getNum(x, "days", al.days, 0, 127);
        getNum(x, "sound", al.sound, 0, 3);
        getNum(x, "snooze", al.snooze, 1, 30);
        getText(x, "label", al.label, sizeof(al.label));
      }
      i++;
    }
  }
  if (o["water"].is<JsonObjectConst>()) reminderFromJson(o["water"].as<JsonObjectConst>(), cfg.water);
  if (o["stretch"].is<JsonObjectConst>()) reminderFromJson(o["stretch"].as<JsonObjectConst>(), cfg.stretch);
  if (o["weather"].is<JsonObjectConst>()) {
    JsonObjectConst w = o["weather"].as<JsonObjectConst>();
    getBool(w, "on", cfg.weatherOn);
    getText(w, "city", cfg.city, sizeof(cfg.city));
    if (w["lat"].is<double>() || w["lat"].is<long>()) {
      double v = w["lat"].as<double>();
      if (v >= -90 && v <= 90) cfg.lat = (float)v;
    }
    if (w["lon"].is<double>() || w["lon"].is<long>()) {
      double v = w["lon"].as<double>();
      if (v >= -180 && v <= 180) cfg.lon = (float)v;
    }
    getNum(w, "every", x22.wxEvery, 10, 180);
    getNum(w, "source", x22.wxSource, 0, 2);
  }
  getBool(o, "iss", cfg.issOn);
  getBool(o, "hugs", cfg.hugsOn);
  if (o["notes"].is<JsonArrayConst>()) notesFromJson(o["notes"].as<JsonArrayConst>());
}

void scoresToJson(JsonObject o) {
  o["jump"] = st.best[0];
  o["flappy"] = st.best[1];
  o["catch"] = st.best[2];
  o["reactionMs"] = st.best[3];
  o["simon"] = best2[0];
  o["copy"] = best2[1];
  o["rhythm"] = best2[2];
  o["rps"] = best2[3];
  o["pets"] = st.pets;
  o["affection"] = st.affection;
}

void scoresFromJson(JsonObjectConst o) {
  getNum(o, "jump", st.best[0], 0, 65535);
  getNum(o, "flappy", st.best[1], 0, 65535);
  getNum(o, "catch", st.best[2], 0, 65535);
  getNum(o, "reactionMs", st.best[3], 0, 65535);
  getNum(o, "simon", best2[0], 0, 65535);
  getNum(o, "copy", best2[1], 0, 65535);
  getNum(o, "rhythm", best2[2], 0, 65535);
  getNum(o, "rps", best2[3], 0, 65535);
  getNum(o, "pets", st.pets, 0, 100000000L);
  getNum(o, "affection", st.affection, 0, 100);
}
