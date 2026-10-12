// =====================================================================
//  schedule.h  -  time of day, quiet hours, alarms and reminders
// =====================================================================

enum DayPart : uint8_t { DP_UNKNOWN, DP_MORNING, DP_DAY, DP_EVENING, DP_NIGHT };

struct tm nowTm;
bool haveTime = false;
unsigned long nextTimeRead = 0;
int lastCheckedMinute = -1;
uint16_t alarmFiredMin[16];         // minute-of-day+1 when each alarm last rang (0 = never)
uint16_t remindFiredMin[2] = {0, 0};
unsigned long remindSnoozeUntil[2] = {0, 0};
bool remindPending[2] = {false, false};
uint16_t wokeEarlyDay = 0xFFFF;    // alarm stopped before wake time on this day

int minutesOf(HM t) { return t.h * 60 + t.m; }
int nowMinutes() { return haveTime ? nowTm.tm_hour * 60 + nowTm.tm_min : -1; }
uint16_t dayKey() { return haveTime ? (uint16_t)((nowTm.tm_year % 50) * 366 + nowTm.tm_yday) : 0; }

// Is minute m inside [from, to)? Works across midnight.
bool inWindow(int m, HM from, HM to) {
  if (m < 0) return false;
  int a = minutesOf(from), b = minutesOf(to);
  if (a == b) return false;
  if (a < b) return m >= a && m < b;
  return m >= a || m < b;
}

bool dayAllowed(uint8_t days) { return haveTime && (days & (1 << nowTm.tm_wday)) != 0; }

bool isNight() {
  int m = nowMinutes();
  if (m < 0) return false;
  if (!inWindow(m, cfg.sleep, cfg.wake)) return false;
  // woke up early for an alarm: stay awake until the normal wake time
  if (wokeEarlyDay == dayKey() && m < minutesOf(cfg.wake)) return false;
  return true;
}

bool isQuiet() { return cfg.quietOn && inWindow(nowMinutes(), cfg.quietFrom, cfg.quietTo); }

DayPart dayPart() {
  int m = nowMinutes();
  if (m < 0) return DP_UNKNOWN;
  if (isNight()) return DP_NIGHT;
  int wake = minutesOf(cfg.wake), sleep = minutesOf(cfg.sleep);
  int sinceWake = (m - wake + 1440) % 1440;
  int toSleep = (sleep - m + 1440) % 1440;
  if (sinceWake < 60) return DP_MORNING;
  if (toSleep <= 60) return DP_EVENING;
  return DP_DAY;
}

bool isPlayTime() {
  int m = nowMinutes();
  if (m < 0) return true;           // time unknown: allow
  return inWindow(m, cfg.playFrom, cfg.playTo) && !isNight();
}

// Sounds are muted at night and in quiet hours (alarms always ring)
bool soundBlocked(SndCat cat) {
  if (cat == C_ALARM || nightSoundOk) return false;
  if (isNight() || isQuiet()) return true;
  return false;
}

const char *DAY_SHORT[7] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
const char *MONTH_SHORT[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

// "7:05 pm" or "19:05"
void timeText(int h, int m, char *out, size_t size, bool withAmPm) {
  if (cfg.clock24) { snprintf(out, size, "%02d:%02d", h, m); return; }
  int h12 = h % 12;
  if (h12 == 0) h12 = 12;
  if (withAmPm) snprintf(out, size, "%d:%02d %s", h12, m, h < 12 ? "am" : "pm");
  else snprintf(out, size, "%d:%02d", h12, m);
}

// ---------- which alarm/reminder is due this minute ----------

// Momo's own alarms are 0..7, the Chronos app's alarms are 8..15
bool alarmDue(int idx, int m) {
  if (m < 0) return false;
  bool on;
  uint8_t h, mi, days;
  if (idx < 8) {
    on = cfg.alarm[idx].on; h = cfg.alarm[idx].h; mi = cfg.alarm[idx].m; days = cfg.alarm[idx].days;
  } else {
#if WOKWI
    return false;
#else
    Alarm &a = watch.getAlarm(idx - 8);
    on = a.enabled; h = a.hour; mi = a.minute;
    // Chronos: bits 0-5 = Mon..Sat, bit 6 = Sun, 0x80 = every day -> Momo: bit0 = Sun
    days = (a.repeat == 0x80 || a.repeat == 0x7F) ? 0x7F : (uint8_t)(((a.repeat & 0x3F) << 1) | ((a.repeat >> 6) & 1));
#endif
  }
  if (!on || h * 60 + mi != m) return false;
  if (days != 0 && !dayAllowed(days)) return false;
  return true;
}

// Is a reminder due now (exact times or the repeat interval)?
bool reminderDue(const ReminderCfg &r, int m) {
  if (!r.on || m < 0 || !dayAllowed(r.days)) return false;
  for (uint8_t i = 0; i < r.n && i < 8; i++)
    if (minutesOf(r.t[i]) == m) return true;
  if (r.every && r.everyMin >= 10 && inWindow(m, r.from, r.to)) {
    int since = (m - minutesOf(r.from) + 1440) % 1440;
    if (since > 0 && since % r.everyMin == 0) return true;
  }
  return false;
}

// Is your own reminder number i due at minute m today?
bool noteDue(int i, int m) {
  const NoteCfg &n = notes[i];
  if (!n.used || !n.on || m < 0 || n.h * 60 + n.m != m) return false;
  if (n.days != 0) return dayAllowed(n.days);
  return n.year == nowTm.tm_year + 1900 && n.month == nowTm.tm_mon + 1 && n.day == nowTm.tm_mday;
}

// The next of your own reminders, e.g. "4:30 pm Book ticket" / "Tmrw 9:00 am Call bank"
bool nextNoteText(char *out, size_t size) {
  int m = nowMinutes();
  if (m < 0) return false;
  struct tm today = nowTm;
  today.tm_hour = 0; today.tm_min = 0; today.tm_sec = 0;
  int64_t day0 = epochOf(today) / 86400;
  long best = -1;
  int bi = -1;
  for (int i = 0; i < MAX_NOTES; i++) {
    const NoteCfg &n = notes[i];
    if (!n.used || !n.on) continue;
    long at = -1;
    if (n.days == 0) {
      struct tm d = {};
      d.tm_year = n.year - 1900; d.tm_mon = n.month - 1; d.tm_mday = n.day;
      long dd = (long)(epochOf(d) / 86400 - day0);
      at = dd * 1440 + n.h * 60 + n.m;
      if (at <= m) at = -1;
    } else {
      for (int k = 0; k < 8; k++) {
        long a = k * 1440L + n.h * 60 + n.m;
        if (a <= m || !(n.days & (1 << ((nowTm.tm_wday + k) % 7)))) continue;
        at = a;
        break;
      }
    }
    if (at >= 0 && (best < 0 || at < best)) { best = at; bi = i; }
  }
  if (bi < 0) return false;
  char t[12];
  timeText((int)((best % 1440) / 60), (int)(best % 60), t, sizeof(t), true);
  long dd = best / 1440;
  if (dd == 0) snprintf(out, size, "%s %s", t, notes[bi].text);
  else if (dd == 1) snprintf(out, size, "Tmrw %s %s", t, notes[bi].text);
  else {
    time_t e = (time_t)((day0 + dd) * 86400);
    struct tm x;
    gmtime_r(&e, &x);
    snprintf(out, size, "%s %d %s %s %s", DAY_SHORT[x.tm_wday], x.tm_mday, MONTH_SHORT[x.tm_mon], t, notes[bi].text);
  }
  return true;
}

// Next alarm today/tomorrow as text, for the clock screen. Returns false if none.
bool nextAlarmText(char *out, size_t size) {
  int m = nowMinutes();
  if (m < 0) return false;
  int best = 100000;
  for (int i = 0; i < 8; i++) {
    const AlarmCfg &a = cfg.alarm[i];
    if (!a.on) continue;
    for (int d = 0; d < 8; d++) {
      int wd = (nowTm.tm_wday + d) % 7;
      int at = d * 1440 + a.h * 60 + a.m;
      if (at <= m) continue;
      if (a.days != 0 && !(a.days & (1 << wd))) continue;
      if (at < best) best = at;
      break;
    }
  }
  if (best == 100000) return false;
  char t[12];
  timeText((best % 1440) / 60, best % 60, t, sizeof(t), true);
  snprintf(out, size, "%s%s", best >= 1440 ? "Tmrw " : "", t);
  return true;
}
