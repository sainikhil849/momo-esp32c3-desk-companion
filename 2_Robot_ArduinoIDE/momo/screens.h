// =====================================================================
//  screens.h  -  everything Momo draws besides the face itself
// =====================================================================

void centerText(const char *s, int y, uint8_t size) {
  int16_t x1, y1;
  uint16_t w, h;
  display.setTextSize(size);
  display.setTextColor(INK);
  display.getTextBounds(s, 0, 0, &x1, &y1, &w, &h);
  display.setCursor((128 - (int)w) / 2, y);
  display.print(s);
}

// Copy text, keeping only characters the screen can show
void copyClean(char *dst, size_t size, const char *src) {
  size_t j = 0;
  bool lastWasSpace = false;
  for (size_t i = 0; src != nullptr && src[i] != '\0' && j + 1 < size; i++) {
    unsigned char c = (unsigned char)src[i];
    if (c == '\n' || c == '\r' || c == '\t') c = ' ';
    if (c >= 0x80) {                       // other alphabets: skip the whole UTF-8 letter
      while ((unsigned char)src[i + 1] >= 0x80 && (unsigned char)src[i + 1] < 0xC0) i++;
      continue;
    }
    if (c < 32 || c > 126) continue;
    if (c == ' ' && (lastWasSpace || j == 0)) continue;
    dst[j++] = (char)c;
    lastWasSpace = (c == ' ');
  }
  while (j > 0 && dst[j - 1] == ' ') j--;
  dst[j] = '\0';
}

void copyText(char *dst, size_t size, const char *src) {
  if (size == 0) return;
  strncpy(dst, src ? src : "", size - 1);
  dst[size - 1] = '\0';
}

// Split text into lines of up to 21 characters (the screen width)
uint8_t wrapText(const char *text, char lines[][22], uint8_t maxLines) {
  uint8_t count = 0;
  const char *p = text;
  while (*p != '\0' && count < maxLines) {
    while (*p == ' ') p++;
    if (*p == '\0') break;
    size_t len = strlen(p);
    size_t take = len;
    if (len > 21) {
      size_t cut = 21;
      while (cut > 0 && p[cut] != ' ') cut--;
      take = (cut > 0) ? cut : 21;
    }
    memcpy(lines[count], p, take);
    lines[count][take] = '\0';
    count++;
    p += take;
  }
  return count;
}

// Text that scrolls sideways when it is too long
void marquee(const char *s, int y, uint8_t size, unsigned long t) {
  int cw = 6 * size;
  int w = (int)strlen(s) * cw;
  display.setTextSize(size);
  display.setTextColor(INK);
  display.setTextWrap(false);
  if (w <= 124) {
    display.setCursor((128 - w) / 2, y);
    display.print(s);
    return;
  }
  int span = w + 30;
  int off = (int)((t / 30) % span);
  display.setCursor(2 - off, y);
  display.print(s);
  display.setCursor(2 - off + span, y);
  display.print(s);
}

void progressBar(int x, int y, int w, int h, float frac) {
  frac = constrain(frac, 0.0f, 1.0f);
  display.drawRect(x, y, w, h, INK);
  display.fillRect(x + 2, y + 2, (int)((w - 4) * frac), h - 4, INK);
}

// ---------- small icons ----------
void iconBell(int x, int y, uint16_t c) {
  display.fillCircle(x + 3, y + 3, 3, c);
  display.fillRect(x, y + 3, 7, 3, c);
  display.drawFastHLine(x - 1, y + 6, 9, c);
  display.drawPixel(x + 3, y + 7, c);
}
void iconWifi(int x, int y, uint16_t c) {
  display.drawPixel(x + 3, y + 6, c);
  display.drawFastHLine(x + 2, y + 4, 3, c);
  display.drawFastHLine(x + 1, y + 2, 5, c);
  display.drawFastHLine(x, y, 7, c);
}
void iconPhone(int x, int y, uint16_t c) {
  display.drawRoundRect(x, y, 5, 8, 1, c);
  display.drawPixel(x + 2, y + 6, c);
}
void iconMute(int x, int y, uint16_t c) {
  display.fillRect(x, y + 2, 2, 3, c);
  display.fillTriangle(x + 2, y + 3, x + 4, y + 1, x + 4, y + 5, c);
  display.drawLine(x + 6, y + 1, x + 9, y + 5, c);
  display.drawLine(x + 9, y + 1, x + 6, y + 5, c);
}
void iconEnvelope(int x, int y, uint16_t c) {
  display.drawRect(x, y, 20, 14, c);
  display.drawLine(x, y, x + 10, y + 8, c);
  display.drawLine(x + 19, y, x + 10, y + 8, c);
}
void iconCloud(int x, int y, uint16_t c) {   // about 34 x 20
  display.fillCircle(x + 9, y + 12, 7, c);
  display.fillCircle(x + 19, y + 8, 9, c);
  display.fillCircle(x + 28, y + 12, 6, c);
  display.fillRect(x + 9, y + 12, 20, 8, c);
}

void drawStatusBar(unsigned long now) {
  int x = 1;
  if (phoneConnected()) { iconPhone(x, 0, INK); x += 8; }
  if (wifiConnected || apMode) { iconWifi(x, 0, INK); x += 10; }
  bool anyAlarm = false;
  for (int i = 0; i < 8; i++) anyAlarm |= cfg.alarm[i].on;
  if (anyAlarm) { iconBell(x + 1, 0, INK); x += 10; }
  if (musicPlaying) { noteShape(x + 1, -1, INK); x += 9; }
  if (cfg.volume == 0) { iconMute(x, 0, INK); x += 11; }
  if (battPresent) { drawBatteryIcon(x + 1, 0, now); x += 16; }
  if (focusRunning) {
    long left = (long)(focusEnd - now);
    if (left < 0) left = 0;
    char buf[12];
    snprintf(buf, sizeof(buf), "%02ld:%02ld", left / 60000, (left / 1000) % 60);
    display.setTextSize(1);
    display.setCursor(97, 0);
    display.print(buf);
  }
}

// ---------- screens ----------
void drawFaceScreen(unsigned long now) {
  drawEyes(now);
  if (life != L_NIGHT) drawStatusBar(now);
}

void drawBoot(unsigned long now) {
  if (heroLook()) { drawHeroBoot(now); return; }
  centerText(cfg.name, 6, 3);
  char parts[32];
  snprintf(parts, sizeof(parts), "Clock:%s Motion:%s", rtcOk ? "ok" : "--", mpuOk ? "ok" : "--");
  centerText(parts, 40, 1);
  centerText("v" MOMO_VERSION, 52, 1);
}

void drawMenu() {
  display.fillRect(0, 0, 128, 12, INK);
  display.setTextSize(1);
  display.setTextColor(ERASE);
  display.setCursor(3, 2);
  display.print(menuTitle());
  char pos[8];
  snprintf(pos, sizeof(pos), "%u/%u", (unsigned)(menuSel + 1), (unsigned)menuCount);
  display.setCursor(125 - 6 * (int)strlen(pos), 2);
  display.print(pos);
  display.setTextColor(INK);
  for (int row = -1; row <= 1; row++) {
    int i = (int)menuSel + row;
    if (i < 0 || i >= (int)menuCount) continue;
    int y = 28 + row * 13;
    if (row == 0) {
      display.fillRoundRect(2, y - 2, 124, 12, 3, INK);
      display.setTextColor(ERASE);
    }
    display.setCursor(8, y);
    display.print(menuLabel(menuItems[i]));
    display.setTextColor(INK);
  }
  display.setCursor(1, 55);
  display.print("Tap:> Hold:OK 2x:back");
}

void drawClock(unsigned long now) {
  (void)now;
  if (!haveTime) {
    centerText("No time yet", 14, 1);
    centerText("Connect the phone", 30, 1);
    centerText("or Wi-Fi once", 42, 1);
    return;
  }
  char buf[24];
  timeText(nowTm.tm_hour, nowTm.tm_min, buf, sizeof(buf), false);
  centerText(buf, 4, 3);
  char date[24];
  if (cfg.clock24) snprintf(date, sizeof(date), "%s %d %s", DAY_SHORT[nowTm.tm_wday], nowTm.tm_mday, MONTH_SHORT[nowTm.tm_mon]);
  else snprintf(date, sizeof(date), "%s  %s %d %s", nowTm.tm_hour < 12 ? "am" : "pm", DAY_SHORT[nowTm.tm_wday], nowTm.tm_mday, MONTH_SHORT[nowTm.tm_mon]);
  centerText(date, 30, 1);
  char al[20];
  if (nextAlarmText(al, sizeof(al))) {
    iconBell(4, 43, INK);
    display.setCursor(14, 43);
    display.print(al);
  }
  char w[20];
  snprintf(w, sizeof(w), "Water %u/%u", (unsigned)st.drinks, (unsigned)cfg.water.goal);
  display.setCursor(4, 54);
  display.print(w);
  if (wx.valid) {
    snprintf(w, sizeof(w), "%dC", wx.temp);
    display.setCursor(124 - (int)strlen(w) * 6, 54);
    display.print(w);
  }
}

void drawFocus(unsigned long now) {
  long left = (long)(focusEnd - now);
  if (left < 0) left = 0;
  centerText(onBreak ? "BREAK" : "FOCUS", 0, 1);
  char buf[16];
  snprintf(buf, sizeof(buf), "%02ld:%02ld", left / 60000, (left / 1000) % 60);
  centerText(buf, 14, 3);
  progressBar(8, 44, 112, 8, 1.0f - (float)left / (float)focusTotal);
  // little hourglass
  int sand = (int)((now / 300) % 6);
  display.drawLine(4, 12, 12, 12, INK); display.drawLine(4, 28, 12, 28, INK);
  display.drawLine(4, 12, 12, 28, INK); display.drawLine(12, 12, 4, 28, INK);
  display.fillTriangle(6, 14 + sand / 2, 10, 14 + sand / 2, 8, 19, INK);
  display.drawPixel(8, 21 + sand, INK);
  display.setTextSize(1);
  display.setCursor(10, 56);
  display.print("Hold: menu to stop");
}

void drawAlertIcon(AlertKind k, int x, int y, unsigned long now) {
  switch (k) {
    case AL_PHONE: iconEnvelope(x, y + 1, INK); break;
    case AL_HUG:   heartShape(x + 10, y + 3, 7, INK); break;
    case AL_SPACE: {
      display.drawCircle(x + 10, y + 8, 4, INK);
      display.drawLine(x, y + 11, x + 20, y + 5, INK);
      for (int i = 0; i < 3; i++) if ((now / 250 + i) % 2) display.drawPixel(x + 3 + i * 7, y + (i * 5) % 14, INK);
      break;
    }
    case AL_DONE:  starShape(x + 10, y + 8, 8, INK); break;
    case AL_FIND:  iconPhone(x + 7, y + 3, INK); display.drawCircle(x + 9, y + 7, 8 + (int)((now / 200) % 3), INK); break;
    case AL_PAIR:  display.drawRoundRect(x + 4, y + 7, 12, 9, 2, INK); display.drawCircle(x + 10, y + 6, 4, INK); break;
    case AL_WIFI:  iconWifi(x + 6, y + 4, INK); break;
    case AL_BATTERY:
      display.drawRect(x + 2, y + 4, 16, 9, INK); display.fillRect(x + 18, y + 6, 2, 5, INK);
      if ((now / 400) % 2) display.fillRect(x + 4, y + 6, 3, 5, INK);
      break;
    default:       display.drawCircle(x + 10, y + 8, 7, INK); display.drawFastVLine(x + 10, y + 4, 5, INK); display.drawPixel(x + 10, y + 11, INK); break;
  }
}

void drawAlert(unsigned long now) {
  unsigned long on = now - screenStart;
  if (alertKind == AL_PHONE && on < 600) {     // envelope flies in
    int x = 128 - (int)(on * 110 / 600);
    iconEnvelope(x, 24, INK);
    display.drawFastHLine(x + 22, 28, 8, INK);
    display.drawFastHLine(x + 24, 32, 10, INK);
    return;
  }
  drawAlertIcon(alertKind, 2, 0, now);
  display.setTextSize(1);
  display.setTextColor(INK);
  display.setCursor(26, 5);
  display.print(alertTitle);
  display.drawFastHLine(0, 18, 128, INK);
  uint8_t first = 0;
  if (alertLineCount > 4) {
    uint8_t pages = alertLineCount - 3;
    first = (uint8_t)((on / 2000) % pages);
  }
  for (uint8_t i = 0; i < 4 && first + i < alertLineCount; i++) {
    display.setCursor(1, 22 + i * 10);
    display.print(alertLines[first + i]);
  }
}

void drawCall(unsigned long now) {
  int shake = callMuted ? 0 : ((now / 60) % 2 ? 2 : -2);
  display.drawRoundRect(10 + shake, 6, 18, 30, 3, INK);
  display.fillRect(13 + shake, 10, 12, 18, INK);
  if (!callMuted && (now / 300) % 2) {
    display.drawCircle(19, 20, 18, INK);
    display.drawCircle(19, 20, 22, INK);
  }
  display.fillRect(40, 0, 88, 40, ERASE);
  display.setTextSize(1);
  display.setCursor(44, 6);
  display.print(callMuted ? "Muted call" : "Incoming call");
  display.setCursor(44, 22);
  char name[14];
  copyText(name, sizeof(name), callerName);
  display.print(name);
  display.setCursor(2, 44);
  display.print("Tap: quiet");
  display.setCursor(2, 55);
  display.print("Hold: close");
}

// ---------- weather ----------
void drawWeatherIcon(uint8_t icon, int x, int y, unsigned long t, bool small) {
  if (small) {   // 12x10 icons for the forecast row
    switch (icon) {
      case WX_SUN: display.drawCircle(x + 6, y + 5, 3, INK); for (int i = 0; i < 4; i++) display.drawPixel(x + 6 + (i % 2 ? 0 : (i ? 5 : -5)), y + 5 + (i % 2 ? (i == 1 ? -5 : 5) : 0), INK); break;
      case WX_RAIN: case WX_STORM: display.fillRoundRect(x, y, 12, 6, 3, INK); display.drawPixel(x + 3, y + 8, INK); display.drawPixel(x + 7, y + 9, INK); break;
      case WX_SNOW: display.fillRoundRect(x, y, 12, 6, 3, INK); display.drawPixel(x + 3, y + 8, INK); display.drawPixel(x + 8, y + 8, INK); display.drawPixel(x + 5, y + 9, INK); break;
      default: display.fillRoundRect(x, y + 2, 12, 7, 3, INK); break;
    }
    return;
  }
  switch (icon) {
    case WX_SUN: {
      int cx = x + 20, cy = y + 20;
      display.fillCircle(cx, cy, 8, INK);
      float a0 = (t % 4000) / 4000.0f * 6.2832f;
      for (int i = 0; i < 8; i++) {
        float a = a0 + i * 0.7854f;
        display.drawLine(cx + (int)(cosf(a) * 11), cy + (int)(sinf(a) * 11), cx + (int)(cosf(a) * 17), cy + (int)(sinf(a) * 17), INK);
      }
      break;
    }
    case WX_PARTLY: {
      display.fillCircle(x + 26, y + 12, 8, INK);
      float a0 = (t % 5000) / 5000.0f * 6.2832f;
      for (int i = 0; i < 6; i++) {
        float a = a0 + i * 1.047f;
        display.drawLine(x + 26 + (int)(cosf(a) * 11), y + 12 + (int)(sinf(a) * 11), x + 26 + (int)(cosf(a) * 15), y + 12 + (int)(sinf(a) * 15), INK);
      }
      int drift = (int)(sinf(t / 900.0f) * 2);
      display.fillCircle(x + 9 + drift, y + 26, 9, ERASE);
      display.fillRect(x + drift, y + 22, 34, 14, ERASE);
      iconCloud(x + drift, y + 16, INK);
      break;
    }
    case WX_CLOUD: {
      int drift = (int)(sinf(t / 900.0f) * 3);
      iconCloud(x + 2 + drift, y + 6, INK);
      iconCloud(x + 10 - drift, y + 16, INK);
      display.drawLine(x + 8 - drift, y + 30, x + 40 - drift, y + 30, ERASE);
      break;
    }
    case WX_RAIN: case WX_STORM: {
      iconCloud(x + 3, y + 2, INK);
      for (int i = 0; i < 6; i++) {
        int dx = x + 6 + i * 6;
        int dy = y + 24 + (int)((t / 40 + i * 7) % 14);
        display.drawLine(dx, dy, dx - 2, dy + 4, INK);
      }
      if (icon == WX_STORM && (t / 150) % 8 < 2) {
        display.fillTriangle(x + 20, y + 20, x + 14, y + 32, x + 22, y + 30, INK);
        display.fillTriangle(x + 22, y + 30, x + 16, y + 42, x + 26, y + 28, INK);
      }
      break;
    }
    case WX_SNOW: {
      iconCloud(x + 3, y + 2, INK);
      for (int i = 0; i < 6; i++) {
        int fx = x + 5 + i * 6 + (int)(sinf((t + i * 300) / 400.0f) * 2);
        int fy = y + 24 + (int)((t / 70 + i * 5) % 16);
        display.drawPixel(fx, fy, INK); display.drawPixel(fx - 1, fy, INK); display.drawPixel(fx + 1, fy, INK);
        display.drawPixel(fx, fy - 1, INK); display.drawPixel(fx, fy + 1, INK);
      }
      break;
    }
    case WX_FOG:
      for (int i = 0; i < 5; i++) {
        int off = (int)((t / 60 + i * 9) % 12);
        display.drawFastHLine(x + off - 4, y + 8 + i * 7, 30, INK);
      }
      break;
    default: {   // wind
      for (int i = 0; i < 3; i++) {
        int off = (int)((t / 30 + i * 13) % 40);
        int yy = y + 10 + i * 10;
        display.drawFastHLine(x + off - 20, yy, 22, INK);
        display.drawCircle(x + off + 3, yy - 3, 3, INK);
      }
      break;
    }
  }
}

// Weather scene across the whole screen (behind the text): rain, snow,
// drifting clouds, sun sparkles, lightning, fog and wind
void drawWeatherScene(uint8_t icon, unsigned long t) {
  switch (icon) {
    case WX_SUN: case WX_PARTLY:
      for (int k = 0; k < 6; k++) {                       // twinkling sparkles
        uint32_t h = hash32((uint32_t)(t / 400) * 13u + (uint32_t)k * 101u);
        int x = 50 + (int)(h % 76), y = (int)((h >> 8) % 50);
        if ((h >> 16) % 3 == 0) sparkleShape(x, y, 2, INK); else display.drawPixel(x, y, INK);
      }
      if (icon == WX_PARTLY) iconCloud((int)((t / 70) % 170) - 40, 40, INK);
      break;
    case WX_CLOUD:
      iconCloud((int)((t / 60) % 180) - 40, 2, INK);
      iconCloud((int)((t / 90 + 90) % 180) - 40, 36, INK);
      break;
    case WX_RAIN: case WX_STORM:
      for (int k = 0; k < 18; k++) {                      // raindrops falling everywhere
        uint32_t h = hash32((uint32_t)k * 2654435761u);
        int x = (int)(h % 140), sp = 2 + (int)((h >> 8) % 3);
        int y = (int)((t / 12 * sp / 2 + (h >> 12)) % 72) - 6;
        display.drawLine(x, y, x - 2, y + 4, INK);
        if (y > 58) { display.drawPixel(x - 3, 63, INK); display.drawPixel(x + 1, 63, INK); }   // splash
      }
      if (icon == WX_STORM && (t / 120) % 25 < 2) {         // lightning flash across the screen
        display.fillRect(0, 0, 128, 64, INK);
        display.fillTriangle(70, 0, 58, 30, 68, 28, ERASE);
        display.fillTriangle(68, 28, 56, 63, 74, 24, ERASE);
      }
      break;
    case WX_SNOW:
      for (int k = 0; k < 16; k++) {                      // snowflakes drifting down
        uint32_t h = hash32((uint32_t)k * 2246822519u);
        int x = (int)(h % 128) + (int)(sinf((t + k * 400) / 500.0f) * 4);
        int y = (int)((t / 45 + (h >> 10)) % 70) - 4;
        display.drawPixel(x, y, INK); display.drawPixel(x - 1, y, INK); display.drawPixel(x + 1, y, INK);
        display.drawPixel(x, y - 1, INK); display.drawPixel(x, y + 1, INK);
      }
      break;
    case WX_FOG:
      for (int k = 0; k < 6; k++) {                       // fog bands sliding by
        int off = (int)((t / (40 + k * 7) + k * 37) % 160) - 30;
        display.drawFastHLine(off, 6 + k * 10, 46, INK);
        display.drawFastHLine(off + 60, 9 + k * 10, 30, INK);
      }
      break;
    default:                                              // wind: streaks and a leaf
      for (int k = 0; k < 5; k++) {
        int off = (int)((t / 8 + k * 47) % 190) - 40;
        display.drawFastHLine(off, 8 + k * 12, 18, INK);
      }
      {
        int lx = (int)((t / 10) % 170) - 20, ly = 30 + (int)(sinf(t / 200.0f) * 12);
        display.fillCircle(lx, ly, 2, INK);
        display.drawLine(lx - 3, ly + 2, lx, ly, INK);
      }
      break;
  }
}

// Text with a black box behind it, so the weather scene never hides it
void boxedText(int x, int y, const char *s, uint8_t size) {
  display.setTextSize(size);
  int w = (int)strlen(s) * 6 * size;
  display.fillRect(x - 1, y - 1, w + 2, 8 * size + 1, ERASE);
  display.setCursor(x, y);
  display.print(s);
}

void drawWeather(unsigned long now) {
  if (!wx.valid) {
    drawWeatherIcon(WX_CLOUD, 2 + (int)(sinf(now / 600.0f) * 3), 8, now, false);
    display.setTextSize(1);
    display.setCursor(50, 10); display.print("No weather");
    display.setCursor(50, 22); display.print("yet. Open");
    display.setCursor(50, 34); display.print("Chronos or");
    display.setCursor(50, 46); display.print("add Wi-Fi.");
    return;
  }
  unsigned long on = now - screenStart;
  drawWeatherScene(wx.icon, now);
  // the big icon slides in, then keeps moving
  int slide = on < 500 ? (int)((500 - on) / 10) : 0;
  display.fillRect(0, 0, 52 - slide, 50, ERASE);
  drawWeatherIcon(wx.icon, 2 - slide, 6, now, false);
  char buf[24];
  snprintf(buf, sizeof(buf), "%d", wx.temp);
  boxedText(56, 4, buf, 3);
  int tx = 56 + (int)strlen(buf) * 18;
  display.fillRect(tx, 2, 12, 10, ERASE);
  display.drawCircle(tx + 2, 6, 2, INK);
  display.setTextSize(1);
  display.setCursor(tx + 6, 4);
  display.print("C");
  snprintf(buf, sizeof(buf), "%d/%d", wx.high, wx.low);
  boxedText(56, 30, buf, 1);
  if ((now / 3000) % 2 == 0) {                            // city, then how fresh the data is
    char city[12];
    copyText(city, sizeof(city), wx.city);
    boxedText(56, 40, city, 1);
  } else {
    unsigned long age = (now - wx.at) / 60000UL;
    if (age == 0) snprintf(buf, sizeof(buf), "now %s", wx.fromPhone ? "phone" : "web");
    else if (age < 100) snprintf(buf, sizeof(buf), "%lum ago %s", age, wx.fromPhone ? "ph" : "web");
    else snprintf(buf, sizeof(buf), "%luh ago", age / 60);
    boxedText(56, 40, buf, 1);
  }
  if (wx.dayCount > 1) {
    display.fillRect(0, 51, 128, 13, ERASE);
    for (uint8_t i = 1; i < wx.dayCount && i < 3; i++) {
      int x = (i - 1) * 64;
      drawWeatherIcon(wx.days[i].icon, x + 2, 52, now, true);
      snprintf(buf, sizeof(buf), "%d/%d", wx.days[i].hi, wx.days[i].lo);
      display.setCursor(x + 18, 54);
      display.print(buf);
    }
  }
}

// Charging: a big battery filling up, the percent, and a happy Momo
void drawChargeScreen(unsigned long now) {
  int x = 14, y = 16, w = 56, h = 28;
  display.drawRoundRect(x, y, w, h, 4, INK);
  display.drawRoundRect(x + 1, y + 1, w - 2, h - 2, 3, INK);
  display.fillRect(x + w, y + 9, 4, 10, INK);
  float from = battPct / 100.0f;
  float lvl = battFull ? 1.0f : from + (1.0f - from) * ((now % 1500) / 1500.0f);
  int fw = (int)((w - 6) * constrain(lvl, 0.0f, 1.0f));
  if (fw > 0) display.fillRect(x + 3, y + 3, fw, h - 6, INK);
  int bx = x + w / 2;                                       // lightning bolt (cut out of the fill)
  display.fillTriangle(bx + 3, y + 4, bx - 6, y + 15, bx, y + 15, ERASE);
  display.fillTriangle(bx - 1, y + 13, bx + 6, y + 13, bx - 3, y + 24, ERASE);
  display.drawTriangle(bx + 3, y + 4, bx - 6, y + 15, bx, y + 15, INK);
  char buf[16];
  snprintf(buf, sizeof(buf), "%u%%", (unsigned)battPct);
  display.setTextSize(1);
  display.setCursor(x, 52);
  display.print(battFull ? "Full!" : "Charging");
  display.setCursor(102 - (int)strlen(buf) * 3, 52);
  display.print(buf);
  drawMiniEyes(102, 30, battFull ? F_CELEBRATE : F_HAPPY, now);
  if (battFull) drawOverlay(O_SPARKLE, now, 0);
}

// Little hints while you hold: the head on the face (let go = menu, keep = rub)
// and the back button (keep holding = sleep)
void drawInputHints(unsigned long now) {
  if (inHead.pressed && !inHead.longDone && holdOnRelease(SRC_HEAD) && now - inHead.pressStart >= LONG_PRESS) {
    display.fillRect(0, 54, 128, 10, ERASE);
    display.setTextSize(1);
    display.setCursor(4, 55);
    display.print("Let go=menu Keep=rub");
  }
  if (inBtn.pressed && !inBtn.longDone && now - inBtn.pressStart >= 1200) {
    float k = (now - inBtn.pressStart - 1200) / (float)(SLEEP_HOLD_MS - 1200);
    display.fillRoundRect(3, 17, 122, 30, 4, ERASE);
    display.drawRoundRect(3, 17, 122, 30, 4, INK);
    display.setTextSize(1);
    display.setCursor(8, 21);
    display.print("Keep holding: sleep");
    progressBar(8, 33, 112, 8, k);
  }
}

// ---------- music ----------
void drawMusic(unsigned long now) {
  if (!musicKnown) {
    noteShape(58, 8, INK);
    centerText("No music yet", 24, 1);
    centerText("Play a song on", 36, 1);
    centerText("your phone", 46, 1);
    return;
  }
  marquee(musicTitle[0] ? musicTitle : "Now playing", 2, 1, now);
  char artist[22];
  copyText(artist, sizeof(artist), musicArtist[0] ? musicArtist : "");
  centerText(artist, 13, 1);
  // equaliser bars
  for (int i = 0; i < 10; i++) {
    int h = musicPlaying ? 3 + (int)(hash32((uint32_t)(now / 110) * 31u + (uint32_t)i) % 14) : 2;
    display.fillRect(4 + i * 6, 46 - h, 4, h, INK);
  }
  drawMiniEyes(100, 36, musicPlaying ? F_MUSIC : F_SLEEPY, now);
  // play / pause symbol
  if (musicPlaying) { display.fillRect(84, 24, 2, 8, INK); display.fillRect(88, 24, 2, 8, INK); }
  else display.fillTriangle(84, 24, 84, 32, 90, 28, INK);
  // flash the last command (over the equaliser, so the title stays readable)
  if (musicCmdShown != 0 && now - musicCmdAt < 700) {
    display.fillRoundRect(8, 25, 48, 22, 5, ERASE);
    display.drawRoundRect(8, 25, 48, 22, 5, INK);
    int cx = 32, cy = 36;
    if (musicCmdShown == 1) {        // next
      display.fillTriangle(cx - 8, cy - 5, cx - 8, cy + 5, cx - 1, cy, INK);
      display.fillTriangle(cx - 1, cy - 5, cx - 1, cy + 5, cx + 6, cy, INK);
      display.fillRect(cx + 6, cy - 5, 2, 11, INK);
    } else if (musicCmdShown == -1) { // previous
      display.fillRect(cx - 8, cy - 5, 2, 11, INK);
      display.fillTriangle(cx + 6, cy - 5, cx + 6, cy + 5, cx - 1, cy, INK);
      display.fillTriangle(cx - 1, cy - 5, cx - 1, cy + 5, cx - 8, cy, INK);
    } else {                          // play/pause
      display.fillTriangle(cx - 7, cy - 5, cx - 7, cy + 5, cx - 1, cy, INK);
      display.fillRect(cx + 2, cy - 5, 2, 11, INK);
      display.fillRect(cx + 6, cy - 5, 2, 11, INK);
    }
  }
  display.setTextSize(1);
  display.setCursor(0, 56);
  display.print(musicPlaying ? "<prev tap:pause next>" : "<prev tap:play  next>");
}

// ---------- alarm ----------
void drawAlarm(unsigned long now) {
  int wob = ((now / 80) % 4) - 1;
  if (wob > 1) wob = 0;
  int bx = 8 + wob * 2;
  display.fillCircle(bx + 10, 14, 10, INK);
  display.fillRect(bx, 14, 21, 12, INK);
  display.fillRect(bx - 3, 25, 27, 3, INK);
  display.fillCircle(bx + 10, 31, 3, INK);
  if ((now / 200) % 2) {
    display.drawLine(bx - 6, 6, bx - 2, 10, INK);
    display.drawLine(bx + 22, 10, bx + 26, 6, INK);
  }
  char buf[16];
  if (haveTime) timeText(nowTm.tm_hour, nowTm.tm_min, buf, sizeof(buf), false);
  else copyText(buf, sizeof(buf), "Alarm");
  display.setTextSize(2);
  display.setCursor(44, 6);
  display.print(buf);
  display.setTextSize(1);
  display.setCursor(44, 26);
  char label[16];
  copyText(label, sizeof(label), ringLabel);
  display.print(label);
  display.setCursor(0, 46);
  display.print("Tap: snooze");
  display.setCursor(0, 56);
  display.print("Hold: stop alarm");
}

// ---------- water and stretch ----------
void drawGlass(int x, int y, float level, unsigned long t) {
  // glass 22 wide at the top, 16 at the bottom, 34 tall
  display.drawLine(x, y, x + 3, y + 34, INK);
  display.drawLine(x + 22, y, x + 19, y + 34, INK);
  display.drawFastHLine(x + 3, y + 34, 17, INK);
  int top = y + 34 - (int)(32 * constrain(level, 0.0f, 1.0f));
  for (int yy = top; yy < y + 34; yy++) {
    int inset = (yy - y) * 3 / 34;
    display.drawFastHLine(x + 1 + inset, yy, 21 - 2 * inset, INK);
  }
  if (level > 0.1f) {   // bubbles
    for (int i = 0; i < 3; i++) {
      int by = y + 33 - (int)((t / 50 + i * 9) % (unsigned long)max(2, y + 33 - top));
      if (by > top + 1) display.drawPixel(x + 7 + i * 4, by, ERASE);
    }
  }
}

// ---------- water and stretch: animations, no words ----------

// Rectangle along a direction (ang radians, 0 = up), from a0 to a1 along it, halfW each side
void fillRotRect(float px, float py, float ang, float a0, float a1, float halfW, uint16_t c) {
  float dx = sinf(ang), dy = -cosf(ang), nx = -dy, ny = dx;
  float x0 = px + dx * a0, y0 = py + dy * a0, x1 = px + dx * a1, y1 = py + dy * a1;
  int ax = (int)lroundf(x0 + nx * halfW), ay = (int)lroundf(y0 + ny * halfW);
  int bx = (int)lroundf(x1 + nx * halfW), by = (int)lroundf(y1 + ny * halfW);
  int cx = (int)lroundf(x1 - nx * halfW), cy = (int)lroundf(y1 - ny * halfW);
  int ex = (int)lroundf(x0 - nx * halfW), ey = (int)lroundf(y0 - ny * halfW);
  display.fillTriangle(ax, ay, bx, by, cx, cy, c);
  display.fillTriangle(ax, ay, cx, cy, ex, ey, c);
}

// A bottle whose bottom sits at (px, py), tipped by ang (0 = standing). Returns its mouth.
void drawBottle(float px, float py, float ang, int &mx, int &my) {
  fillRotRect(px, py, ang, 0, 17, 6, INK);          // body
  fillRotRect(px, py, ang, 5, 11, 3.5f, ERASE);      // label window
  fillRotRect(px, py, ang, 17, 21, 4, INK);          // shoulder
  fillRotRect(px, py, ang, 21, 26, 2.5f, INK);       // neck
  mx = (int)lroundf(px + sinf(ang) * 27);
  my = (int)lroundf(py - cosf(ang) * 27);
}

// Tiny glasses (goal) at the bottom right: full ones = glasses you drank today
void drawGlassRow(uint8_t have, uint8_t goal, int y, bool popLast, unsigned long now) {
  uint8_t n = (uint8_t)min(8, (int)goal);
  int x0 = 127 - n * 6;
  for (uint8_t i = 0; i < n; i++) {
    int x = x0 + i * 6;
    bool full = i < have;
    if (popLast && i + 1 == have && (now / 120) % 2) continue;   // the new one blinks
    display.drawFastVLine(x, y, 8, INK);
    display.drawFastVLine(x + 4, y, 8, INK);
    display.drawFastHLine(x, y + 8, 5, INK);
    if (full) display.fillRect(x + 1, y + 2, 3, 6, INK);
  }
}

float easeInOut(float k) { k = constrain(k, 0.0f, 1.0f); return k * k * (3 - 2 * k); }

void drawWater(unsigned long now) {
  if (heroLook()) { drawHeroWater(now); return; }
  const int GX = 46, GY = 24;                       // the glass: 22 wide, 34 tall
  if (!remindDone) {
    unsigned long t = now - screenStart;
    float level = 0.85f;
    float px = 32, py = 10, ang = 1.83f;            // pouring position (about 105 degrees)
    bool bottle = true, pouring = false;
    if (t < 600) {                                  // the bottle comes in
      float k = easeInOut(t / 600.0f);
      px = 6 + 26 * k; py = -14 + 24 * k; ang = 0.52f + 1.31f * k; level = 0;
    } else if (t < 3200) {                          // pouring
      pouring = true;
      ang += sinf(t / 90.0f) * 0.05f;
      level = 0.85f * (t - 600) / 2600.0f;
    } else if (t < 3800) {                          // the bottle goes away
      float k = easeInOut((t - 3200) / 600.0f);
      px = 32 - 26 * k; py = 10 - 24 * k; ang = 1.83f - 1.31f * k;
    } else bottle = false;
    drawGlass(GX, GY, level, now);
    int surf = GY + 34 - (int)(32 * level);
    if (level > 0.05f)                              // little waves on the water
      for (int x = GX + 2; x < GX + 20; x++) if (((x + now / 90) % 5) == 0) display.drawPixel(x, surf, ERASE);
    if (bottle) {
      int mx, my;
      drawBottle(px, py, ang, mx, my);
      if (pouring) {                                // the stream and the splash
        int tx = GX + 11;
        for (int y = my; y < surf; y++) {
          int x = mx + (tx - mx) * (y - my) / max(1, surf - my);
          display.drawPixel(x, y, INK);
          if (((y + now / 25) % 6) < 2) display.drawPixel(x + 1, y, INK);
        }
        for (int k = 0; k < 4; k++) {
          uint32_t h = hash32((uint32_t)(now / 80) * 7u + (uint32_t)k);
          int sx = tx - 6 + (int)(h % 13), sy = surf - 2 - (int)((h >> 8) % 5);
          display.drawPixel(sx, sy, INK);
        }
      }
    }
    // Momo watches the glass fill, then waits for you to drink
    drawMiniEyes(104, 24, t < 3800 ? F_SURPRISED : F_THIRSTY, now);
    if (t >= 3800) dropShape(118, 8 + (int)((now / 200) % 3), 2, INK);
    drawGlassRow(st.drinks, cfg.water.goal, 54, false, now);
    return;
  }
  // you drank: Momo sips through a straw, the glass empties, hearts pop out
  unsigned long since = now - remindDoneAt;
  float level = since < 300 ? 0.85f : since < 2300 ? 0.85f - 0.85f * (since - 300) / 2000.0f : 0.0f;
  drawGlass(GX, GY, level, now);
  // straw: from the glass to Momo's mouth
  display.drawLine(GX + 12, GY + 26, GX + 18, GY - 6, INK);
  display.drawLine(GX + 13, GY + 26, GX + 19, GY - 6, INK);
  display.drawLine(GX + 18, GY - 6, 97, 38, INK);
  display.drawLine(GX + 19, GY - 6, 97, 39, INK);
  if (since >= 300 && since < 2300)                 // water going up the straw
    for (int k = 0; k < 3; k++) {
      float f = fmodf(since / 400.0f + k / 3.0f, 1.0f);
      int x = (int)(GX + 19 + (97 - GX - 19) * f), y = (int)(GY - 6 + (38 - GY + 6) * f);
      display.fillRect(x - 1, y - 2, 2, 2, ERASE);
    }
  bool goal = st.drinks >= cfg.water.goal;
  drawMiniEyes(104, 24, since < 2300 ? F_SLEEPY : (goal ? F_CELEBRATE : F_HAPPY), now);
  display.fillCircle(100, 40, since < 2300 ? 2 + (int)((since / 150) % 2) : 2, INK);   // sipping mouth
  if (since >= 2300) {
    for (int k = 0; k < 3; k++) heartShape(92 + k * 12, 6 - (int)((since - 2300) / 120) % 6 + k * 2, 2, INK);
    if (goal) drawOverlay(O_CONFETTI, now, 0);
  }
  drawGlassRow(st.drinks, cfg.water.goal, 54, since >= 2300, now);
}

// A little Momo-headed buddy doing a stretch routine (front view)
struct Pose { int8_t lean; uint8_t lUp, lLow, rUp, rLow, knee; };
const Pose P_STAND = {0, 15, 10, 15, 10, 0}, P_UP = {0, 125, 155, 125, 155, 0}, P_BENDL = {-24, 125, 150, 125, 150, 0},
           P_BENDR = {24, 125, 150, 125, 150, 0}, P_T = {0, 90, 90, 90, 90, 0}, P_SQUAT = {0, 110, 120, 110, 120, 60};
struct RoutineStep { const Pose *pose; uint16_t ms; bool circles; };
const RoutineStep ROUTINE[] = {
  {&P_STAND, 900, false}, {&P_UP, 1700, false}, {&P_BENDL, 1700, false}, {&P_UP, 900, false}, {&P_BENDR, 1700, false},
  {&P_T, 900, false}, {&P_T, 2600, true}, {&P_SQUAT, 1700, false}, {&P_STAND, 900, false}
};
const uint8_t ROUTINE_N = sizeof(ROUTINE) / sizeof(ROUTINE[0]);

float lerpf(float a, float b, float k) { return a + (b - a) * k; }

void stretchPoseAt(unsigned long t, float &lean, float &lUp, float &lLow, float &rUp, float &rLow, float &knee) {
  unsigned long total = 0;
  for (uint8_t i = 0; i < ROUTINE_N; i++) total += ROUTINE[i].ms;
  t %= total;
  uint8_t i = 0;
  while (t >= ROUTINE[i].ms) { t -= ROUTINE[i].ms; i++; }
  const Pose &a = *ROUTINE[(i + ROUTINE_N - 1) % ROUTINE_N].pose, &b = *ROUTINE[i].pose;
  float k = easeInOut(t / 500.0f);                  // half a second to move into the next pose
  lean = lerpf(a.lean, b.lean, k);
  lUp = lerpf(a.lUp, b.lUp, k); lLow = lerpf(a.lLow, b.lLow, k);
  rUp = lerpf(a.rUp, b.rUp, k); rLow = lerpf(a.rLow, b.rLow, k);
  knee = lerpf(a.knee, b.knee, k);
  if (ROUTINE[i].circles) {                         // shoulder circles
    float w = sinf(t / 220.0f) * 55;
    lUp += w; lLow += w; rUp += w; rLow += w;
  }
}

void drawBuddy(int cx, int footY, float lean, float lUp, float lLow, float rUp, float rLow, float knee, int lift, unsigned long now) {
  const float D2R = 0.0174533f;
  // legs (knees go out when squatting)
  float thigh = (8 + knee / 2) * D2R, shin = (8 - knee / 3) * D2R;
  float legH = cosf(thigh) * 11 + cosf(shin) * 11;
  float hx = cx, hy = footY - legH - lift;
  for (int side = -1; side <= 1; side += 2) {
    float kx = hx + side * sinf(thigh) * 11, ky = hy + cosf(thigh) * 11;
    float fx = kx + side * sinf(shin) * 11, fy = ky + cosf(shin) * 11;
    thickLine((int)hx + side, (int)hy, (int)kx, (int)ky, INK);
    thickLine((int)kx, (int)ky, (int)fx, (int)fy, INK);
    display.drawFastHLine((int)fx + (side > 0 ? 0 : -3), (int)fy + 1, 4, INK);
  }
  // body
  float tl = (180 - lean) * D2R;
  float nx = hx + sinf(tl) * 15, ny = hy + cosf(tl) * 15;
  thickLine((int)hx, (int)hy, (int)nx, (int)ny, INK);
  float sx = hx + sinf(tl) * 9, sy = hy + cosf(tl) * 9;        // shoulders
  // arms (angles: 0 = down, 90 = out to the side, 180 = up)
  float up[2] = {lUp, rUp}, low[2] = {lLow, rLow};
  for (int s = 0; s < 2; s++) {
    int side = s == 0 ? -1 : 1;
    float a1 = (side * up[s] - lean) * D2R, a2 = (side * low[s] - lean) * D2R;
    float ex = sx + sinf(a1) * 10, ey = sy + cosf(a1) * 10;
    float wx = ex + sinf(a2) * 10, wy = ey + cosf(a2) * 10;
    thickLine((int)sx, (int)sy, (int)ex, (int)ey, INK);
    thickLine((int)ex, (int)ey, (int)wx, (int)wy, INK);
    display.fillCircle((int)wx, (int)wy, 1, INK);
  }
  // Momo's head: a rounded box with two eyes
  float hcx = hx + sinf(tl) * 21, hcy = hy + cosf(tl) * 21;
  display.fillRoundRect((int)hcx - 6, (int)hcy - 5, 13, 11, 3, INK);
  bool blink = (now % 2600) < 140;
  display.fillRect((int)hcx - 3, (int)hcy - 2 + (blink ? 2 : 0), 2, blink ? 1 : 4, ERASE);
  display.fillRect((int)hcx + 2, (int)hcy - 2 + (blink ? 2 : 0), 2, blink ? 1 : 4, ERASE);
}

void drawStretch(unsigned long now) {
  if (heroLook()) { drawHeroStretch(now); return; }
  uint8_t goal = (uint8_t)min(8, (int)cfg.stretch.goal);
  if (remindDone) {                                  // done: jump for joy
    unsigned long since = now - remindDoneAt;
    int lift = (int)(fabsf(sinf(since / 160.0f)) * 7);
    float w = sinf(since / 120.0f) * 12;
    drawBuddy(40, 61, 0, 125 + w, 155 + w, 125 - w, 155 - w, 0, lift, now);
    drawMiniEyes(104, 14, F_CELEBRATE, now);
    drawOverlay(O_CONFETTI, now, 0);
  } else {
    float lean, lUp, lLow, rUp, rLow, knee;
    unsigned long t = now - screenStart;
    stretchPoseAt(t, lean, lUp, lLow, rUp, rLow, knee);
    drawBuddy(40, 61, lean, lUp, lLow, rUp, rLow, knee, 0, now);
    for (int k = 0; k < 2; k++)                       // little motion marks
      if (((now / 300) + k) % 2) display.drawPixel(18 + k * 44, 14 + k * 3, INK);
    drawMiniEyes(104, 14, F_EFFORT, now);
    // a ring of dots fills up over 20 seconds
    int filled = (int)min(12UL, t / (20000UL / 12));
    for (int k = 0; k < 12; k++) {
      float a = k * 0.5236f - 1.5708f;
      int x = 104 + (int)lroundf(cosf(a) * 11), y = 48 + (int)lroundf(sinf(a) * 11);
      if (k < filled) display.fillCircle(x, y, 2, INK); else display.drawPixel(x, y, INK);
    }
  }
  // stretches today: one dot each
  for (uint8_t i = 0; i < goal; i++) {
    int x = 104 - ((int)goal - 1) * 3 + i * 6;
    if (i < st.stretches) display.fillCircle(x, 29, 2, INK); else display.drawCircle(x, 29, 2, INK);
  }
}

// Your own reminder: a ringing bell and your words, big
void drawNote(unsigned long now) {
  if (noteIdx < 0) return;
  const NoteCfg &n = notes[noteIdx];
  int swing = (now - noteStart < 180000) ? (int)(sinf(now / 70.0f) * 3) : 0;
  display.fillCircle(10 + swing, 9, 7, INK);                 // bell
  display.fillRect(3 + swing, 9, 15, 6, INK);
  display.fillRect(1 + swing, 14, 19, 2, INK);
  display.fillCircle(10 - swing, 18, 2, INK);
  display.fillRect(9 + swing, 0, 3, 2, INK);
  if (swing != 0) {                                         // ring marks
    display.drawFastVLine(24, 4, 4, INK); display.drawFastVLine(27, 2, 7, INK);
  }
  char t[12];
  timeText(n.h, n.m, t, sizeof(t), true);
  display.setTextSize(1);
  display.setCursor(34, 6);
  display.print(t);
  // the text: big if it fits in two lines of 10, else small (up to 4 lines)
  char lines[4][22];
  uint8_t cnt = 0;
  const char *p = n.text;
  int width = (int)strlen(n.text) <= 20 ? 10 : 21;
  while (*p && cnt < 4) {
    while (*p == ' ') p++;
    int len = (int)strlen(p);
    int take = len <= width ? len : width;
    if (len > width) { int sp = take; while (sp > 0 && p[sp] != ' ') sp--; if (sp > 0) take = sp; }
    memcpy(lines[cnt], p, take); lines[cnt][take] = '\0';
    cnt++; p += take;
  }
  if (width == 10 && cnt > 2) {                              // did not fit big: go small
    width = 21; cnt = 0; p = n.text;
    while (*p && cnt < 4) {
      while (*p == ' ') p++;
      int len = (int)strlen(p);
      int take = len <= width ? len : width;
      if (len > width) { int sp = take; while (sp > 0 && p[sp] != ' ') sp--; if (sp > 0) take = sp; }
      memcpy(lines[cnt], p, take); lines[cnt][take] = '\0';
      cnt++; p += take;
    }
  }
  uint8_t size = width == 10 ? 2 : 1;
  int lh = size == 2 ? 18 : 10;
  int y0 = 24 + ((size == 2 ? 2 : 4) - cnt) * lh / 2;
  for (uint8_t i = 0; i < cnt; i++) centerText(lines[i], y0 + i * lh, size);
  drawMiniEyes(110, 9, (now - noteStart < 180000) ? F_SURPRISED : F_HOPEFUL, now);
}

void drawGallery(unsigned long now) {
  drawEyes(now);
  display.fillRect(0, 55, 128, 9, ERASE);
  char buf[24];
  snprintf(buf, sizeof(buf), "%u/%u %s", (unsigned)(galleryIdx + 1), (unsigned)F_COUNT, FACE_NAMES[galleryIdx]);
  display.setTextSize(1);
  display.setCursor(1, 56);
  display.print(buf);
}

void drawPair(unsigned long now) {
  display.drawRoundRect(4, 14, 18, 14, 3, INK);
  display.drawCircle(13, 13, 6, INK);
  display.fillRect(6, 13, 14, 4, ERASE);
  display.drawRoundRect(4, 14, 18, 14, 3, INK);
  display.fillRect(12, 19, 3, 5, INK);
  display.setTextSize(1);
  display.setCursor(30, 2);
  display.print("Pair code for");
  display.setCursor(30, 12);
  display.print("the Momo app:");
  display.setTextSize(2);
  display.setCursor(30, 26);
  display.print(pairCode);
  long left = (long)(pairUntil - now);
  if (left < 0) left = 0;
  progressBar(30, 46, 94, 6, left / 120000.0f);
  display.setTextSize(1);
  display.setCursor(30, 56);
  display.print("2x tap: cancel");
}

void drawWifiInfo(unsigned long now) {
  (void)now;
  display.setTextSize(1);
  display.setCursor(0, 0);
  if (apMode) {
    display.print("Setup hotspot:");
    display.setCursor(0, 11); display.print(apName);
    display.setCursor(0, 22); display.print("Password: "); display.print(apPass);
    display.setCursor(0, 33); display.print("Then open:");
    display.setCursor(0, 44); display.print("http://192.168.4.1");
    display.setCursor(0, 56); display.print("Hold or 2x tap: back");
    return;
  }
  if (wifiConnected) {
    display.print("Wi-Fi: ");
    char ss[15];
    copyText(ss, sizeof(ss), currentSsid());
    display.print(ss);
    display.setCursor(0, 12); display.print("Momo app on phone:");
    display.setCursor(0, 24); display.print("http://"); display.print(ipText);
#if WOKWI
    display.setCursor(0, 36); display.print("PC: localhost:8180");
#else
    display.setCursor(0, 36); display.print("or http://momo.local");
#endif
  } else if (MOMO_NO_WIFI) {
    display.print("No Wi-Fi in this");
    display.setCursor(0, 12); display.print("build. Use the Wi-Fi");
    display.setCursor(0, 24); display.print("version for the app.");
  } else {
    display.print(wifiSsid[0] ? "Wi-Fi: connecting..." : "Wi-Fi not set up");
    display.setCursor(0, 12); display.print("Menu > Phone & Wi-Fi");
    display.setCursor(0, 24); display.print("> Wi-Fi setup");
  }
  display.setCursor(0, 56);
  display.print("Hold or 2x tap: back");
}

void drawHello(unsigned long now) {
  unsigned long on = now - screenStart;
  if (helloMorning) {
    int rise = (int)min(16UL, on / 80);
    display.fillCircle(20, 40 - rise, 10, INK);
    for (int i = 0; i < 8; i++) {
      float a = i * 0.7854f + on / 900.0f;
      display.drawLine(20 + (int)(cosf(a) * 13), 40 - rise + (int)(sinf(a) * 13),
                       20 + (int)(cosf(a) * 17), 40 - rise + (int)(sinf(a) * 17), INK);
    }
    display.fillRect(0, 44, 42, 20, ERASE);
    display.drawFastHLine(0, 44, 42, INK);
    display.setTextSize(1);
    display.setCursor(46, 4);
    int h = haveTime ? nowTm.tm_hour : 8;
    display.print(h < 12 ? "Good morning!" : h < 17 ? "Hello there!" : "Good evening!");
  } else {
    display.fillCircle(20, 22, 11, INK);
    display.fillCircle(26, 18, 10, ERASE);
    for (int i = 0; i < 5; i++) if ((on / 300 + i) % 3) display.drawPixel(6 + i * 9, 44 + (i % 2) * 6, INK);
    display.setTextSize(1);
    display.setCursor(46, 4);
    display.print("Good night!");
  }
  if (haveTime) {
    char t[12], d[24];
    timeText(nowTm.tm_hour, nowTm.tm_min, t, sizeof(t), true);
    snprintf(d, sizeof(d), "%s %d %s", DAY_SHORT[nowTm.tm_wday], nowTm.tm_mday, MONTH_SHORT[nowTm.tm_mon]);
    display.setCursor(46, 18); display.print(t);
    display.setCursor(46, 28); display.print(d);
  }
  if (helloMorning && wx.valid) {
    char w[22];
    snprintf(w, sizeof(w), "%dC %d/%d", wx.temp, wx.high, wx.low);
    drawWeatherIcon(wx.icon, 46, 40, now, true);
    display.setCursor(62, 42);
    display.print(w);
  } else if (!helloMorning) {
    display.setCursor(46, 42);
    display.print("Sleep well :)");
  }
}

void drawFrame(unsigned long now) {
  display.clearDisplay();
  display.setTextWrap(false);
  display.setTextColor(INK);
  switch (screen) {
    case SC_BOOT:    drawBoot(now); break;
    case SC_FACE:    drawFaceScreen(now); break;
    case SC_MENU:    drawMenu(); break;
    case SC_CLOCK:   drawClock(now); break;
    case SC_FOCUS:   drawFocus(now); break;
    case SC_ALERT:   drawAlert(now); break;
    case SC_CALL:    drawCall(now); break;
    case SC_GAME:    drawGame(now); break;
    case SC_PONG:    drawPong(); break;
    case SC_WEATHER: drawWeather(now); break;
    case SC_MUSIC:   drawMusic(now); break;
    case SC_ALARM:   drawAlarm(now); break;
    case SC_WATER:   drawWater(now); break;
    case SC_STRETCH: drawStretch(now); break;
    case SC_GALLERY: drawGallery(now); break;
    case SC_PAIR:    drawPair(now); break;
    case SC_WIFI:    drawWifiInfo(now); break;
    case SC_HELLO:   drawHello(now); break;
    case SC_NOTE:    drawNote(now); break;
    case SC_CHARGE:  drawChargeScreen(now); break;
  }
  drawInputHints(now);
  display.display();
}
