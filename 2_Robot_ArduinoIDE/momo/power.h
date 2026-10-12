// =====================================================================
//  power.h  -  Momo 2.2: battery with USB-C charging, the back button
//              and sleep (soft off)
//
//  Battery: 3.7 V LiPo (500 mAh) charged by a TP4056 USB-C board.
//    Battery +  -> 100k -> GPIO 4 -> 100k -> GND   (Momo reads half the voltage)
//    USB-C 5 V of the charger board (IN+) -> 100k -> GPIO 7 -> 150k -> GND
//  Back button: GPIO 5 -> button -> GND (+ 10k from GPIO 5 to 3.3 V).
//    It does everything the head does (tap / hold / double-tap = back),
//    and holding it 3 seconds puts Momo to sleep. Press it to wake Momo.
//  With no battery wired, Momo simply hides the battery icon.
// =====================================================================

#include <esp_sleep.h>

const int PIN_BUTTON = 5;    // back button to GND, wakes Momo from sleep
const int PIN_BATT   = 4;    // battery voltage through a 100k / 100k divider
const int PIN_USB    = 7;    // charger's USB-C 5 V through 100k / 150k: HIGH = plugged in

bool battPresent = false, charging = false, battFull = false;
uint16_t battMv = 0;         // smoothed battery voltage (mV)
uint8_t battPct = 0;
float battMvF = 0;
unsigned long nextBattRead = 0;
bool lowWarned = false;
uint8_t criticalCount = 0;
bool asleep = false;         // only used by the desktop test rig (the real chip never comes back)
bool wokeByButton = false, wokeForAlarm = false;
bool btnArmed = true;        // a button that is already down at start counts only after it is let go

void softOff(unsigned long now, uint8_t reason);

// LiPo charge from voltage (resting, small load)
uint8_t lipoPercent(uint16_t mv) {
  static const uint16_t V[] = {3300, 3500, 3610, 3690, 3710, 3730, 3750, 3770, 3790, 3800, 3820, 3840, 3850, 3870, 3910, 3950, 3980, 4020, 4080, 4110, 4150, 4200};
  static const uint8_t P[] = {0, 3, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75, 80, 85, 90, 95, 100};
  const int n = sizeof(V) / sizeof(V[0]);
  if (mv <= V[0]) return 0;
  if (mv >= V[n - 1]) return 100;
  for (int i = 1; i < n; i++)
    if (mv <= V[i]) return (uint8_t)(P[i - 1] + (long)(P[i] - P[i - 1]) * (mv - V[i - 1]) / (V[i] - V[i - 1]));
  return 100;
}

#ifndef WOKWI_BATTERY_KNOB
#define WOKWI_BATTERY_KNOB 1   // 0 = simulator without the battery knob (the backup simulator)
#endif

uint16_t readBattMv() {
#if WOKWI
#if !WOKWI_BATTERY_KNOB
  return 0;                    // backup simulator: no battery
#endif
  // simulator: the knob on GPIO 4 is the battery (turned right = 4.2 V full, near the left =
  // 3.3 V empty, all the way left = no battery)
  uint32_t raw = analogReadMilliVolts(PIN_BATT);
  if (raw < 20) return 0;
  return (uint16_t)(3300 + min(3000UL, (unsigned long)raw) * 900 / 3000);
#else
  return (uint16_t)(analogReadMilliVolts(PIN_BATT) * 2);   // 100k / 100k divider
#endif
}

void startPower() {
  pinMode(PIN_BUTTON, INPUT_PULLUP);
  pinMode(PIN_USB, INPUT_PULLDOWN);
  esp_sleep_wakeup_cause_t wc = esp_sleep_get_wakeup_cause();
  wokeByButton = (wc == ESP_SLEEP_WAKEUP_GPIO);
  wokeForAlarm = (wc == ESP_SLEEP_WAKEUP_TIMER);
  // The finger that woke Momo (or a stuck / shorted button) must not count as a press:
  // the button works again once it has been let go.
  btnArmed = digitalRead(PIN_BUTTON) == HIGH;
  if (!btnArmed && !wokeByButton) Serial.println("Back button is down at start: it works again after you let go");
  if (wokeByButton) Serial.println("Woke up: back button");
  if (wokeForAlarm) Serial.println("Woke up for an alarm");
}

void drawChargeScreen(unsigned long now);

// Back button state for the gesture engine (LOW = pressed)
bool buttonDown() {
  bool down = digitalRead(PIN_BUTTON) == LOW;
  if (!btnArmed) {
    if (!down) btnArmed = true;
    return false;
  }
  return down;
}

void readBattery(unsigned long now) {
  if ((long)(now - nextBattRead) < 0) return;
  nextBattRead = now + 1000;
  uint16_t mv = readBattMv();
  bool present = mv > 2500;
  if (present && !battPresent) battMvF = mv;
  battPresent = present;
  if (!present) { charging = battFull = false; battMv = 0; battPct = 0; return; }
  battMvF += (mv - battMvF) * 0.25f;
  battMv = (uint16_t)lroundf(battMvF);
  battPct = lipoPercent(battMv);
  bool usb = digitalRead(PIN_USB) == HIGH;
  if (usb != charging) {
    charging = usb;
    if (usb) {
      Serial.printf("Charging started (battery %u%%)\n", (unsigned)battPct);
      lowWarned = false;
      criticalCount = 0;
      if (screen == SC_FACE || screen == SC_CLOCK || screen == SC_WEATHER || screen == SC_HELLO) {
        goScreen(SC_CHARGE, now);
        if (life != L_AWAKE && life != L_NIGHT) setLife(L_AWAKE, now);
        lastTouch = now;
      }
      PLAY_NOTES(HS_CHARGE, C_UI);
    } else {
      Serial.printf("Charger unplugged (battery %u%%)\n", (unsigned)battPct);
      if (screen == SC_CHARGE) backToFace(now, F_NEUTRAL);
    }
  }
  bool full = charging && battMv >= 4150;
  if (full && !battFull) Serial.println("Battery full");
  battFull = full;
  if (charging || battPct >= 25) lowWarned = false;
  if (!charging && battPct <= 15 && !lowWarned && x22.lowBattAlert) {
    lowWarned = true;
    Serial.printf("Battery low: %u%%\n", (unsigned)battPct);
    showAlert(AL_BATTERY, "Battery low", "Please charge me with the USB-C cable.", now);
    showFace(F_SLEEPY, 3000, now);
  }
  // nearly empty: go to sleep before the battery protection cuts the power
  if (!charging && battMv < 3350) {
    if (++criticalCount >= 10) softOff(now, 2);
  } else {
    criticalCount = 0;
  }
}

// Battery icon for the status bar (13 x 7)
void drawBatteryIcon(int x, int y, unsigned long now) {
  display.drawRect(x, y, 11, 7, INK);
  display.fillRect(x + 11, y + 2, 2, 3, INK);
  int w = (battPct * 9 + 50) / 100;
  if (charging && !battFull) w = (int)((now / 300) % 10);    // filling up while charging
  if (w > 0) display.fillRect(x + 1, y + 1, min(9, w), 5, INK);
  if (charging) {                                           // little lightning bolt
    display.drawLine(x + 6, y + 1, x + 4, y + 3, battFull ? ERASE : ERASE);
    display.drawLine(x + 4, y + 3, x + 7, y + 3, ERASE);
    display.drawLine(x + 7, y + 3, x + 5, y + 5, ERASE);
  }
}

// Minutes until the next alarm or your own reminder (-1 = none in the next day)
long minutesToNextRing() {
  int m = nowMinutes();
  if (m < 0) return -1;
  long best = -1;
  for (int i = 0; i < 8; i++) {
    const AlarmCfg &a = cfg.alarm[i];
    if (!a.on) continue;
    for (int d = 0; d < 2; d++) {
      long at = d * 1440L + a.h * 60 + a.m;
      if (at <= m) continue;
      if (a.days != 0 && !(a.days & (1 << ((nowTm.tm_wday + d) % 7)))) continue;
      if (best < 0 || at - m < best) best = at - m;
      break;
    }
  }
  for (int i = 0; i < MAX_NOTES; i++) {
    const NoteCfg &n = notes[i];
    if (!n.used || !n.on) continue;
    for (int d = 0; d < 2; d++) {
      long at = d * 1440L + n.h * 60 + n.m;
      if (at <= m) continue;
      bool ok;
      if (n.days != 0) ok = (n.days & (1 << ((nowTm.tm_wday + d) % 7))) != 0;
      else {
        struct tm x = nowTm;
        time_t e = (time_t)(epochOf(x) + d * 86400L);
        gmtime_r(&e, &x);
        ok = n.year == x.tm_year + 1900 && n.month == x.tm_mon + 1 && n.day == x.tm_mday;
      }
      if (!ok) continue;
      if (best < 0 || at - m < best) best = at - m;
      break;
    }
  }
  return best;
}

// Goodbye animation: Momo waves, the eyes close and the screen goes dark
void drawSleepScreen(unsigned long t, uint8_t reason) {
  display.clearDisplay();
  float k = min(1.0f, t / 900.0f);
  int lid = (int)(k * 12);
  for (int side = -1; side <= 1; side += 2) {
    int x = 64 + side * 22 - 9, y = 14;
    display.fillRoundRect(x, y, 18, 22, 6, INK);
    display.fillRect(x - 1, y - 1, 20, 1 + lid * 2, ERASE);
  }
  display.setTextSize(1);
  display.setTextColor(INK);
  const char *msg = reason == 2 ? "Battery empty - charge me!" : "Good night! Press the";
  int w = (int)strlen(msg) * 6;
  display.setCursor((128 - w) / 2, 44);
  display.print(msg);
  if (reason != 2) {
    const char *m2 = "back button to wake me";
    display.setCursor((128 - (int)strlen(m2) * 6) / 2, 54);
    display.print(m2);
  }
}

void screenOff() {
#if SCREEN_IS_SSD1306
  display.ssd1306_command(SSD1306_DISPLAYOFF);
#else
  display.oled_command(SH110X_DISPLAYOFF);
#endif
}

// Sleep: everything off, the back button (or the next alarm) wakes Momo up.
// reason 1 = back button held, 2 = battery empty
void softOff(unsigned long now, uint8_t reason) {
  Serial.printf("Going to sleep (%s)\n", reason == 2 ? "battery empty" : "back button");
  stopSound();
  if (reason != 2) PLAY_NOTES(HS_SLEEP, C_UI);
  saveStore(now, true);
  unsigned long t0 = millis();
  while (millis() - t0 < 1400) {                  // short goodbye
    unsigned long t = millis();
    updateSound(t);
    drawSleepScreen(t - t0, reason);
    display.display();
    delay(40);
  }
  stopSound();
  display.clearDisplay();
  display.display();
  screenOff();
  if (mpuOk) i2cWriteReg(MPU_ADDR, 0x6B, 0x40);   // motion sensor sleeps too
  unsigned long w0 = millis();                    // let go of the button first, or it wakes Momo at once
  while (digitalRead(PIN_BUTTON) == LOW && millis() - w0 < 8000) delay(10);
  delay(50);
  long mins = reason == 2 ? -1 : minutesToNextRing();
  long secs = -1;
  if (mins > 0 && mins <= 1440) {                 // wake up just before the next alarm / reminder
    secs = mins * 60L - nowTm.tm_sec - 20;
    if (secs < 5) secs = 5;
    Serial.printf("I'll wake up for the alarm in %ld minutes\n", mins);
  }
#if WOKWI
  // Simulator: Momo waits here with the screen off and starts again when you press the
  // back button (B key) or the alarm time comes - the same as the real robot waking up.
  Serial.println("Sleeping. Press the back button (B key) to wake Momo.");
  Serial.flush();
  unsigned long sleptAt = millis();
  while (digitalRead(PIN_BUTTON) == HIGH) {
    if (secs > 0 && millis() - sleptAt >= (unsigned long)secs * 1000UL) break;
    delay(20);
  }
  ESP.restart();
#else
  esp_deep_sleep_enable_gpio_wakeup(1ULL << PIN_BUTTON, ESP_GPIO_WAKEUP_GPIO_LOW);
  if (secs > 0) esp_sleep_enable_timer_wakeup((uint64_t)secs * 1000000ULL);
  Serial.flush();
  esp_deep_sleep_start();
#endif
  asleep = true;                                  // never reached on the real chip
}
