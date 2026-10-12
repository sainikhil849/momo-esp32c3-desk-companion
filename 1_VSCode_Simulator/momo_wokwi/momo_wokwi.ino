/*
  =====================================================================
   MOMO 2  -  desk companion robot
  =====================================================================
   Board   : ESP32-C3 Super Mini
   Screen  : 1.3" OLED 128x64, SH1106 chip, I2C        (address 0x3C)
   Touch   : TTP223 touch sensor on the head            (pin 3)
   Button  : back button on the back, to GND            (pin 5)   - new in 2.2
   Battery : 500 mAh LiPo + TP4056 USB-C charger        (pins 4, 7) - new in 2.2
   Motion  : MPU6050, AD0 pin tied to 3.3V              (address 0x69)
   Clock   : DS3231 real-time clock module              (address 0x68)
   Sound   : passive buzzer                             (pin 10)
   Phone   : Android with the free "Chronos" app (notifications, calls,
             music, weather, phone alarms) + the Momo app, which opens in
             the phone's browser over Wi-Fi (alarms, reminders, sounds,
             games, backup). No extra parts are needed for Momo 2.

   What Momo does
     - 46 expressions that glide smoothly from one to the next
     - Follows your day: wakes up in the morning, gets sleepy in the
       evening, sleeps at night (screen dimmed, sounds off)
     - Personality: remembers how much you pet it; affection sounds
       (chirps, purrs, giggles, kisses) and heart eyes
     - Alarms with snooze, from the Momo app or the Chronos app
     - Water and stretch reminders at exact times and/or every X minutes,
       with a filling glass and a stretching buddy
     - Games you play (Jump, Flappy, Catch by tilting, Reaction) with
       saved best scores; plays Pong by itself only when bored, and only
       in play hours
     - Music from your phone: song title, equaliser animation,
       tap = play/pause, tilt left/right = previous/next
     - Weather from the phone or the internet, with animated icons
     - Sound themes (cute, robot, soft, retro), volume, quiet hours and
       your own tune
     - Sends and receives hugs to/from other Momos nearby

   How to use Momo (head touch and back button work the same way)
     - Tap            : pet Momo / next menu item
     - Hold 1 second  : open the menu / choose
     - Double-tap     : BACK (out of a game, up one menu, close a screen)
     - Keep a hand on the head : rub - Momo closes its eyes and purrs
     - Hold the back button 3 s: sleep (press it again to wake Momo)
     - Shake Momo     : it gets dizzy
     - Music screen   : tap = play/pause, tilt left/right = previous/next

   Arduino IDE settings (Tools menu) for the real robot
     Board            : ESP32C3 Dev Module
     USB CDC On Boot  : Enabled
     Partition Scheme : Huge APP (3MB No OTA/1MB SPIFFS)
     Flash Mode       : DIO
   Libraries to install
     Adafruit GFX Library, Adafruit SH110X, Adafruit BusIO, ArduinoJson,
     ChronosESP32 (it also installs NimBLE-Arduino and ESP32Time)
   Everything else (Wi-Fi, alarms, reminders, sounds...) is set in the
   Momo app - nothing to change in this code.
  =====================================================================
*/

// ===================== SETTINGS YOU CAN CHANGE ======================

#define WOKWI 1   // 0 = real Momo robot, 1 = Wokwi simulator

// Screen chip: most 1.3" screens use SH1106 (leave 0). If the picture looks
// shifted with a noisy line at one edge, your screen is an SSD1306: set 1 and
// install the "Adafruit SSD1306" library.
#define SCREEN_IS_SSD1306 0

const uint8_t OLED_ADDR   = 0x3C;     // try 0x3D if the screen stays blank
const bool    FLIP_SCREEN = false;    // true if the face shows upside down

#define MOMO_VERSION "2.2"

// ===================== PINS AND ADDRESSES ===========================

const int PIN_TOUCH  = 3;   // TTP223 OUT  (HIGH while touched)
const int PIN_BUZZER = 10;  // passive buzzer +
const int PIN_SDA    = 8;   // I2C data  : screen, clock, motion sensor
const int PIN_SCL    = 9;   // I2C clock : screen, clock, motion sensor

const uint8_t RTC_ADDR = 0x68;   // DS3231
const uint8_t MPU_ADDR = 0x69;   // MPU6050 (AD0 pin connected to 3.3V)

// ===================== LIBRARIES ====================================

#if WOKWI
#undef SCREEN_IS_SSD1306
#define SCREEN_IS_SSD1306 1   // Wokwi only has the SSD1306 screen
#define RTC_IS_UTC 1          // Wokwi's clock module starts at UTC time
#else
#define RTC_IS_UTC 0          // the real clock module keeps your local time
#endif
#ifndef MOMO_NO_WIFI
#define MOMO_NO_WIFI 0        // 1 = build without Wi-Fi (simulator fallback)
#endif

#include <Wire.h>
#include <Adafruit_GFX.h>
#if SCREEN_IS_SSD1306
#include <Adafruit_SSD1306.h>
#else
#include <Adafruit_SH110X.h>
#endif
#if !WOKWI
#include <ChronosESP32.h>
#endif
#include <Preferences.h>
#include <ArduinoJson.h>
#include <esp_system.h>
#include <esp_random.h>
#include <esp_rom_sys.h>
#include "mbedtls/sha256.h"
#if !MOMO_NO_WIFI
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>
#include <ESPmDNS.h>
#include <HTTPClient.h>
#include <NetworkClientSecure.h>
#endif
#if !WOKWI && !MOMO_NO_WIFI
#include <esp_wifi.h>
#include <esp_now.h>
#endif
#include <time.h>
#include <math.h>

#if SCREEN_IS_SSD1306
Adafruit_SSD1306 display(128, 64, &Wire, -1);
#else
Adafruit_SH1106G display(128, 64, &Wire, -1);
#endif
#if !WOKWI
ChronosESP32 watch("Momo");   // Bluetooth link to the Chronos app
#endif

const uint16_t INK = 1;     // white pixel
const uint16_t ERASE = 0;   // black pixel

#include "store.h"
#include "sound.h"
#include "face.h"

// ===================== SHARED STATE =================================

enum Scr : uint8_t {
  SC_BOOT, SC_FACE, SC_MENU, SC_CLOCK, SC_FOCUS, SC_ALERT, SC_CALL, SC_GAME, SC_PONG, SC_WEATHER,
  SC_MUSIC, SC_ALARM, SC_WATER, SC_STRETCH, SC_GALLERY, SC_PAIR, SC_WIFI, SC_HELLO, SC_NOTE, SC_CHARGE
};
Scr screen = SC_BOOT;
unsigned long screenStart = 0;

enum Life : uint8_t { L_AWAKE, L_BORED, L_NAP, L_NIGHT };
Life life = L_AWAKE;
unsigned long lifeSince = 0;

bool displayOk = false, rtcOk = false, mpuOk = false;
float tiltX = 0;                     // sideways tilt in g, smoothed

uint8_t tempFace = 255;              // a short expression (reaction)
unsigned long tempUntil = 0;
uint8_t idleFace = 255;              // a short idle expression
unsigned long idleUntil = 0, nextIdle = 0;
unsigned long lastTouch = 0, lastSelfPlay = 0, lastAffectionDecay = 0, pongUntil = 0;
unsigned long lastFrame = 0;
const unsigned long FRAME_TIME = 40; // 25 frames per second

const unsigned long LONG_PRESS = 800;      // hold = choose / menu
const unsigned long DOUBLE_TAP_MS = 250;   // next tap starts this soon after the last one = back
const unsigned long RUB_MS = 1600;         // hand kept on the head this long = rub
const unsigned long SLEEP_HOLD_MS = 3000;  // back button held this long = sleep

// Two ways to talk to Momo: the touch sensor on the head and the back button.
// Both understand tap, hold and double-tap (= back).
enum InSrc : uint8_t { SRC_HEAD, SRC_BUTTON };
struct InputState { bool pressed, longDone, rubbing; unsigned long pressStart, lastEdge; };
InputState inHead = {false, false, false, 0, 0}, inBtn = {false, false, false, 0, 0};
bool tapPending = false, secondPress = false;
unsigned long tapPendingAt = 0, lastTapAt = 0;
uint8_t tapStreak = 0;
unsigned long rubStart = 0, rubNextPurr = 0, rubNextLove = 0;

unsigned long nextMotionRead = 0, shakeCooldown = 0, tiltSince = 0;
float shakeScore = 0;
int8_t tiltState = 0;

enum AlertKind : uint8_t { AL_PHONE, AL_HUG, AL_SPACE, AL_DONE, AL_INFO, AL_FIND, AL_PAIR, AL_WIFI, AL_BATTERY };
AlertKind alertKind = AL_INFO;
char alertTitle[19];
char alertText[200];
char alertLines[12][22];
uint8_t alertLineCount = 0;
unsigned long alertTimeout = 20000;

char callerName[32];
bool callMuted = false;
unsigned long nextRing = 0;

bool focusRunning = false, onBreak = false;
unsigned long focusEnd = 0, focusTotal = 1;

enum MenuId : uint8_t { MENU_MAIN, MENU_GAMES, MENU_PHONE, MENU_TEST };
enum MenuItem : uint8_t {
  MI_CLOCK, MI_MUSIC, MI_WEATHER, MI_GAMES, MI_FOCUS, MI_WATER, MI_STRETCH, MI_FACES, MI_HUG, MI_SOUND,
  MI_PHONE, MI_TEST, MI_BACK,
  MI_G_JUMP, MI_G_FLAPPY, MI_G_CATCH, MI_G_REACT, MI_G_SIMON, MI_G_COPY, MI_G_RHYTHM, MI_G_RPS,
  MI_P_APP, MI_P_PAIR, MI_P_SETUP, MI_P_UNPAIR,
  MI_T_MSG, MI_T_CALL, MI_T_MUSIC, MI_T_WEATHER, MI_T_ALARM, MI_T_WATER, MI_T_STRETCH, MI_T_HUG,
  MI_T_SPACE, MI_T_MORNING, MI_T_NIGHT
};
uint8_t menuId = MENU_MAIN;
uint8_t menuItems[16];
uint8_t menuCount = 0, menuSel = 0;
uint8_t galleryIdx = 0;
uint8_t lastVolume = 2;

char musicTitle[48] = "", musicArtist[32] = "";
bool musicPlaying = false, musicKnown = false;
int8_t musicCmdShown = 0;
unsigned long musicCmdAt = 0, musicSeenAt = 0;

enum WxIcon : uint8_t { WX_SUN, WX_PARTLY, WX_CLOUD, WX_RAIN, WX_STORM, WX_SNOW, WX_FOG, WX_WIND };
struct WxDay { uint8_t icon; int8_t hi, lo; };
struct WeatherNow {
  bool valid, fromPhone;
  char city[24];
  int temp, high, low;
  uint8_t icon, humidity, dayCount;
  WxDay days[3];
  unsigned long at;
};
WeatherNow wx;

int8_t ringingAlarm = -1;
unsigned long ringStart = 0, nextRingSound = 0;
bool ringWasBusy = false;
uint8_t ringSoundId = 2, ringSnoozeMin = 5;
char ringLabel[16] = "";
int8_t snoozeIdx = -1;
unsigned long snoozeAt = 0;
uint8_t snoozeSound = 2, snoozeMin = 5;
char snoozeLabel[16] = "";

// your own reminders ("Book ticket" at 16:30)
int8_t noteIdx = -1;                 // the one on screen
unsigned long noteStart = 0, noteNextRing = 0;
bool noteRingWasBusy = false;
uint16_t noteFiredMin[MAX_NOTES];
unsigned long noteSnoozeUntil[MAX_NOTES];
bool notePending[MAX_NOTES];

uint8_t remindKind = 0;              // 0 water, 1 stretch
bool remindDone = false;
unsigned long remindDoneAt = 0, remindNextRing = 0;
uint8_t remindRings = 0;

bool helloMorning = true;
bool nightSoundOk = false;           // lets the "good night" sound through

char pairCode[7] = "";
unsigned long pairUntil = 0;
bool wifiConnected = false, apMode = false;
char apName[24] = "", apPass[12] = "", ipText[16] = "";

// Events from Bluetooth / radio (background tasks): noted here, handled in loop()
volatile bool evNotif = false, evTimeSet = false, evFind = false, evHug = false, evWeather = false, evMusic = false;
volatile uint8_t evCall = 0, evConn = 0;
volatile uint8_t evCfgMask = 0;
volatile uint32_t evCfgA[4], evCfgB[4];   // 0 water, 1 stretch, 2 quiet hours, 3 sleep time
char evApp[24], evText[200], evCaller[32], evHugFrom[16];
bool hugsReady = false;

// ===================== FUNCTION LIST ================================

bool phoneConnected();
uint32_t minutesSince2024();
const char *menuTitle();
const char *menuLabel(uint8_t item);
const char *currentSsid();
void goScreen(Scr s, unsigned long now);
void backToFace(unsigned long now, uint8_t face);
void showFace(uint8_t face, unsigned long ms, unsigned long now);
void react(uint8_t face, unsigned long ms, unsigned long now);
void startNote(uint8_t i, unsigned long now);
void updateNotes(unsigned long now);
void noteDoneTap(unsigned long now);
void noteSnooze(unsigned long now);
void showAlert(AlertKind kind, const char *title, const char *text, unsigned long now);
void showPairScreen(unsigned long now);
void endPairScreen(unsigned long now, bool ok);
void settingsChanged(unsigned long now);
bool appAction(const char *what, JsonObjectConst args, unsigned long now);
void timeFromNetwork(const struct tm &t, unsigned long now);
void weatherArrived(unsigned long now);
void wifiJoined(unsigned long now);
bool i2cPresent(uint8_t addr);
bool i2cWriteReg(uint8_t addr, uint8_t reg, uint8_t val);
bool i2cReadRegs(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t n);
uint8_t bcd2dec(uint8_t v);
uint8_t dec2bcd(uint8_t v);
int dayOfWeek(int y, int m, int d);
int64_t epochOf(const struct tm &t);
bool readRtc(struct tm &t);
bool writeRtc(const struct tm &t);
void refreshTime(unsigned long now);
bool mpuInit();
void readMotion(unsigned long now);
void onShake(unsigned long now);
void onTilt(int8_t dir, unsigned long now);
void readTouch(unsigned long now);
void readInputs(unsigned long now);
void onPress(unsigned long now);
void onTap(unsigned long now);
void onLongPress(unsigned long now);
void onBack(unsigned long now);
bool holdOnRelease(uint8_t src);
bool backByDoubleTap();
void startRub(unsigned long now);
void updateRub(unsigned long now);
void endRub(unsigned long now);
void drawHeroWater(unsigned long now);
void drawHeroStretch(unsigned long now);
void drawHeroBoot(unsigned long now);
void petMomo(unsigned long now);
void wakeFromTouch(unsigned long now);
void setDim(bool dim);
void setLife(Life l, unsigned long now);
uint8_t baseFaceNow();
void updateFace(unsigned long now);
void updateLife(unsigned long now);
void enterNight(unsigned long now);
void wakeUpMorning(unsigned long now);
unsigned long boredMs();
unsigned long napMs();
void openMenu(uint8_t id, unsigned long now);
void chooseMenuItem(unsigned long now);
void startFocus(unsigned long now);
void updateFocus(unsigned long now);
void startCall(const char *caller, unsigned long now);
void startAlarm(int idx, unsigned long now);
void snoozeRinging(unsigned long now);
void stopRinging(unsigned long now, bool byUser);
void updateAlarm(unsigned long now);
void requestReminder(uint8_t kind, unsigned long now);
void showReminder(uint8_t kind, unsigned long now);
void reminderDoneTap(unsigned long now);
void reminderSnooze(unsigned long now);
void updateReminders(unsigned long now);
void checkSchedule(unsigned long now);
void musicCommand(int8_t dir, unsigned long now);
void musicInfoArrived(unsigned long now);
uint8_t weatherFace();
void openWeather(unsigned long now);
void updateScreens(unsigned long now);
void handleEvents(unsigned long now);
void startHugs();
void sendHug(unsigned long now);
void startGame(uint8_t id, unsigned long now);
void testMenuAction(uint8_t item, unsigned long now);
void drawFrame(unsigned long now);
#if !WOKWI
void onPhoneConnection(bool connected);
void onPhoneNotification(Notification n);
void onPhoneRinger(String caller, bool ringing);
void onPhoneConfig(Config config, uint32_t a, uint32_t b);
#endif
#if !WOKWI && !MOMO_NO_WIFI
#if ESP_ARDUINO_VERSION_MAJOR >= 3
void onEspNowData(const esp_now_recv_info_t *info, const uint8_t *data, int len);
#else
void onEspNowData(const uint8_t *mac, const uint8_t *data, int len);
#endif
#endif
#if WOKWI
void momoEarlyHello() __attribute__((constructor));
const char *resetName(esp_reset_reason_t r);
#endif

// Simulator check-points: in Wokwi each step of start-up is printed in the
// terminal, so a problem shows exactly where it happens. Off on the real robot.
#if WOKWI
#define STEP(msg) do { Serial.println("[Momo] " msg); Serial.flush(); } while (0)
#else
#define STEP(msg) do { } while (0)
#endif

#include "schedule.h"
#include "power.h"
#include "games.h"
#include "screens.h"
#include "hero.h"
#include "net.h"

// ===================== TIMINGS ======================================

#if WOKWI   // short times so everything can be tried quickly in the simulator
unsigned long boredMs() { return 20000; }
unsigned long napMs() { return 90000; }
const unsigned long PLAY_GAP_MS = 60000, PONG_MS = 12000, SNOOZE_REMIND_MS = 30000, NOTE_SNOOZE_MS = 30000;
#else
unsigned long boredMs() { return (unsigned long)cfg.boredMin * 60000UL; }
unsigned long napMs() { return (unsigned long)cfg.napMin * 60000UL; }
const unsigned long PLAY_GAP_MS = 0, PONG_MS = 30000, SNOOZE_REMIND_MS = 600000, NOTE_SNOOZE_MS = 300000;
#endif


// ===================== SMALL HELPERS ================================

bool phoneConnected() {
#if WOKWI
  return false;
#else
  return watch.isConnected();
#endif
}

// Minutes since 1 Jan 2024 (used to remember when a phone was paired)
uint32_t minutesSince2024() {
  if (!haveTime) return millis() / 60000;
  int y = nowTm.tm_year + 1900;
  uint32_t days = 0;
  for (int yy = 2024; yy < y; yy++) days += (yy % 4 == 0 && (yy % 100 != 0 || yy % 400 == 0)) ? 366 : 365;
  days += (uint32_t)nowTm.tm_yday;
  return days * 1440 + (uint32_t)(nowTm.tm_hour * 60 + nowTm.tm_min);
}

// ===================== I2C, CLOCK AND MOTION ========================

bool i2cPresent(uint8_t addr) {
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}

bool i2cWriteReg(uint8_t addr, uint8_t reg, uint8_t val) {
  Wire.beginTransmission(addr);
  Wire.write((uint8_t)reg);
  Wire.write((uint8_t)val);
  return Wire.endTransmission() == 0;
}

bool i2cReadRegs(uint8_t addr, uint8_t reg, uint8_t *buf, uint8_t n) {
  Wire.beginTransmission(addr);
  Wire.write((uint8_t)reg);
  if (Wire.endTransmission(false) != 0) return false;
  if (Wire.requestFrom(addr, (size_t)n, true) != (size_t)n) return false;   // same call works on core 2.x and 3.x
  for (uint8_t i = 0; i < n; i++) buf[i] = (uint8_t)Wire.read();
  return true;
}

uint8_t bcd2dec(uint8_t v) { return (uint8_t)((v >> 4) * 10 + (v & 0x0F)); }
uint8_t dec2bcd(uint8_t v) { return (uint8_t)(((v / 10) << 4) | (v % 10)); }

// 0 = Sunday ... 6 = Saturday
int dayOfWeek(int y, int m, int d) {
  static const int t[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};
  if (m < 3) y -= 1;
  return (y + y / 4 - y / 100 + y / 400 + t[m - 1] + d) % 7;
}

// Seconds since 1970 for a calendar date and time (no time zone applied)
int64_t epochOf(const struct tm &t) {
  int y = t.tm_year + 1900, m = t.tm_mon + 1, d = t.tm_mday;
  y -= m <= 2;
  int era = (y >= 0 ? y : y - 399) / 400;
  int yoe = y - era * 400;
  int doy = (153 * (m + (m > 2 ? -3 : 9)) + 2) / 5 + d - 1;
  int doe = yoe * 365 + yoe / 4 - yoe / 100 + doy;
  int64_t days = (int64_t)era * 146097 + doe - 719468;
  return days * 86400 + t.tm_hour * 3600 + t.tm_min * 60 + t.tm_sec;
}

bool readRtc(struct tm &t) {
  uint8_t b[7];
  if (!i2cReadRegs(RTC_ADDR, 0x00, b, 7)) return false;
  memset(&t, 0, sizeof(t));
  t.tm_sec  = bcd2dec(b[0] & 0x7F);
  t.tm_min  = bcd2dec(b[1] & 0x7F);
  t.tm_hour = bcd2dec(b[2] & 0x3F);
  t.tm_mday = bcd2dec(b[4] & 0x3F);
  t.tm_mon  = bcd2dec(b[5] & 0x1F) - 1;
  t.tm_year = bcd2dec(b[6]) + 100;
  if (t.tm_year < 124 || t.tm_mon < 0 || t.tm_mon > 11 || t.tm_mday < 1 || t.tm_hour > 23 || t.tm_min > 59) return false;
#if RTC_IS_UTC
  time_t e = (time_t)(epochOf(t) + cfg.utcOffset);   // simulator clock runs on UTC: show local time
  gmtime_r(&e, &t);
  return true;
#endif
  t.tm_wday = dayOfWeek(t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);
  static const int cum[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334};
  int y = t.tm_year + 1900;
  bool leap = (y % 4 == 0 && (y % 100 != 0 || y % 400 == 0));
  t.tm_yday = cum[t.tm_mon] + t.tm_mday - 1 + ((leap && t.tm_mon > 1) ? 1 : 0);
  return true;
}

bool writeRtc(const struct tm &local) {
  struct tm t = local;
#if RTC_IS_UTC
  time_t e = (time_t)(epochOf(local) - cfg.utcOffset);
  gmtime_r(&e, &t);
#endif
  int year = t.tm_year + 1900;
  if (year < 2024 || year > 2099) return false;
  Wire.beginTransmission(RTC_ADDR);
  Wire.write((uint8_t)0x00);
  Wire.write(dec2bcd((uint8_t)t.tm_sec));
  Wire.write(dec2bcd((uint8_t)t.tm_min));
  Wire.write(dec2bcd((uint8_t)t.tm_hour));
  Wire.write(dec2bcd((uint8_t)(dayOfWeek(year, t.tm_mon + 1, t.tm_mday) + 1)));
  Wire.write(dec2bcd((uint8_t)t.tm_mday));
  Wire.write(dec2bcd((uint8_t)(t.tm_mon + 1)));
  Wire.write(dec2bcd((uint8_t)(year - 2000)));
  bool ok = Wire.endTransmission() == 0;
  Serial.printf("Clock module %s: %02d:%02d %02d/%02d/%d\n", ok ? "set" : "NOT set",
                local.tm_hour, local.tm_min, local.tm_mday, local.tm_mon + 1, local.tm_year + 1900);
  return ok;
}

// Best available time: clock module first, then phone / internet time
void refreshTime(unsigned long now) {
  if ((long)(now - nextTimeRead) < 0) return;
  nextTimeRead = now + 500;
  struct tm t;
  if (rtcOk && readRtc(t)) {
    nowTm = t;
    haveTime = true;
    return;
  }
  time_t s = time(nullptr);
  if (s > 1700000000) {
    localtime_r(&s, &nowTm);
    haveTime = true;
    return;
  }
  haveTime = false;
}

void timeFromNetwork(const struct tm &t, unsigned long now) {
  if (rtcOk) writeRtc(t);
  nowTm = t;
  haveTime = true;
  nextTimeRead = 0;
  Serial.printf("Time from the internet: %02d:%02d\n", t.tm_hour, t.tm_min);
  (void)now;
}

bool mpuInit() {
  return i2cWriteReg(MPU_ADDR, 0x6B, 0x00)     // wake up
      && i2cWriteReg(MPU_ADDR, 0x1C, 0x00);    // +/- 2g range
}

void readMotion(unsigned long now) {
  if (!mpuOk || (long)(now - nextMotionRead) < 0) return;
  nextMotionRead = now + 20;
  uint8_t b[6];
  if (!i2cReadRegs(MPU_ADDR, 0x3B, b, 6)) return;
  float ax = (int16_t)((b[0] << 8) | b[1]) / 16384.0f;
  float ay = (int16_t)((b[2] << 8) | b[3]) / 16384.0f;
  float az = (int16_t)((b[4] << 8) | b[5]) / 16384.0f;
  float dev = fabsf(sqrtf(ax * ax + ay * ay + az * az) - 1.0f);   // 0 when still
  if (dev > 0.6f) shakeScore += 1.0f;
  else if (shakeScore > 0) shakeScore -= 0.25f;
  if (shakeScore >= 4.0f && (long)(now - shakeCooldown) >= 0) {
    shakeScore = 0;
    shakeCooldown = now + (screen == SC_GAME ? 700 : 2500);
    onShake(now);
  }
  float side = cfg.tiltFlip ? -ax : ax;
  tiltX += (side - tiltX) * 0.3f;
  // tilt gesture (music): lean past ~27 degrees for 0.25 s, come back to use again
  if (tiltState == 0) {
    if (fabsf(tiltX) > 0.45f) {
      if (tiltSince == 0) tiltSince = now;
      if (now - tiltSince > 250) {
        tiltState = tiltX > 0 ? 1 : -1;
        onTilt(tiltState, now);
      }
    } else {
      tiltSince = 0;
    }
  } else if (fabsf(tiltX) < 0.2f) {
    tiltState = 0;
    tiltSince = 0;
  }
}

void onShake(unsigned long now) {
  if (screen == SC_GAME) { lastTouch = now; gameShake(now); return; }   // shaking is part of some games
  lastTouch = now;
  Serial.println("Shake!");
  if (screen == SC_FACE || screen == SC_PONG) {
    if (life != L_AWAKE) setLife(L_AWAKE, now);
    goScreen(SC_FACE, now);
    react(F_DIZZY, 3000, now);
  }
}

void onTilt(int8_t dir, unsigned long now) {
  if (screen == SC_MUSIC) musicCommand(dir, now);
  else if (screen == SC_GAME) gameTilt(dir, now);
}

// ===================== TOUCH AND BACK BUTTON ========================
// Head (TTP223, HIGH while touched) and back button (LOW while pressed) share
// the same gestures. A single tap waits a moment on screens where a second
// tap means BACK, so a double-tap never does "next" first.

// Screens where double-tap = back (taps there wait 0.3 s to see if a second one comes)
bool backByDoubleTap() {
  switch (screen) {
    case SC_MENU: case SC_GALLERY: case SC_CLOCK: case SC_WEATHER: case SC_MUSIC: case SC_FOCUS:
    case SC_ALERT: case SC_WIFI: case SC_HELLO: case SC_PONG: case SC_PAIR: case SC_CHARGE:
      return true;
    case SC_GAME:
      return gState != GS_PLAY;              // while playing, taps are moves (hold = back)
    default:
      return false;                          // face (pets are instant), alarm, reminders, call
  }
}

// Holds that act when you let go: the head on the face screen (keep holding = rub)
// and the back button (keep holding 3 s = sleep)
bool holdOnRelease(uint8_t src) {
  if (src == SRC_BUTTON) return true;
  return screen == SC_FACE && x22.rub && life != L_NIGHT;
}

void handleSource(uint8_t src, bool down, unsigned long now) {
  InputState &s = src == SRC_HEAD ? inHead : inBtn;
  if (down != s.pressed && now - s.lastEdge > 30) {   // ignore 30 ms of bounce
    s.lastEdge = now;
    s.pressed = down;
    if (down) {
      s.pressStart = now;
      s.longDone = false;
      s.rubbing = false;
      secondPress = tapPending && now - tapPendingAt <= DOUBLE_TAP_MS;
      if (secondPress) tapPending = false;
      onPress(now);
    } else if (s.rubbing) {
      s.rubbing = false;
      endRub(now);
    } else if (!s.longDone) {
      unsigned long held = now - s.pressStart;
      if (holdOnRelease(src) && held >= LONG_PRESS) {   // a hold that acts on release
        if (secondPress) { secondPress = false; onTap(now); }
        onLongPress(now);
      } else if (secondPress) {
        secondPress = false;
        onBack(now);
      } else if (backByDoubleTap()) {
        tapPending = true;
        tapPendingAt = now;
      } else {
        onTap(now);
      }
    }
  }
  if (s.pressed && !s.longDone) {
    unsigned long held = now - s.pressStart;
    if (!holdOnRelease(src)) {
      if (held >= LONG_PRESS) {
        s.longDone = true;
        if (secondPress) { secondPress = false; onTap(now); }
        onLongPress(now);
      }
    } else if (src == SRC_HEAD) {
      if (held >= RUB_MS) {                  // hand stays on the head: rub
        s.longDone = true;
        s.rubbing = true;
        if (secondPress) { secondPress = false; onTap(now); }
        startRub(now);
      }
    } else if (held >= SLEEP_HOLD_MS) {      // back button held: sleep
      s.longDone = true;
      secondPress = false;
      softOff(now, 1);
    }
  }
  if (s.rubbing) updateRub(now);
}

void readInputs(unsigned long now) {
  handleSource(SRC_HEAD, digitalRead(PIN_TOUCH) == HIGH, now);
  handleSource(SRC_BUTTON, buttonDown(), now);
  if (tapPending && now - tapPendingAt > DOUBLE_TAP_MS && !inHead.pressed && !inBtn.pressed) {
    tapPending = false;
    onTap(now);
  }
}

void readTouch(unsigned long now) { readInputs(now); }

void onPress(unsigned long now) {
  lastTouch = now;
  if (screen == SC_GAME && gState == GS_PLAY) gamePress(now);   // instant: jumping, flapping
}

// BACK: one step back from wherever Momo is
void onBack(unsigned long now) {
  lastTouch = now;
  Serial.println("Back");
  switch (screen) {
    case SC_MENU:
      playSound(S_BACK);
      if (menuId != MENU_MAIN) openMenu(MENU_MAIN, now);
      else backToFace(now, F_NEUTRAL);
      break;
    case SC_GAME:
      playSound(S_BACK);
      openMenu(MENU_GAMES, now);
      break;
    case SC_PAIR:
      pairUntil = 0;
      memset(pairCode, 0, sizeof(pairCode));
      playSound(S_BACK);
      backToFace(now, F_NEUTRAL);
      break;
    case SC_PONG:
      lastSelfPlay = now;
      setLife(L_AWAKE, now);
      backToFace(now, F_HAPPY);
      break;
    case SC_FACE:
      if (life != L_AWAKE) wakeFromTouch(now);
      break;
    default:
      playSound(S_BACK);
      backToFace(now, F_NEUTRAL);
      break;
  }
}

// ---------- rubbing Momo's head ----------
void startRub(unsigned long now) {
  rubStart = now;
  rubNextPurr = now;
  rubNextLove = now + 2000;
  st.pets++;
  markStats(now);
  lookAt(0, 0);
  Serial.println("Rub! Momo closes its eyes and purrs");
}

void updateRub(unsigned long now) {
  lastTouch = now;
  showFace(F_GRATEFUL, 700, now);            // happy closed eyes + a floating heart
  if ((long)(now - rubNextPurr) >= 0) {
    playSound(S_PURR);
    rubNextPurr = now + 1500;
  }
  if ((long)(now - rubNextLove) >= 0) {      // a little more love every 2 seconds
    st.affection = (uint8_t)min(100, st.affection + 1);
    markStats(now);
    rubNextLove = now + 2000;
  }
}

void endRub(unsigned long now) {
  unsigned long dur = now - rubStart;
  Serial.printf("Rub ended after %lu s, affection %u\n", dur / 1000, (unsigned)st.affection);
  react(dur > 4000 ? F_LOVE : F_RELAXED, 2400, now);
}

void wakeFromTouch(unsigned long now) {
  if (life == L_NIGHT) {                 // a little peek at night
    showFace(F_SLEEPY, 5000, now);
    return;
  }
  if (life == L_NAP || life == L_BORED) {
    bool wasNap = (life == L_NAP);
    setLife(L_AWAKE, now);
    react(wasNap ? F_SURPRISED : F_EXCITED, 1500, now);
  }
}

void onTap(unsigned long now) {
  switch (screen) {
    case SC_FACE:
      if (life != L_AWAKE) wakeFromTouch(now);
      else petMomo(now);
      break;
    case SC_MENU:
      menuSel = (uint8_t)((menuSel + 1) % menuCount);
      screenStart = now;
      playSound(S_CLICK);
      break;
    case SC_CALL:    callMuted = true; break;
    case SC_PONG:    lastSelfPlay = now; setLife(L_AWAKE, now); goScreen(SC_FACE, now); react(F_HAPPY, 2200, now); break;
    case SC_GAME:    if (gState != GS_PLAY) gamePress(now); break;   // start / play again (moves are on press)
    case SC_MUSIC:   musicCommand(0, now); break;
    case SC_ALARM:   snoozeRinging(now); break;
    case SC_WATER:
    case SC_STRETCH: reminderDoneTap(now); break;
    case SC_GALLERY:
      galleryIdx = (uint8_t)((galleryIdx + 1) % F_COUNT);
      setFace(galleryIdx);
      playFaceSound(galleryIdx);      // every emotion has its own sound
      screenStart = now;
      break;
    case SC_PAIR:    break;
    case SC_NOTE:    noteDoneTap(now); break;
    case SC_FOCUS:   backToFace(now, F_FOCUSED); break;
    case SC_WEATHER: backToFace(now, weatherFace()); break;
    case SC_CHARGE:  backToFace(now, F_HAPPY); break;
    default:         backToFace(now, F_HAPPY); break;
  }
}

void onLongPress(unsigned long now) {
  switch (screen) {
    case SC_MENU:    chooseMenuItem(now); break;
    case SC_GAME:    playSound(S_BACK); openMenu(MENU_GAMES, now); break;
    case SC_ALARM:   stopRinging(now, true); break;
    case SC_NOTE:    noteSnooze(now); break;
    case SC_WATER:
    case SC_STRETCH: reminderSnooze(now); break;
    case SC_CALL:    callMuted = true; backToFace(now, F_NEUTRAL); break;
    case SC_PAIR:
      pairUntil = 0;
      memset(pairCode, 0, sizeof(pairCode));
      backToFace(now, F_NEUTRAL);
      break;
    case SC_FACE:
      if (life == L_NIGHT) showFace(F_SLEEPY, 5000, now);
      openMenu(MENU_MAIN, now);
      break;
    default:         playSound(S_BACK); backToFace(now, F_NEUTRAL); break;
  }
}

// ===================== AFFECTION ====================================

void petMomo(unsigned long now) {
  st.pets++;
  st.affection = (uint8_t)min(100, st.affection + 2);
  markStats(now);
  tapStreak = (now - lastTapAt < 1200) ? (uint8_t)min(250, tapStreak + 1) : 1;
  lastTapAt = now;
  Serial.printf("Pet! pets=%u affection=%u\n", (unsigned)st.pets, (unsigned)st.affection);
  if (st.pets % 10 == 0) {
    react(F_LOVE, 2600, now);
  } else if (tapStreak >= 5) {
    react(F_CELEBRATE, 2200, now);
  } else if (tapStreak >= 3) {
    react(F_LAUGH, 1800, now);
  } else if (st.affection >= 85 && random(0, 4) == 0) {
    react(F_KISS, 1800, now);
  } else {
    int pick = (int)random(0, st.affection >= 70 ? 4 : (st.affection >= 50 ? 3 : 2));
    switch (pick) {
      case 0: react(F_HAPPY, 1600, now); break;
      case 1: react(F_BLUSH, 1800, now); break;
      case 2: react(F_RELAXED, 2000, now); break;
      default: react(F_GRATEFUL, 2000, now); break;
    }
  }
}

// ===================== FACE AND DAY RHYTHM ==========================

void setDim(bool dim) {
#if SCREEN_IS_SSD1306
  display.dim(dim);
#else
  display.setContrast(dim ? 1 : 0x80);
#endif
}

void setLife(Life l, unsigned long now) {
  if (l == life) return;
  life = l;
  lifeSince = now;
  setDim(l == L_NIGHT);
  const char *names[4] = {"awake", "bored", "napping", "asleep for the night"};
  Serial.printf("Momo is %s\n", names[l]);
}

void showFace(uint8_t face, unsigned long ms, unsigned long now) {
  tempFace = face;
  tempUntil = now + ms;
}

// Momo reacts: the expression plus that emotion's own sound
void react(uint8_t face, unsigned long ms, unsigned long now) {
  showFace(face, ms, now);
  playFaceSound(face);
}

uint8_t baseFaceNow() {
  switch (life) {
    case L_NIGHT:
    case L_NAP:   return F_ASLEEP;
    case L_BORED: return F_BORED;
    default: break;
  }
  if (musicPlaying) return F_MUSIC;
  if (focusRunning && !onBreak) return F_FOCUSED;
  if (st.affection < 20) return F_DISAPPOINTED;
  return F_NEUTRAL;
}

void updateFace(unsigned long now) {
  if (screen == SC_GALLERY) { setFace(galleryIdx); return; }
  uint8_t target;
  if (tempFace != 255 && (long)(now - tempUntil) < 0) {
    target = tempFace;
  } else {
    tempFace = 255;
    target = baseFaceNow();
    // idle expressions, only when Momo is awake and calm
    if (target == F_NEUTRAL && life == L_AWAKE && screen == SC_FACE) {
      if (idleFace != 255 && (long)(now - idleUntil) < 0) {
        target = idleFace;
      } else {
        idleFace = 255;
        if ((long)(now - nextIdle) >= 0) {
          DayPart dp = dayPart();
          static const uint8_t dayPool[] = {F_CURIOUS, F_THINKING, F_RELAXED, F_HAPPY, F_HOPEFUL, F_SNEAKY, F_WINK, F_PLAYFUL};
          static const uint8_t morningPool[] = {F_SLEEPY, F_YAWN, F_RELAXED, F_HAPPY};
          static const uint8_t eveningPool[] = {F_SLEEPY, F_YAWN, F_RELAXED, F_THINKING};
          static const uint8_t lonelyPool[] = {F_SAD, F_HOPEFUL, F_DISAPPOINTED, F_BORED};
          const uint8_t *pool = dayPool;
          int n = st.affection >= 70 ? 8 : 6;
          if (dp == DP_MORNING) { pool = morningPool; n = 4; }
          else if (dp == DP_EVENING) { pool = eveningPool; n = 4; }
          else if (st.affection < 30) { pool = lonelyPool; n = 4; }
          if (nextIdle != 0) {
            idleFace = pool[random(0, n)];
            idleUntil = now + (unsigned long)random(2500, 4500);
          }
          unsigned long lo = cfg.lively == 0 ? 45000 : cfg.lively == 1 ? 20000 : 8000;
          unsigned long hi = cfg.lively == 0 ? 80000 : cfg.lively == 1 ? 45000 : 20000;
          nextIdle = now + (unsigned long)random((long)lo, (long)hi);
          if (idleFace != 255) target = idleFace;
        }
      }
      // look around now and then
      if ((long)(now - nextGaze) >= 0) {
        int r = (int)random(0, 10);
        if (r < 4) lookAt(0, 0);
        else lookAt((float)random(-9, 10), (float)random(-4, 5));
        nextGaze = now + (unsigned long)random(1800, 5000);
      }
    } else {
      lookAt(0, 0);
    }
  }
  setFace(target);
}

void enterNight(unsigned long now) {
  if (screen == SC_FACE || screen == SC_CLOCK || screen == SC_WEATHER || screen == SC_MENU) {
    helloMorning = false;
    goScreen(SC_HELLO, now);
    nightSoundOk = true;
    playSound(S_GOODNIGHT);
    nightSoundOk = false;
    showFace(F_YAWN, 2500, now);
  }
  setLife(L_NIGHT, now);
}

void wakeUpMorning(unsigned long now) {
  setLife(L_AWAKE, now);
  lastTouch = now;
  helloMorning = true;
  if (screen == SC_FACE || screen == SC_CLOCK || screen == SC_MENU || screen == SC_HELLO) goScreen(SC_HELLO, now);
  openEyesFromClosed();
  showFace(F_YAWN, 2500, now);
  playSound(S_WAKE);
}

void updateLife(unsigned long now) {
  bool night = isNight();
  if (night && life != L_NIGHT) {
    if (screen == SC_FACE || screen == SC_CLOCK || screen == SC_WEATHER || screen == SC_MENU || screen == SC_PONG) enterNight(now);
    return;
  }
  if (!night && life == L_NIGHT) {
    wakeUpMorning(now);
    return;
  }
  if (life == L_NIGHT) {
    // after a peek, back to sleep
    if (screen != SC_FACE && screen != SC_HELLO && now - lastTouch > 30000 && screen != SC_ALARM && screen != SC_NOTE) backToFace(now, F_ASLEEP);
    return;
  }
  unsigned long idle = now - lastTouch;
  bool busy = focusRunning || musicPlaying || screen != SC_FACE;
  if (life == L_AWAKE && !busy && idle > boredMs()) {
    setLife(L_BORED, now);
    playSound(S_SAD);
  }
  if (life == L_BORED && screen == SC_FACE && cfg.selfPlay && isPlayTime() && now - lifeSince > 6000 &&
      (lastSelfPlay == 0 || now - lastSelfPlay > (PLAY_GAP_MS ? PLAY_GAP_MS : (unsigned long)cfg.playGap * 60000UL))) {
    lastSelfPlay = now;
    pongReset();
    pongUntil = now + PONG_MS;
    goScreen(SC_PONG, now);
    Serial.println("Bored... playing a game by myself");
  }
  if ((life == L_AWAKE || life == L_BORED) && !busy && idle > napMs()) setLife(L_NAP, now);
  if (life != L_AWAKE && (focusRunning || musicPlaying) && screen == SC_FACE) setLife(L_AWAKE, now);
}

// ===================== SCREENS ======================================

void goScreen(Scr s, unsigned long now) {
  screen = s;
  screenStart = now;
}

void backToFace(unsigned long now, uint8_t face) {
  goScreen(SC_FACE, now);
  if (face != F_NEUTRAL) showFace(face, 2200, now);
}

void showAlert(AlertKind kind, const char *title, const char *text, unsigned long now) {
  if (screen == SC_CALL || screen == SC_ALARM || screen == SC_NOTE) return;   // never cover a call, an alarm or your reminder
  alertKind = kind;
  copyClean(alertTitle, sizeof(alertTitle), title);
  copyClean(alertText, sizeof(alertText), text);
  alertLineCount = wrapText(alertText, alertLines, 12);
  alertTimeout = 20000;
  goScreen(SC_ALERT, now);
  if (life != L_AWAKE && life != L_NIGHT) setLife(L_AWAKE, now);
  lastTouch = now;
  switch (kind) {
    case AL_HUG:   playSound(S_HUG); showFace(F_LOVE, 2500, now); break;
    case AL_DONE:  playSound(S_DONE); break;
    case AL_PHONE: playSound(S_NOTIFY); showFace(F_SURPRISED, 1200, now); break;
    case AL_FIND:  playSound(S_RING); break;
    case AL_PAIR:  playSound(S_PAIR); break;
    case AL_BATTERY: PLAY_NOTES(HS_LOWBATT, C_UI); break;
    default:       playSound(S_ALERT); break;
  }
  Serial.printf("Message: %s - %s\n", alertTitle, alertText);
}

void startCall(const char *caller, unsigned long now) {
  copyClean(callerName, sizeof(callerName), caller);
  if (callerName[0] == '\0') copyText(callerName, sizeof(callerName), "Unknown");
  callMuted = (life == L_NIGHT) || isQuiet();
  nextRing = now;
  goScreen(SC_CALL, now);
  lastTouch = now;
  Serial.printf("Incoming call: %s\n", callerName);
}

void showPairScreen(unsigned long now) {
  goScreen(SC_PAIR, now);
  playSound(S_PAIR);
}

void endPairScreen(unsigned long now, bool ok) {
  if (ok) showAlert(AL_PAIR, "Paired!", "Your phone can now control Momo. Unpair it any time in the app.", now);
  else if (screen == SC_PAIR) showAlert(AL_INFO, "Pairing locked", "Too many wrong codes. Try again in 5 minutes.", now);
}

void openWeather(unsigned long now) { goScreen(SC_WEATHER, now); }

uint8_t weatherFace() {
  if (!wx.valid) return F_CONFUSED;
  if (wx.temp >= 37) return F_EFFORT;
  if (wx.temp <= 8) return F_NERVOUS;
  switch (wx.icon) {
    case WX_SUN:   return F_HAPPY;
    case WX_PARTLY: return F_RELAXED;
    case WX_RAIN:  return F_RELAXED;
    case WX_STORM: return F_SCARED;
    case WX_SNOW:  return F_EXCITED;
    case WX_FOG:   return F_CONFUSED;
    case WX_WIND:  return F_SURPRISED;
    default:       return F_THINKING;
  }
}

void updateScreens(unsigned long now) {
  unsigned long on = now - screenStart;
  switch (screen) {
    case SC_BOOT:
      if (on > 1500) {
        goScreen(apMode ? SC_WIFI : SC_FACE, now);   // first start: show the setup hotspot
        if (life != L_NIGHT) {
          openEyesFromClosed();
          showFace(F_HAPPY, 2000, now);
          playSound(S_WAKE);
        }
      }
      break;
    case SC_MENU:
      if (on > 12000) backToFace(now, F_NEUTRAL);
      break;
    case SC_CLOCK:
      if (on > 10000) backToFace(now, F_NEUTRAL);
      break;
    case SC_WEATHER:
      if (on > 12000) backToFace(now, weatherFace());
      break;
    case SC_FOCUS:
      if (on > 8000) backToFace(now, F_NEUTRAL);
      break;
    case SC_CHARGE:
      if (on > 4000) backToFace(now, F_HAPPY);
      break;
    case SC_WIFI:
      if (on > 30000 && !apMode) backToFace(now, F_NEUTRAL);
      break;
    case SC_HELLO:
      if (on > 8000) backToFace(now, helloMorning ? F_HAPPY : F_ASLEEP);
      break;
    case SC_ALERT:
      if (on > alertTimeout) backToFace(now, F_NEUTRAL);
      break;
    case SC_GALLERY:
      if (on > 60000) backToFace(now, F_NEUTRAL);
      break;
    case SC_MUSIC:
      if (on > 20000 && now - musicCmdAt > 20000) backToFace(now, F_NEUTRAL);
      break;
    case SC_PONG:
      if ((long)(now - pongUntil) >= 0) backToFace(now, F_BORED);
      break;
    case SC_PAIR:
      if (pairUntil == 0 || (long)(now - pairUntil) >= 0) {
        if (pairCode[0] != '\0') showAlert(AL_INFO, "Pairing ended", "No phone used the code. Ask the app for a new one.", now);
        else backToFace(now, F_NEUTRAL);
        memset(pairCode, 0, sizeof(pairCode));
        pairUntil = 0;
      }
      break;
    case SC_CALL:
      if (!callMuted && (long)(now - nextRing) >= 0) {
        playSound(S_RING);
        nextRing = now + 2000;
      }
      if (on > 45000) backToFace(now, F_NEUTRAL);
      break;
    case SC_GAME:
      if (gState == GS_READY && on > 30000) backToFace(now, F_NEUTRAL);
      if (gState == GS_OVER && now - gStateAt > 20000) backToFace(now, F_NEUTRAL);
      if (gState == GS_OVER && gNewBest && now - gStateAt < 100) showFace(F_CELEBRATE, 3000, now);
      break;
    default:
      break;
  }
}

// ===================== MENU =========================================

void openMenu(uint8_t id, unsigned long now) {
  menuId = id;
  menuCount = 0;
  switch (id) {
    case MENU_GAMES:
      menuItems[menuCount++] = MI_G_JUMP;
      menuItems[menuCount++] = MI_G_FLAPPY;
      menuItems[menuCount++] = MI_G_CATCH;
      menuItems[menuCount++] = MI_G_REACT;
      menuItems[menuCount++] = MI_G_SIMON;
      menuItems[menuCount++] = MI_G_COPY;
      menuItems[menuCount++] = MI_G_RHYTHM;
      menuItems[menuCount++] = MI_G_RPS;
      break;
    case MENU_PHONE:
      menuItems[menuCount++] = MI_P_APP;
      menuItems[menuCount++] = MI_P_PAIR;
      menuItems[menuCount++] = MI_P_SETUP;
      menuItems[menuCount++] = MI_P_UNPAIR;
      break;
    case MENU_TEST:
      for (uint8_t i = MI_T_MSG; i <= MI_T_NIGHT; i++) menuItems[menuCount++] = i;
      break;
    default:
      menuItems[menuCount++] = MI_CLOCK;
      menuItems[menuCount++] = MI_MUSIC;
      menuItems[menuCount++] = MI_WEATHER;
      menuItems[menuCount++] = MI_GAMES;
      menuItems[menuCount++] = MI_FOCUS;
      menuItems[menuCount++] = MI_WATER;
      menuItems[menuCount++] = MI_STRETCH;
      menuItems[menuCount++] = MI_FACES;
      menuItems[menuCount++] = MI_HUG;
      menuItems[menuCount++] = MI_SOUND;
      menuItems[menuCount++] = MI_PHONE;
#if WOKWI
      menuItems[menuCount++] = MI_TEST;
#endif
      break;
  }
  menuItems[menuCount++] = MI_BACK;
  menuSel = 0;
  goScreen(SC_MENU, now);
  playSound(S_CLICK);
}

const char *menuTitle() {
  switch (menuId) {
    case MENU_GAMES: return "GAMES";
    case MENU_PHONE: return "PHONE & WI-FI";
    case MENU_TEST:  return "TEST";
    default:         return "MENU";
  }
}

const char *menuLabel(uint8_t item) {
  static char buf[22];
  switch (item) {
    case MI_CLOCK:    return "Clock";
    case MI_MUSIC:    return "Music";
    case MI_WEATHER:  return "Weather";
    case MI_GAMES:    return "Games >";
    case MI_FOCUS:    return focusRunning ? "Stop focus timer" : "Focus timer";
    case MI_WATER:    return "I drank water";
    case MI_STRETCH:  return "Stretch now";
    case MI_FACES:    return "46 expressions";
    case MI_HUG:      return "Send a hug";
    case MI_SOUND:    return cfg.volume ? "Sound: on" : "Sound: off";
    case MI_PHONE:    return "Phone & Wi-Fi >";
    case MI_TEST:     return "Test >";
    case MI_BACK:     return menuId == MENU_MAIN ? "Back to face" : "Back";
    case MI_G_JUMP:   snprintf(buf, sizeof(buf), "Jump    best %u", (unsigned)st.best[0]); return buf;
    case MI_G_FLAPPY: snprintf(buf, sizeof(buf), "Flappy  best %u", (unsigned)st.best[1]); return buf;
    case MI_G_CATCH:  snprintf(buf, sizeof(buf), "Catch   best %u", (unsigned)st.best[2]); return buf;
    case MI_G_REACT:
      if (st.best[3]) snprintf(buf, sizeof(buf), "Reaction %ums", (unsigned)st.best[3]);
      else snprintf(buf, sizeof(buf), "Reaction game");
      return buf;
    case MI_G_SIMON:  snprintf(buf, sizeof(buf), "Simon   best %u", (unsigned)best2[0]); return buf;
    case MI_G_COPY:   snprintf(buf, sizeof(buf), "Copy me best %u", (unsigned)best2[1]); return buf;
    case MI_G_RHYTHM: snprintf(buf, sizeof(buf), "Rhythm  best %u", (unsigned)best2[2]); return buf;
    case MI_G_RPS:    snprintf(buf, sizeof(buf), "Rock-paper %u", (unsigned)best2[3]); return buf;
    case MI_P_APP:    return "App address";
    case MI_P_PAIR:   return "Pair a phone";
    case MI_P_SETUP:  return "Wi-Fi setup";
    case MI_P_UNPAIR: return "Unpair all phones";
    case MI_T_MSG:    return "Test: message";
    case MI_T_CALL:   return "Test: call";
    case MI_T_MUSIC:  return "Test: music";
    case MI_T_WEATHER: return "Test: weather";
    case MI_T_ALARM:  return "Test: alarm";
    case MI_T_WATER:  return "Test: water";
    case MI_T_STRETCH: return "Test: stretch";
    case MI_T_HUG:    return "Test: hug";
    case MI_T_SPACE:  return "Test: space";
    case MI_T_MORNING: return "Test: morning";
    case MI_T_NIGHT:  return "Test: good night";
  }
  return "";
}

void startGame(uint8_t id, unsigned long now) {
  if (life != L_AWAKE) setLife(L_AWAKE, now);
  gameStart(id, now);
  goScreen(SC_GAME, now);
}

void chooseMenuItem(unsigned long now) {
  uint8_t m = menuItems[menuSel];
  playSound(S_CLICK);
  switch (m) {
    case MI_CLOCK:   goScreen(SC_CLOCK, now); break;
    case MI_MUSIC:   goScreen(SC_MUSIC, now); musicCmdAt = now; break;
    case MI_WEATHER: openWeather(now); break;
    case MI_GAMES:   openMenu(MENU_GAMES, now); break;
    case MI_PHONE:   openMenu(MENU_PHONE, now); break;
    case MI_TEST:    openMenu(MENU_TEST, now); break;
    case MI_FOCUS:
      if (focusRunning) {
        focusRunning = false;
        onBreak = false;
        showAlert(AL_INFO, "Focus stopped", "The focus timer was cancelled.", now);
      } else {
        startFocus(now);
      }
      break;
    case MI_WATER:   showReminder(0, now); reminderDoneTap(now); break;
    case MI_STRETCH: showReminder(1, now); break;
    case MI_FACES:   galleryIdx = 0; goScreen(SC_GALLERY, now); playFaceSound(0); break;
    case MI_HUG:     sendHug(now); break;
    case MI_SOUND:
      if (cfg.volume) { lastVolume = cfg.volume; cfg.volume = 0; stopSound(); }
      else { cfg.volume = lastVolume ? lastVolume : 2; playSound(S_PET); }
      markCfg(now);
      screenStart = now;
      break;
    case MI_BACK:
      if (menuId == MENU_MAIN) backToFace(now, F_HAPPY);
      else openMenu(MENU_MAIN, now);
      break;
    case MI_G_JUMP:   startGame(G_JUMP, now); break;
    case MI_G_FLAPPY: startGame(G_FLAPPY, now); break;
    case MI_G_CATCH:  startGame(G_CATCH, now); break;
    case MI_G_REACT:  startGame(G_REACT, now); break;
    case MI_G_SIMON:  startGame(G_SIMON, now); break;
    case MI_G_COPY:   startGame(G_COPY, now); break;
    case MI_G_RHYTHM: startGame(G_RHYTHM, now); break;
    case MI_G_RPS:    startGame(G_RPS, now); break;
    case MI_P_APP:    goScreen(SC_WIFI, now); break;
    case MI_P_PAIR:
#if MOMO_NO_WIFI
      showAlert(AL_INFO, "No Wi-Fi", "This build has no Wi-Fi.", now);
#else
      randomDigits(pairCode, 6);
      pairUntil = now + 120000;
      pairFails = 0;
      showPairScreen(now);
#endif
      break;
    case MI_P_SETUP:  startSetupHotspot(now); break;
    case MI_P_UNPAIR:
      memset(phones, 0, sizeof(phones));
      savePhones();
      showAlert(AL_INFO, "Unpaired", "All phones were removed. Pair again from the app.", now);
      break;
    default:
      testMenuAction(m, now);
      break;
  }
}

// ===================== FOCUS TIMER ==================================

void startFocus(unsigned long now) {
  focusRunning = true;
  onBreak = false;
#if WOKWI
  focusTotal = 60000;
#else
  focusTotal = (unsigned long)cfg.focusMin * 60000UL;
#endif
  focusEnd = now + focusTotal;
  goScreen(SC_FOCUS, now);
  Serial.println("Focus timer started");
}

void updateFocus(unsigned long now) {
  if (!focusRunning || (long)(now - focusEnd) < 0) return;
  if (!onBreak) {
    onBreak = true;
#if WOKWI
    focusTotal = 30000;
#else
    focusTotal = (unsigned long)cfg.breakMin * 60000UL;
#endif
    focusEnd = now + focusTotal;
    showAlert(AL_DONE, "Focus done!", "Great work! Take a short break now.", now);
    showFace(F_PROUD, 3000, now);
  } else {
    focusRunning = false;
    onBreak = false;
    showAlert(AL_DONE, "Break over", "Ready for another round? Hold for the menu.", now);
  }
}

// ===================== ALARMS =======================================

void startAlarm(int idx, unsigned long now) {
  ringingAlarm = (int8_t)idx;
  ringStart = now;
  nextRingSound = now;
  ringWasBusy = false;
  if (idx >= 0 && idx < 8) {
    ringSoundId = cfg.alarm[idx].sound;
    ringSnoozeMin = cfg.alarm[idx].snooze;
    copyText(ringLabel, sizeof(ringLabel), cfg.alarm[idx].label[0] ? cfg.alarm[idx].label : "Alarm");
  } else if (idx == 99) {
    // snoozed alarm: keep the saved settings
    ringSoundId = snoozeSound;
    ringSnoozeMin = snoozeMin;
    copyText(ringLabel, sizeof(ringLabel), snoozeLabel);
  } else {
    ringSoundId = 2;
    ringSnoozeMin = 5;
    copyText(ringLabel, sizeof(ringLabel), "Phone alarm");
  }
  stopSound();
  if (screen == SC_NOTE && noteIdx >= 0) { notePending[noteIdx] = true; noteIdx = -1; }   // shown again after the alarm
  goScreen(SC_ALARM, now);
  lastTouch = now;
  Serial.printf("Alarm! %s\n", ringLabel);
}

void updateAlarm(unsigned long now) {
  if (snoozeIdx >= 0 && (long)(now - snoozeAt) >= 0) {
    snoozeIdx = -1;
    startAlarm(99, now);
  }
  if (ringingAlarm < 0 || screen != SC_ALARM) return;
  if (soundBusy()) {
    ringWasBusy = true;
  } else {
    if (ringWasBusy) { nextRingSound = now + 700; ringWasBusy = false; }
    if ((long)(now - nextRingSound) >= 0) {
      uint8_t loud = (uint8_t)min(3UL, 1 + (now - ringStart) / 20000);   // gets louder
      playAlarm(ringSoundId, loud);
      ringWasBusy = true;
    }
  }
  if (now - ringStart > 180000) stopRinging(now, false);   // stop after 3 minutes
}

void snoozeRinging(unsigned long now) {
  if (ringingAlarm < 0) { backToFace(now, F_NEUTRAL); return; }
  snoozeIdx = ringingAlarm;
  snoozeSound = ringSoundId;
  snoozeMin = ringSnoozeMin;
  copyText(snoozeLabel, sizeof(snoozeLabel), ringLabel);
#if WOKWI
  snoozeAt = now + 20000;
#else
  snoozeAt = now + (unsigned long)snoozeMin * 60000UL;
#endif
  ringingAlarm = -1;
  stopSound();
  char msg[64];
#if WOKWI
  snprintf(msg, sizeof(msg), "I'll ring again in 20 seconds (simulator). Hold to stop.");
#else
  snprintf(msg, sizeof(msg), "I'll ring again in %u minute%s. Hold to stop next time.", (unsigned)snoozeMin, snoozeMin == 1 ? "" : "s");
#endif
  goScreen(SC_FACE, now);
  showAlert(AL_INFO, "Snoozed", msg, now);
  alertTimeout = 5000;
  showFace(F_SLEEPY, 3000, now);
}

void stopRinging(unsigned long now, bool byUser) {
  ringingAlarm = -1;
  snoozeIdx = -1;
  stopSound();
  if (byUser && isNight()) wokeEarlyDay = dayKey();   // stay awake after an early alarm
  if (byUser && !isNight()) {
    helloMorning = true;
    setLife(L_AWAKE, now);
    goScreen(SC_HELLO, now);
    showFace(F_HAPPY, 3000, now);
    playSound(S_DONE);
  } else {
    backToFace(now, F_NEUTRAL);
  }
  Serial.println(byUser ? "Alarm stopped" : "Alarm stopped by itself after 3 minutes");
}

// ===================== REMINDERS (WATER, STRETCH) ===================

void requestReminder(uint8_t kind, unsigned long now) {
  if (isNight()) return;
  bool busy = screen == SC_GAME || screen == SC_CALL || screen == SC_ALARM || screen == SC_PAIR ||
              screen == SC_WATER || screen == SC_STRETCH || screen == SC_NOTE || (focusRunning && !onBreak);
  if (busy) {
    remindPending[kind] = true;
    Serial.printf("Reminder (%s) waits until Momo is free\n", kind ? "stretch" : "water");
    return;
  }
  showReminder(kind, now);
}

void showReminder(uint8_t kind, unsigned long now) {
  remindKind = kind;
  remindDone = false;
  remindRings = 1;
  remindNextRing = now + 60000;
  remindPending[kind] = false;
  remindSnoozeUntil[kind] = 0;
  if (life != L_AWAKE) setLife(L_AWAKE, now);
  lastTouch = now;
  goScreen(kind ? SC_STRETCH : SC_WATER, now);
  playSound(kind ? S_STRETCH : S_WATER);
  Serial.printf("Reminder: %s\n", kind ? "stretch" : "drink water");
}

void reminderDoneTap(unsigned long now) {
  if (remindDone) { backToFace(now, F_HAPPY); return; }
  remindDone = true;
  remindDoneAt = now;
  if (remindKind == 0) st.drinks++;
  else st.stretches++;
  markStats(now);
  playSound(S_DONE);
  Serial.printf("%s done: %u today\n", remindKind ? "Stretch" : "Water", (unsigned)(remindKind ? st.stretches : st.drinks));
}

void reminderSnooze(unsigned long now) {
  remindSnoozeUntil[remindKind] = now + SNOOZE_REMIND_MS;
  remindPending[remindKind] = false;
  playSound(S_BACK);
  backToFace(now, F_SLEEPY);
  Serial.println("Reminder snoozed");
}

void updateReminders(unsigned long now) {
  if (screen == SC_WATER || screen == SC_STRETCH) {
    unsigned long on = now - screenStart;
    if (remindDone) {
      if (now - remindDoneAt > 3500) {
        uint8_t goal = remindKind ? cfg.stretch.goal : cfg.water.goal;
        uint8_t have = remindKind ? st.stretches : st.drinks;
        goScreen(SC_FACE, now);
        react(have >= goal ? F_CELEBRATE : F_PROUD, 2200, now);
      }
    } else {
      if (remindRings < 3 && (long)(now - remindNextRing) >= 0) {
        playSound(remindKind ? S_STRETCH : S_WATER);
        remindRings++;
        remindNextRing = now + 60000;
      }
      if (on > 300000) {   // nobody came: try again later
        Serial.println("Reminder missed");
        backToFace(now, F_DISAPPOINTED);
      }
    }
    return;
  }
  for (uint8_t k = 0; k < 2; k++) {
    if (remindSnoozeUntil[k] != 0 && (long)(now - remindSnoozeUntil[k]) >= 0) {
      remindSnoozeUntil[k] = 0;
      remindPending[k] = true;
    }
    if (remindPending[k] && screen == SC_FACE && !(focusRunning && !onBreak) && life != L_NIGHT) {
      showReminder(k, now);
      return;
    }
  }
}

// ===================== YOUR OWN REMINDERS ===========================
// Set in the Momo app (text, time, once on a date / every day / some days).
// Momo shows the text and rings until you tap. Hold = again in 5 minutes.

void startNote(uint8_t i, unsigned long now) {
  if (i >= MAX_NOTES || !notes[i].used) return;
  if (screen == SC_ALARM || screen == SC_CALL || (screen == SC_NOTE && noteIdx >= 0 && noteIdx != (int8_t)i)) {
    notePending[i] = true;   // shown when Momo is free
    Serial.printf("Your reminder waits: %s\n", notes[i].text);
    return;
  }
  notePending[i] = false;
  noteSnoozeUntil[i] = 0;
  noteIdx = (int8_t)i;
  noteStart = now;
  noteNextRing = now;
  noteRingWasBusy = false;
  stopSound();
  if (life == L_BORED || life == L_NAP) setLife(L_AWAKE, now);
  lastTouch = now;
  goScreen(SC_NOTE, now);
  Serial.printf("Your reminder: %s\n", notes[i].text);
}

void noteDoneTap(unsigned long now) {
  stopSound();
  if (noteIdx >= 0) Serial.printf("Reminder done: %s\n", notes[noteIdx].text);
  noteIdx = -1;
  goScreen(SC_FACE, now);
  react(F_PROUD, 2200, now);
}

void noteSnooze(unsigned long now) {
  stopSound();
  if (noteIdx >= 0) {
    noteSnoozeUntil[noteIdx] = now + NOTE_SNOOZE_MS;
    Serial.printf("Your reminder snoozed: %s\n", notes[noteIdx].text);
  }
  noteIdx = -1;
  goScreen(SC_FACE, now);
#if WOKWI
  showAlert(AL_INFO, "Later", "I'll remind you again in 30 seconds (simulator).", now);
#else
  showAlert(AL_INFO, "Later", "I'll remind you again in 5 minutes.", now);
#endif
  alertTimeout = 4000;
  showFace(F_SLEEPY, 3000, now);
}

void updateNotes(unsigned long now) {
  for (uint8_t i = 0; i < MAX_NOTES; i++) {
    if (noteSnoozeUntil[i] != 0 && (long)(now - noteSnoozeUntil[i]) >= 0) {
      noteSnoozeUntil[i] = 0;
      notePending[i] = true;
    }
  }
  if (screen != SC_NOTE && noteIdx >= 0) {   // something else took the screen: show it again later
    notePending[noteIdx] = true;
    noteIdx = -1;
  }
  if (screen != SC_NOTE) {
    if (screen == SC_ALARM || screen == SC_CALL || screen == SC_GAME || screen == SC_PAIR) return;
    for (uint8_t i = 0; i < MAX_NOTES; i++) if (notePending[i]) { startNote(i, now); break; }
    return;
  }
  unsigned long on = now - noteStart;
  if (on < 180000) {                       // ring for 3 minutes
    if (soundBusy()) {
      noteRingWasBusy = true;
    } else {
      if (noteRingWasBusy) { noteNextRing = now + 4000; noteRingWasBusy = false; }
      if ((long)(now - noteNextRing) >= 0) { playMyReminder(); noteRingWasBusy = true; }
    }
  } else if (on > 1800000) {               // nobody came for 30 minutes
    if (noteIdx >= 0) Serial.printf("Reminder not seen: %s\n", notes[noteIdx].text);
    noteIdx = -1;
    backToFace(now, F_DISAPPOINTED);
  }
}

// Runs once every new minute: alarms, reminders, new day, chime, affection
void checkSchedule(unsigned long now) {
  if (!haveTime) return;
  int m = nowMinutes();
  if (m == lastCheckedMinute) return;
  lastCheckedMinute = m;
  uint16_t dk = dayKey();
  if (st.dayKey != dk) {
    st.dayKey = dk;
    st.drinks = 0;
    st.stretches = 0;
    markStats(now);
  }
  for (int i = 0; i < 16; i++) {
    if (alarmDue(i, m) && alarmFiredMin[i] != (uint16_t)(m + 1)) {
      alarmFiredMin[i] = (uint16_t)(m + 1);
      if (i < 8 && cfg.alarm[i].days == 0) { cfg.alarm[i].on = false; markCfg(now); }   // "once" alarm
      startAlarm(i, now);
    }
  }
  for (int i = 0; i < MAX_NOTES; i++) {
    if (noteDue(i, m) && noteFiredMin[i] != (uint16_t)(m + 1)) {
      noteFiredMin[i] = (uint16_t)(m + 1);
      if (notes[i].days == 0) { notes[i].on = false; saveNotes(); }   // one-time reminder: done after it rings
      startNote((uint8_t)i, now);
    }
  }
  if (reminderDue(cfg.water, m) && remindFiredMin[0] != (uint16_t)(m + 1)) {
    remindFiredMin[0] = (uint16_t)(m + 1);
    requestReminder(0, now);
  }
  if (reminderDue(cfg.stretch, m) && remindFiredMin[1] != (uint16_t)(m + 1)) {
    remindFiredMin[1] = (uint16_t)(m + 1);
    requestReminder(1, now);
  }
  if (cfg.chime && nowTm.tm_min == 0 && life == L_AWAKE && screen == SC_FACE) {
    playSound(S_CHIME);
    showFace(F_HAPPY, 1500, now);
  }
  // Momo misses you a little after an hour alone (not while sleeping)
  if (life != L_NIGHT && now - lastTouch > 3600000UL && now - lastAffectionDecay > 3600000UL && st.affection > 0) {
    lastAffectionDecay = now;
    st.affection--;
    markStats(now);
  }
}

// ===================== MUSIC ========================================

const char *FAKE_SONGS[4][2] = {
  {"Sunny Morning", "Momo Band"}, {"Chai Break Beats", "Desk Groove"},
  {"Night Drive", "The Little Bots"}, {"Happy Circuits", "Momo Band"}
};
uint8_t fakeSong = 0;

void musicCommand(int8_t dir, unsigned long now) {
  musicCmdShown = dir == 0 ? 2 : dir;
  musicCmdAt = now;
  lastTouch = now;
#if WOKWI
  if (!musicKnown) { musicKnown = true; fakeSong = 0; }
  if (dir == 0) musicPlaying = !musicPlaying;
  else { fakeSong = (uint8_t)((fakeSong + (dir > 0 ? 1 : 3)) % 4); musicPlaying = true; }
  copyText(musicTitle, sizeof(musicTitle), FAKE_SONGS[fakeSong][0]);
  copyText(musicArtist, sizeof(musicArtist), FAKE_SONGS[fakeSong][1]);
#else
  if (dir == 0) { watch.musicControl(MUSIC_TOGGLE); musicPlaying = !musicPlaying; }
  else if (dir > 0) watch.musicControl(MUSIC_NEXT);
  else watch.musicControl(MUSIC_PREVIOUS);
#endif
  Serial.printf("Music: %s\n", dir == 0 ? (musicPlaying ? "play" : "pause") : dir > 0 ? "next" : "previous");
}

void musicInfoArrived(unsigned long now) {
  char oldTitle[48];
  copyText(oldTitle, sizeof(oldTitle), musicTitle);
#if !WOKWI
  MusicInfo &mi = watch.getMusicInfo();
  copyClean(musicTitle, sizeof(musicTitle), mi.title.c_str());
  copyClean(musicArtist, sizeof(musicArtist), mi.artist.c_str());
  if (musicTitle[0] == '\0') copyClean(musicTitle, sizeof(musicTitle), mi.appName.c_str());
  musicPlaying = mi.state == 1;
#endif
  musicKnown = true;
  bool newSong = strcmp(oldTitle, musicTitle) != 0 && musicTitle[0] != '\0';
  if (newSong) {
    Serial.printf("Now playing: %s - %s\n", musicTitle, musicArtist);
    if (screen == SC_FACE && life != L_NIGHT) {
      if (life != L_AWAKE) setLife(L_AWAKE, now);
      goScreen(SC_MUSIC, now);
      musicCmdAt = now;
    }
  }
  musicSeenAt = now;
}

// ===================== WEATHER ======================================

void weatherArrived(unsigned long now) {
  (void)now;
  Serial.printf("Weather: %s %dC (%d/%d) %s\n", wx.city, wx.temp, wx.high, wx.low, WX_NAMES[wx.icon % 8]);
}

// ===================== HUGS (ESP-NOW, real robot only) ==============

struct HugPacket { char magic[4]; uint8_t type; char name[16]; };
const uint8_t BROADCAST_MAC[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

#if !WOKWI && !MOMO_NO_WIFI
#if ESP_ARDUINO_VERSION_MAJOR >= 3
void onEspNowData(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
  (void)info;
#else
void onEspNowData(const uint8_t *mac, const uint8_t *data, int len) {
  (void)mac;
#endif
  if (len != (int)sizeof(HugPacket)) return;
  HugPacket p;
  memcpy(&p, data, sizeof(p));
  if (memcmp(p.magic, "MOMO", 4) != 0 || p.type != 1) return;
  p.name[sizeof(p.name) - 1] = '\0';
  copyClean(evHugFrom, sizeof(evHugFrom), p.name);
  evHug = true;
}
#endif

void startHugs() {
#if !WOKWI && !MOMO_NO_WIFI
  if (!cfg.hugsOn) return;
  if (wifiSsid[0] == '\0') esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);   // all Momos meet on channel 1
  if (esp_now_init() != ESP_OK) {
    Serial.println("Hugs: ESP-NOW did not start");
    return;
  }
  esp_now_register_recv_cb(onEspNowData);
  esp_now_peer_info_t peer;
  memset(&peer, 0, sizeof(peer));
  memcpy(peer.peer_addr, BROADCAST_MAC, 6);
  peer.channel = 0;
  peer.ifidx = WIFI_IF_STA;
  peer.encrypt = false;
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Hugs: could not add broadcast peer");
    return;
  }
  hugsReady = true;
  Serial.println("Hugs: ready (ESP-NOW)");
#else
  Serial.println("Hugs: off in the simulator (try Test: hug in the menu)");
#endif
}

void sendHug(unsigned long now) {
#if !WOKWI && !MOMO_NO_WIFI
  if (!hugsReady) {
    showAlert(AL_INFO, "Hugs are off", "Turn on hugs in the Momo app.", now);
    return;
  }
  HugPacket p;
  memset(&p, 0, sizeof(p));
  memcpy(p.magic, "MOMO", 4);
  p.type = 1;
  strncpy(p.name, cfg.name, sizeof(p.name) - 1);
  esp_err_t r = esp_now_send(BROADCAST_MAC, (const uint8_t *)&p, sizeof(p));
  showAlert(AL_HUG, "Hug sent!", r == ESP_OK ? "Every Momo nearby will feel it." : "Could not send. Please try again.", now);
#else
  showAlert(AL_INFO, "Hugs are off", "Hugs need two real Momos. Try 'Test: hug' in the menu.", now);
#endif
}

// ===================== PHONE (BLUETOOTH, CHRONOS APP) ===============

#if !WOKWI
void onPhoneConnection(bool connected) { evConn = connected ? 1 : 2; }

void onPhoneNotification(Notification n) {
  copyClean(evApp, sizeof(evApp), n.app.c_str());
  String body = n.message;
  if (n.title.length() > 0) body = n.title + ": " + n.message;
  copyClean(evText, sizeof(evText), body.c_str());
  evNotif = true;
}

void onPhoneRinger(String caller, bool ringing) {
  if (ringing) {
    copyClean(evCaller, sizeof(evCaller), caller.c_str());
    evCall = 1;
  } else {
    evCall = 2;
  }
}

void onPhoneConfig(Config config, uint32_t a, uint32_t b) {
  switch (config) {
    case CF_TIME:    if (a == 1) evTimeSet = true; break;     // phone has set the time
    case CF_FIND:    evFind = true; break;                    // "find my watch"
    case CF_WEATHER: evWeather = true; break;
    case CF_MUSIC:   evMusic = true; break;
    case CF_WATER:   evCfgA[0] = a; evCfgB[0] = b; evCfgMask |= 1; break;
    case CF_SED:     evCfgA[1] = a; evCfgB[1] = b; evCfgMask |= 2; break;
    case CF_QUIET:   evCfgA[2] = a; evCfgB[2] = b; evCfgMask |= 4; break;
    case CF_SLEEP:   evCfgA[3] = a; evCfgB[3] = b; evCfgMask |= 8; break;
    default: break;
  }
}
#endif

// Water / stretch / quiet / sleep settings sent from the Chronos app
void applyPhoneSettings(unsigned long now) {
  uint8_t mask = evCfgMask;
  if (mask == 0) return;
  evCfgMask = 0;
  for (int i = 0; i < 4; i++) {
    if (!(mask & (1 << i))) continue;
    uint32_t a = evCfgA[i], b = evCfgB[i];
    HM from = hm((uint8_t)(b >> 24), (uint8_t)((b >> 16) & 0xFF));
    HM to = hm((uint8_t)((b >> 8) & 0xFF), (uint8_t)(b & 0xFF));
    if (from.h > 23 || from.m > 59 || to.h > 23 || to.m > 59) continue;
    if (i < 2) {
      ReminderCfg &r = i == 0 ? cfg.water : cfg.stretch;
      r.every = (a & 0xFFFF) != 0;
      uint16_t every = (uint16_t)(a >> 16);
      if (every >= 10) r.everyMin = every;
      r.from = from;
      r.to = to;
      if (r.every) r.on = true;
    } else if (i == 2) {
      cfg.quietOn = a != 0;
      cfg.quietFrom = from;
      cfg.quietTo = to;
    } else if (a != 0) {
      cfg.sleep = from;
      cfg.wake = to;
    }
  }
  markCfg(now);
  Serial.println("Settings received from the Chronos app");
}

void handleEvents(unsigned long now) {
  if (evNotif) {
    evNotif = false;
    showAlert(AL_PHONE, evApp[0] != '\0' ? evApp : "Phone", evText, now);
  }
  if (evCall == 1) {
    evCall = 0;
    if (screen != SC_ALARM) startCall(evCaller, now);
  } else if (evCall == 2) {
    evCall = 0;
    if (screen == SC_CALL) backToFace(now, F_NEUTRAL);
  }
  if (evHug) {
    evHug = false;
    char msg[64];
    snprintf(msg, sizeof(msg), "From %s. Send one back from the menu!", evHugFrom);
    showAlert(AL_HUG, "Hug received!", msg, now);
  }
  if (evFind) {
    evFind = false;
    showAlert(AL_FIND, "I'm here!", "Your phone is looking for me.", now);
  }
  if (evConn == 1) {
    evConn = 0;
    Serial.println("Phone connected");
    if (screen == SC_FACE && life == L_AWAKE) react(F_HAPPY, 1500, now);
  } else if (evConn == 2) {
    evConn = 0;
    Serial.println("Phone disconnected");
  }
  if (evMusic) {
    evMusic = false;
    musicInfoArrived(now);
  }
  applyPhoneSettings(now);
#if !WOKWI
  if (evTimeSet) {
    evTimeSet = false;
    struct tm t = watch.getTimeStruct();
    Serial.printf("Time from phone: %02d:%02d\n", t.tm_hour, t.tm_min);
    if (rtcOk) writeRtc(t);
    nextTimeRead = 0;
  }
  if (evWeather) {
    evWeather = false;
    int n = watch.getWeatherCount();
    if (n > 0) {
      Weather &w0 = watch.getWeatherAt(0);
      wx.temp = w0.temp;
      wx.high = w0.high;
      wx.low = w0.low;
      wx.icon = chronosToIcon(w0.icon);
      wx.dayCount = 0;
      for (int i = 0; i < n && i < 3; i++) {
        Weather &wi = watch.getWeatherAt(i);
        wx.days[i].icon = chronosToIcon(wi.icon);
        wx.days[i].hi = (int8_t)wi.high;
        wx.days[i].lo = (int8_t)wi.low;
        wx.dayCount++;
      }
      copyClean(wx.city, sizeof(wx.city), watch.getWeatherCity().c_str());
      if (wx.city[0] == '\0') copyText(wx.city, sizeof(wx.city), cfg.city);
      wx.valid = true;
      wx.fromPhone = true;
      wx.at = now;
      weatherArrived(now);
    }
  }
#endif
}

// ===================== THE MOMO APP =================================

void settingsChanged(unsigned long now) {
  if (cfg.volume == 0) stopSound();
  lastCheckedMinute = -1;     // re-check alarms and reminders with the new times
  if (nowMinutes() >= 0) {    // do not ring for the current minute again
    int m = nowMinutes();
    for (int i = 0; i < 16; i++) if (alarmDue(i, m)) alarmFiredMin[i] = (uint16_t)(m + 1);
    for (int i = 0; i < MAX_NOTES; i++) if (noteDue(i, m)) noteFiredMin[i] = (uint16_t)(m + 1);
    if (reminderDue(cfg.water, m)) remindFiredMin[0] = (uint16_t)(m + 1);
    if (reminderDue(cfg.stretch, m)) remindFiredMin[1] = (uint16_t)(m + 1);
  }
  (void)now;
  Serial.println("Settings saved from the Momo app");
}

void wifiJoined(unsigned long now) {
  if (screen == SC_WIFI || apMode) {
    char msg[64];
    snprintf(msg, sizeof(msg), "Momo app: http://%s  (or momo.local)", ipText);
    showAlert(AL_WIFI, "Wi-Fi connected", msg, now);
    alertTimeout = 30000;
  }
}

bool appAction(const char *what, JsonObjectConst args, unsigned long now) {
  if (strcmp(what, "face") == 0) {
    int id = args["id"] | -1;
    if (id < 0 || id >= F_COUNT) return false;
    if (screen != SC_FACE && screen != SC_GALLERY) goScreen(SC_FACE, now);
    if (life != L_AWAKE && life != L_NIGHT) setLife(L_AWAKE, now);
    react((uint8_t)id, 6000, now);
    lastTouch = now;
    return true;
  }
  if (strcmp(what, "note") == 0) {   // try one of your reminders now
    int i = args["index"] | -1;
    if (i < 0 || i >= MAX_NOTES || !notes[i].used) return false;
    startNote((uint8_t)i, now);
    return true;
  }
  if (strcmp(what, "sound") == 0) {
    const char *name = args["name"] | "";
    for (int i = 0; i < S_COUNT; i++) {
      if (strcmp(name, SOUND_NAMES[i]) == 0) {
        if (SOUNDS[i].cat == C_ALARM && i != S_TUNE) playAlarm((uint8_t)(i - S_ALARM_CLASSIC), max(1, (int)cfg.volume));
        else if (i == S_TUNE) playAlarm(3, max(1, (int)cfg.volume));
        else playSound((Snd)i);
        return true;
      }
    }
    return false;
  }
  if (strcmp(what, "tune") == 0) {   // try a tune before saving it
    const char *notes = args["notes"] | cfg.tune;
    Note tmp[TUNE_MAX];
    uint8_t n = parseTune(notes, tmp, TUNE_MAX);
    if (n == 0) return false;
    startNotes(tmp, n, true, volumeDuty(max(1, (int)cfg.volume)));
    return true;
  }
  if (strcmp(what, "pet") == 0) { if (screen != SC_FACE) goScreen(SC_FACE, now); if (life != L_AWAKE) setLife(L_AWAKE, now); petMomo(now); lastTouch = now; return true; }
  if (strcmp(what, "water") == 0) { showReminder(0, now); reminderDoneTap(now); return true; }
  if (strcmp(what, "remindWater") == 0) { showReminder(0, now); return true; }
  if (strcmp(what, "stretch") == 0) { showReminder(1, now); return true; }
  if (strcmp(what, "alarm") == 0) { int i = args["index"] | 0; startAlarm(constrain(i, 0, 7), now); return true; }
  if (strcmp(what, "weather") == 0) {
#if !MOMO_NO_WIFI
    weatherAt = now;
#endif
    openWeather(now);
    return true;
  }
  if (strcmp(what, "weatherNow") == 0) {   // fetch the weather now (it shows on Momo when it arrives)
#if !MOMO_NO_WIFI
    weatherAt = now;
    weatherForced = true;
#endif
    return true;
  }
  if (strcmp(what, "back") == 0) { onBack(now); return true; }
  if (strcmp(what, "find") == 0) { showAlert(AL_FIND, "I'm here!", "Your phone is looking for me.", now); return true; }
  if (strcmp(what, "game") == 0) { int g = args["id"] | 0; startGame((uint8_t)constrain(g, 0, G_COUNT - 1), now); return true; }
  if (strcmp(what, "music") == 0) {
    const char *c = args["cmd"] | "toggle";
    musicCommand(strcmp(c, "next") == 0 ? 1 : strcmp(c, "prev") == 0 ? -1 : 0, now);
    if (screen == SC_FACE) goScreen(SC_MUSIC, now);
    return true;
  }
  if (strcmp(what, "scoresReset") == 0) {
    for (int i = 0; i < 4; i++) { st.best[i] = 0; best2[i] = 0; }
    markStats(now);
    return true;
  }
  if (strcmp(what, "wifiSetup") == 0) { startSetupHotspot(now); return true; }
  return false;
}

// ===================== SIMULATOR TESTS (Wokwi) ======================

void testMenuAction(uint8_t item, unsigned long now) {
  switch (item) {
    case MI_T_MSG:
      copyClean(evApp, sizeof(evApp), "WhatsApp");
      copyClean(evText, sizeof(evText), "Amma: Lunch is ready, come home soon!");
      evNotif = true;
      break;
    case MI_T_CALL:
      copyClean(evCaller, sizeof(evCaller), "Amma");
      evCall = 1;
      break;
    case MI_T_MUSIC:   // pretend the phone started a song
      fakeSong = 0;
      musicKnown = true;
      musicPlaying = true;
      copyText(musicTitle, sizeof(musicTitle), FAKE_SONGS[0][0]);
      copyText(musicArtist, sizeof(musicArtist), FAKE_SONGS[0][1]);
      Serial.printf("Now playing: %s - %s\n", musicTitle, musicArtist);
      goScreen(SC_MUSIC, now);
      musicCmdAt = now;
      break;
    case MI_T_WEATHER:
      wx.valid = true; wx.fromPhone = true; wx.at = now;
      copyText(wx.city, sizeof(wx.city), cfg.city);
      wx.temp = 31; wx.high = 34; wx.low = 24; wx.icon = WX_SUN; wx.dayCount = 3;
      wx.days[0] = {WX_SUN, 34, 24}; wx.days[1] = {WX_RAIN, 29, 23}; wx.days[2] = {WX_STORM, 27, 22};
      weatherArrived(now);
      openWeather(now);
      break;
    case MI_T_ALARM:
      startAlarm(0, now);
      copyText(ringLabel, sizeof(ringLabel), "Test alarm");
      break;
    case MI_T_WATER:   showReminder(0, now); break;
    case MI_T_STRETCH: showReminder(1, now); break;
    case MI_T_HUG:
      copyClean(evHugFrom, sizeof(evHugFrom), "Momo 2");
      evHug = true;
      break;
    case MI_T_SPACE:
      showAlert(AL_SPACE, "Space station!", "The ISS is flying over your area right now. Look up!", now);
      break;
    case MI_T_MORNING: wakeUpMorning(now); goScreen(SC_HELLO, now); break;
    case MI_T_NIGHT:
      helloMorning = false;
      goScreen(SC_HELLO, now);
      nightSoundOk = true;
      playSound(S_GOODNIGHT);
      nightSoundOk = false;
      break;
    default: backToFace(now, F_NEUTRAL); break;
  }
}

// ===================== SETUP AND LOOP ===============================

#if WOKWI
// Runs before setup(), as soon as the program itself starts.
void momoEarlyHello() {
  esp_rom_printf("\r\n[Momo] program started\r\n");
}

const char *resetName(esp_reset_reason_t r) {
  switch (r) {
    case ESP_RST_POWERON:  return "power on (normal)";
    case ESP_RST_SW:       return "software restart";
    case ESP_RST_PANIC:    return "crash";
    case ESP_RST_INT_WDT:  return "interrupt watchdog";
    case ESP_RST_TASK_WDT: return "task watchdog";
    case ESP_RST_WDT:      return "watchdog";
    case ESP_RST_BROWNOUT: return "brownout (power dip)";
    default:               return "other";
  }
}
#endif

void setup() {
  Serial.begin(115200);
#if WOKWI
  Serial.printf("[Momo] setup started - last restart was: %s\n", resetName(esp_reset_reason()));
  Serial.flush();
#endif
  delay(300);
  Serial.println();
  Serial.printf("Hello! I am Momo. (version %s)\n", MOMO_VERSION);

  pinMode(PIN_TOUCH, INPUT_PULLDOWN);
  pinMode(PIN_BUZZER, OUTPUT);
  startPower();
  memset(&wx, 0, sizeof(wx));

  Wire.begin(PIN_SDA, PIN_SCL);
  Wire.setClock(400000);
  STEP("I2C bus started");

#if SCREEN_IS_SSD1306
  displayOk = display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);
#else
  displayOk = display.begin(OLED_ADDR, true);
#endif
  STEP("screen started");
  if (FLIP_SCREEN) display.setRotation(2);
  display.setTextWrap(false);
  display.setTextColor(INK);

  rtcOk = i2cPresent(RTC_ADDR);
  mpuOk = i2cPresent(MPU_ADDR) && mpuInit();
  Serial.printf("Screen: %s | Clock module: %s | Motion sensor: %s\n",
                displayOk ? "OK" : "NOT FOUND", rtcOk ? "OK" : "NOT FOUND", mpuOk ? "OK" : "NOT FOUND");

  loadStore();
  if (x22Fresh) {   // first start of 2.2: save the new settings (hero look and sounds)
    markCfg(millis());
    Serial.println("Momo 2.2: hero look and sounds are on (change them in the app)");
  }
  lastVolume = cfg.volume ? cfg.volume : 2;
  Serial.printf("Personality: %u pets, affection %u/100\n", (unsigned)st.pets, (unsigned)st.affection);
  STEP("settings loaded");

  goScreen(SC_BOOT, millis());
  drawFrame(millis());
  STEP("start-up screen drawn");

  refreshTime(millis());
  if (haveTime) {
    char t[12];
    timeText(nowTm.tm_hour, nowTm.tm_min, t, sizeof(t), true);
    Serial.printf("Time: %s\n", t);
  } else {
    Serial.println("Clock not set yet (connect the phone or Wi-Fi once)");
  }
  setFace(F_ASLEEP);
  if (isNight()) setLife(L_NIGHT, millis());
  else setFace(F_NEUTRAL);

#if !WOKWI
  watch.setConnectionCallback(onPhoneConnection);
  watch.setNotificationCallback(onPhoneNotification);
  watch.setRingerCallback(onPhoneRinger);
  watch.setConfigurationCallback(onPhoneConfig);
  watch.begin();
  Serial.println("Bluetooth: ready - connect from the Chronos app");
#endif

  STEP("starting Wi-Fi");
  startWifi(millis());
  STEP("starting hugs");
  startHugs();

  unsigned long now = millis();
  lastTouch = now;
  nextIdle = now + 15000;
  goScreen(SC_BOOT, now);
  STEP("reading the battery");
  readBattery(now);
  if (battPresent) Serial.printf("Battery: %u%% (%u mV)%s\n", (unsigned)battPct, (unsigned)battMv, charging ? ", charging" : "");
  STEP("setup finished - Momo is running");
}

void loop() {
  if (asleep) return;   // desktop test rig only: the real chip restarts when it wakes
  unsigned long now = millis();
#if !WOKWI
  watch.loop();
#endif
  handleEvents(now);
  readInputs(now);
  readBattery(now);
  if (asleep) return;
  readMotion(now);
  updateSound(now);
  refreshTime(now);
  checkSchedule(now);
  updateAlarm(now);
  updateReminders(now);
  updateNotes(now);
  updateFocus(now);
  updateLife(now);
  updateScreens(now);
  if (screen == SC_GAME) updateGame(now, tiltX);
  updateFace(now);
  updateNetwork(now);
  saveStore(now, false);
  if (now - lastFrame >= FRAME_TIME) {
    lastFrame = now;
    drawFrame(now);
  }
}
