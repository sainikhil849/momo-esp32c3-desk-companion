// =====================================================================
//  net.h  -  Wi-Fi, internet time, weather, space station, and the
//            Momo app (a web app served by Momo itself)
//
//  Security:
//   - The app must be paired once with a 6-digit code shown on Momo's
//     screen (only someone next to Momo can see it).
//   - Pairing gives the phone a long secret token; Momo stores only a
//     SHA-256 fingerprint of it. Max 3 phones; unpair any time.
//   - 5 wrong codes lock pairing for 5 minutes.
//   - The Wi-Fi password is never sent to the app or printed.
// =====================================================================

#include "webapp.h"

unsigned long apUntil = 0, ntpCheckAt = 0, weatherAt = 0, issAt = 0, issAlertedAt = 0;
bool weatherForced = false;   // "Update now" in the app
bool ntpDone = false;
unsigned long pairLockUntil = 0;
uint8_t pairFails = 0;

char jsonBuf[7000];

const char *currentSsid() {
#if WOKWI
  return "Wokwi-GUEST";
#else
  return wifiSsid;
#endif
}

void sha256(const uint8_t *data, size_t len, uint8_t out[32]) {
  mbedtls_sha256(data, len, out, 0);
}

float distanceKm(float lat1, float lon1, float lat2, float lon2) {
  const float D2R = 0.01745329f;
  float dLat = (lat2 - lat1) * D2R, dLon = (lon2 - lon1) * D2R;
  float a = sinf(dLat / 2) * sinf(dLat / 2) + cosf(lat1 * D2R) * cosf(lat2 * D2R) * sinf(dLon / 2) * sinf(dLon / 2);
  return 6371.0f * 2.0f * atan2f(sqrtf(a), sqrtf(1.0f - a));
}

void randomDigits(char *out, uint8_t n) {
  for (uint8_t i = 0; i < n; i++) out[i] = (char)('0' + esp_random() % 10);
  out[n] = '\0';
}

// ---------- weather helpers (also used by the Chronos app) ----------
uint8_t wmoToIcon(int code) {
  if (code == 0) return WX_SUN;
  if (code <= 2) return WX_PARTLY;
  if (code == 3) return WX_CLOUD;
  if (code == 45 || code == 48) return WX_FOG;
  if ((code >= 71 && code <= 77) || code == 85 || code == 86) return WX_SNOW;
  if (code >= 95) return WX_STORM;
  if (code >= 51) return WX_RAIN;
  return WX_CLOUD;
}

uint8_t chronosToIcon(int icon) {   // Chronos weather icon codes
  switch (icon) {
    case 0: return WX_PARTLY;
    case 1: return WX_SUN;
    case 2: return WX_SNOW;
    case 3: return WX_RAIN;
    case 4: return WX_CLOUD;
    case 5: return WX_STORM;
    case 6: return WX_WIND;
    case 7: return WX_FOG;
    default: return WX_CLOUD;
  }
}

const char *WX_NAMES[8] = {"Sunny", "Partly cloudy", "Cloudy", "Rain", "Storm", "Snow", "Fog", "Windy"};

// Parse an Open-Meteo answer into Momo's weather (kept separate so it can be tested)
bool parseOpenMeteo(const char *body, unsigned long now) {
  JsonDocument doc;
  if (deserializeJson(doc, body) != DeserializationError::Ok) return false;
  JsonObjectConst cur = doc["current"];
  if (cur.isNull() || !cur["temperature_2m"].is<double>()) return false;
  wx.temp = (int)lroundf(cur["temperature_2m"].as<float>());
  wx.humidity = (uint8_t)cur["relative_humidity_2m"].as<int>();
  wx.icon = wmoToIcon(cur["weather_code"].as<int>());
  JsonArrayConst codes = doc["daily"]["weather_code"];
  JsonArrayConst hi = doc["daily"]["temperature_2m_max"];
  JsonArrayConst lo = doc["daily"]["temperature_2m_min"];
  wx.dayCount = 0;
  for (size_t i = 0; i < codes.size() && i < 3; i++) {
    wx.days[i].icon = wmoToIcon(codes[i].as<int>());
    wx.days[i].hi = (int8_t)lroundf(hi[i].as<float>());
    wx.days[i].lo = (int8_t)lroundf(lo[i].as<float>());
    wx.dayCount++;
  }
  wx.high = wx.dayCount ? wx.days[0].hi : wx.temp;
  wx.low = wx.dayCount ? wx.days[0].lo : wx.temp;
  copyText(wx.city, sizeof(wx.city), cfg.city);
  wx.valid = true;
  wx.fromPhone = false;
  wx.at = now;
  return true;
}

#if !MOMO_NO_WIFI

WebServer server(80);
DNSServer dns;
bool serverStarted = false, mdnsStarted = false;

void sendJson(int code) { server.send(code, "application/json", jsonBuf); }

void sendError(int code, const char *msg) {
  snprintf(jsonBuf, sizeof(jsonBuf), "{\"error\":\"%s\"}", msg);
  sendJson(code);
}

void sendDoc(int code, JsonDocument &doc) {
  if (serializeJson(doc, jsonBuf, sizeof(jsonBuf)) >= sizeof(jsonBuf) - 1) { sendError(500, "too big"); return; }
  sendJson(code);
}

// Which paired phone sent this request? -1 = not paired
int authSlot() {
  String tok = server.header("X-Momo-Token");
  if (tok.length() != 64) return -1;
  uint8_t h[32];
  sha256((const uint8_t *)tok.c_str(), 64, h);
  for (int i = 0; i < MAX_PHONES; i++) {
    if (!phones[i].used) continue;
    uint8_t diff = 0;
    for (int k = 0; k < 32; k++) diff |= (uint8_t)(phones[i].hash[k] ^ h[k]);   // constant-time compare
    if (diff == 0) return i;
  }
  return -1;
}

bool requireAuth() {
  if (authSlot() >= 0) return true;
  sendError(401, "pair");
  return false;
}

bool readBody(JsonDocument &doc) {
  if (!server.hasArg("plain")) return false;
  String body = server.arg("plain");
  if (body.length() > 6000) return false;
  return deserializeJson(doc, body.c_str()) == DeserializationError::Ok;
}

// ---------- pages ----------
void handleRoot() {
  server.sendHeader("Content-Encoding", "gzip");
  server.sendHeader("Cache-Control", "no-cache");
  server.send_P(200, "text/html", (const char *)WEBAPP_GZ, WEBAPP_GZ_LEN);
}

void handleManifest() {
  server.send(200, "application/manifest+json",
              "{\"name\":\"Momo\",\"short_name\":\"Momo\",\"start_url\":\"/\",\"display\":\"standalone\","
              "\"background_color\":\"#101322\",\"theme_color\":\"#101322\","
              "\"icons\":[{\"src\":\"/icon.svg\",\"sizes\":\"any\",\"type\":\"image/svg+xml\"}]}");
}

void handleIcon() {
  server.send(200, "image/svg+xml",
              "<svg xmlns='http://www.w3.org/2000/svg' viewBox='0 0 64 64'><rect width='64' height='64' rx='14' fill='#101322'/>"
              "<rect x='12' y='18' width='16' height='22' rx='6' fill='#7cf3e8'/><rect x='36' y='18' width='16' height='22' rx='6' fill='#7cf3e8'/></svg>");
}

void handleNotFound() {
  if (apMode) {   // captive portal: send every page to the Momo app
    server.sendHeader("Location", "http://192.168.4.1/", true);
    server.send(302, "text/plain", "");
    return;
  }
  sendError(404, "not found");
}

// ---------- API ----------
void apiInfo() {
  JsonDocument doc;
  doc["name"] = cfg.name;
  doc["version"] = MOMO_VERSION;
  doc["wokwi"] = (bool)WOKWI;
  bool any = false;
  for (int i = 0; i < MAX_PHONES; i++) any |= phones[i].used;
  doc["paired"] = any;
  doc["you"] = authSlot() >= 0;
  doc["setup"] = apMode;
  JsonArray f = doc["faces"].to<JsonArray>();
  for (int i = 0; i < F_COUNT; i++) f.add(FACE_NAMES[i]);
  JsonArray g = doc["games"].to<JsonArray>();
  for (int i = 0; i < G_COUNT; i++) g.add(GAME_NAMES[i]);
  sendDoc(200, doc);
}

void apiPairStart() {
  unsigned long now = millis();
  if ((long)(pairLockUntil - now) > 0) {
    snprintf(jsonBuf, sizeof(jsonBuf), "{\"error\":\"locked\",\"wait\":%lu}", (pairLockUntil - now) / 1000 + 1);
    sendJson(429);
    return;
  }
  randomDigits(pairCode, 6);
  pairUntil = now + 120000;
  pairFails = 0;
  showPairScreen(now);
  Serial.println("Pairing: code shown on Momo's screen");
  server.send(200, "application/json", "{\"ok\":true,\"seconds\":120}");
}

void apiPair() {
  unsigned long now = millis();
  JsonDocument doc;
  if (!readBody(doc)) { sendError(400, "bad request"); return; }
  if ((long)(pairLockUntil - now) > 0) { sendError(429, "locked"); return; }
  if (pairUntil == 0 || (long)(pairUntil - now) <= 0) { sendError(403, "expired"); return; }
  const char *code = doc["code"] | "";
  if (strlen(code) != 6 || strcmp(code, pairCode) != 0) {
    pairFails++;
    if (pairFails >= 5) {
      pairLockUntil = now + 300000;
      pairUntil = 0;
      endPairScreen(now, false);
      Serial.println("Pairing: too many wrong codes, locked for 5 minutes");
    }
    snprintf(jsonBuf, sizeof(jsonBuf), "{\"error\":\"wrong\",\"left\":%d}", 5 - (int)pairFails);
    sendJson(403);
    return;
  }
  // new secret token for this phone
  uint8_t raw[32];
  esp_fill_random(raw, sizeof(raw));
  char token[65];
  for (int i = 0; i < 32; i++) snprintf(token + i * 2, 3, "%02x", raw[i]);
  int slot = -1;
  for (int i = 0; i < MAX_PHONES; i++) if (!phones[i].used) { slot = i; break; }
  if (slot < 0) {   // all used: replace the oldest
    slot = 0;
    for (int i = 1; i < MAX_PHONES; i++) if (phones[i].when < phones[slot].when) slot = i;
  }
  phones[slot].used = true;
  sha256((const uint8_t *)token, 64, phones[slot].hash);
  copyClean(phones[slot].device, sizeof(phones[slot].device), doc["device"] | "Phone");
  if (phones[slot].device[0] == '\0') copyText(phones[slot].device, sizeof(phones[slot].device), "Phone");
  phones[slot].when = minutesSince2024();
  savePhones();
  pairUntil = 0;
  memset(pairCode, 0, sizeof(pairCode));
  endPairScreen(now, true);
  Serial.printf("Pairing: paired with %s\n", phones[slot].device);
  snprintf(jsonBuf, sizeof(jsonBuf), "{\"token\":\"%s\"}", token);
  sendJson(200);
}

void stateToJson(JsonObject o) {
  o["name"] = cfg.name;
  o["face"] = faceNow;
  o["faceName"] = FACE_NAMES[faceNow];
  o["life"] = (int)life;
  o["haveTime"] = haveTime;
  if (haveTime) {
    char t[12];
    snprintf(t, sizeof(t), "%02d:%02d", nowTm.tm_hour, nowTm.tm_min);
    o["time"] = t;
  }
  o["affection"] = st.affection;
  o["pets"] = st.pets;
  o["drinks"] = st.drinks;
  o["waterGoal"] = cfg.water.goal;
  o["stretches"] = st.stretches;
  o["stretchGoal"] = cfg.stretch.goal;
  o["phone"] = phoneConnected();
  o["volume"] = cfg.volume;
  char al[20];
  if (nextAlarmText(al, sizeof(al))) o["nextAlarm"] = al;
  char nt[64];
  if (nextNoteText(nt, sizeof(nt))) o["nextNote"] = nt;
  JsonObject w = o["weather"].to<JsonObject>();
  w["valid"] = wx.valid;
  if (wx.valid) {
    w["city"] = wx.city;
    w["temp"] = wx.temp;
    w["high"] = wx.high;
    w["low"] = wx.low;
    w["text"] = WX_NAMES[wx.icon % 8];
    w["fromPhone"] = wx.fromPhone;
    w["ageMin"] = (millis() - wx.at) / 60000UL;
  }
  JsonObject b = o["battery"].to<JsonObject>();
  b["present"] = battPresent;
  if (battPresent) {
    b["percent"] = battPct;
    b["mv"] = battMv;
    b["charging"] = charging;
    b["full"] = battFull;
  }
  JsonObject m = o["music"].to<JsonObject>();
  m["known"] = musicKnown;
  m["title"] = musicTitle;
  m["artist"] = musicArtist;
  m["playing"] = musicPlaying;
  JsonObject n = o["wifi"].to<JsonObject>();
  n["connected"] = wifiConnected;
  n["ssid"] = currentSsid();
  n["ip"] = ipText;
  n["setup"] = apMode;
  JsonObject sc = o["scores"].to<JsonObject>();
  scoresToJson(sc);
}

void apiState() {
  if (!requireAuth()) return;
  JsonDocument doc;
  stateToJson(doc.to<JsonObject>());
  sendDoc(200, doc);
}

void apiGetSettings() {
  if (!requireAuth()) return;
  JsonDocument doc;
  JsonObject o = doc.to<JsonObject>();
  settingsToJson(o);
  o["wifiSsid"] = currentSsid();
  o["wifiSaved"] = WOKWI || wifiSsid[0] != '\0';
  sendDoc(200, doc);
}

void apiSetSettings() {
  if (!requireAuth()) return;
  JsonDocument doc;
  if (!readBody(doc) || !doc.is<JsonObject>()) { sendError(400, "bad request"); return; }
  int32_t oldOffset = cfg.utcOffset;
  float oldLat = cfg.lat, oldLon = cfg.lon;
  settingsFromJson(doc.as<JsonObjectConst>());
  unsigned long now = millis();
  markCfg(now);
  saveStore(now, true);
  if (oldOffset != cfg.utcOffset) ntpDone = false;
  if (oldLat != cfg.lat || oldLon != cfg.lon) weatherAt = now;
  settingsChanged(now);
  apiGetSettings();
}

void apiAction() {
  if (!requireAuth()) return;
  JsonDocument doc;
  if (!readBody(doc)) { sendError(400, "bad request"); return; }
  const char *what = doc["do"] | "";
  unsigned long now = millis();
  bool ok = appAction(what, doc.as<JsonObjectConst>(), now);
  if (!ok) { sendError(400, "unknown action"); return; }
  server.send(200, "application/json", "{\"ok\":true}");
}

void apiBackup() {
  if (!requireAuth()) return;
  JsonDocument doc;
  doc["type"] = "momo-backup";
  doc["version"] = 2;
  doc["name"] = cfg.name;
  settingsToJson(doc["settings"].to<JsonObject>());
  scoresToJson(doc["scores"].to<JsonObject>());
  server.sendHeader("Content-Disposition", "attachment; filename=\"momo-backup.json\"");
  sendDoc(200, doc);
}

void apiRestore() {
  if (!requireAuth()) return;
  JsonDocument doc;
  if (!readBody(doc)) { sendError(400, "not a backup file"); return; }
  if (strcmp(doc["type"] | "", "momo-backup") != 0 || !doc["settings"].is<JsonObjectConst>()) {
    sendError(400, "not a Momo backup");
    return;
  }
  settingsFromJson(doc["settings"].as<JsonObjectConst>());
  if (doc["scores"].is<JsonObjectConst>()) scoresFromJson(doc["scores"].as<JsonObjectConst>());
  unsigned long now = millis();
  markCfg(now);
  markStats(now);
  saveStore(now, true);
  ntpDone = false;
  weatherAt = now;
  settingsChanged(now);
  server.send(200, "application/json", "{\"ok\":true}");
}

void apiWifi() {
  if (!requireAuth()) return;
  JsonDocument doc;
  if (!readBody(doc)) { sendError(400, "bad request"); return; }
  const char *ssid = doc["ssid"] | "";
  if (strlen(ssid) == 0 || strlen(ssid) > 32) { sendError(400, "Wi-Fi name missing"); return; }
  bool sameNet = strcmp(ssid, wifiSsid) == 0;
  copyText(wifiSsid, sizeof(wifiSsid), ssid);
  if (doc["pass"].is<const char *>()) copyText(wifiPass, sizeof(wifiPass), doc["pass"].as<const char *>());
  else if (!sameNet) wifiPass[0] = '\0';
  saveWifi();
  Serial.printf("Wi-Fi: new network saved: %s\n", wifiSsid);   // never the password
#if !WOKWI
  WiFi.disconnect();
  WiFi.begin(wifiSsid, wifiPass);
  if (apMode) apUntil = millis() + 180000;   // keep the hotspot a bit so the phone can read the result
#endif
  server.send(200, "application/json", "{\"ok\":true}");
}

void apiWifiScan() {
  if (!requireAuth()) return;
  JsonDocument doc;
  JsonArray a = doc["networks"].to<JsonArray>();
#if WOKWI
  JsonObject x = a.add<JsonObject>();
  x["ssid"] = "Wokwi-GUEST";
  x["rssi"] = -40;
  x["open"] = true;
#else
  int n = WiFi.scanNetworks();
  for (int i = 0; i < n && i < 20; i++) {
    String s = WiFi.SSID(i);
    if (s.length() == 0) continue;
    JsonObject x = a.add<JsonObject>();
    x["ssid"] = s.c_str();
    x["rssi"] = WiFi.RSSI(i);
    x["open"] = WiFi.encryptionType(i) == WIFI_AUTH_OPEN;
  }
  WiFi.scanDelete();
#endif
  sendDoc(200, doc);
}

void apiDevices() {
  int me = authSlot();
  if (me < 0) { sendError(401, "pair"); return; }
  JsonDocument doc;
  JsonArray a = doc["devices"].to<JsonArray>();
  for (int i = 0; i < MAX_PHONES; i++) {
    if (!phones[i].used) continue;
    JsonObject x = a.add<JsonObject>();
    x["slot"] = i;
    x["device"] = phones[i].device;
    x["you"] = (i == me);
  }
  sendDoc(200, doc);
}

void apiUnpair() {
  int me = authSlot();
  if (me < 0) { sendError(401, "pair"); return; }
  JsonDocument doc;
  readBody(doc);
  if (doc["all"] | false) {
    memset(phones, 0, sizeof(phones));
  } else {
    int slot = doc["slot"] | me;
    if (slot >= 0 && slot < MAX_PHONES) memset(&phones[slot], 0, sizeof(PairSlot));
  }
  savePhones();
  Serial.println("Pairing: phone(s) removed");
  server.send(200, "application/json", "{\"ok\":true}");
}

void startServer() {
  if (serverStarted) return;
  static const char *hdrs[] = {"X-Momo-Token"};
  server.collectHeaders(hdrs, 1);
  server.on("/", HTTP_GET, handleRoot);
  server.on("/index.html", HTTP_GET, handleRoot);
  server.on("/manifest.json", HTTP_GET, handleManifest);
  server.on("/icon.svg", HTTP_GET, handleIcon);
  server.on("/api/info", HTTP_GET, apiInfo);
  server.on("/api/pair/start", HTTP_POST, apiPairStart);
  server.on("/api/pair", HTTP_POST, apiPair);
  server.on("/api/state", HTTP_GET, apiState);
  server.on("/api/settings", HTTP_GET, apiGetSettings);
  server.on("/api/settings", HTTP_POST, apiSetSettings);
  server.on("/api/action", HTTP_POST, apiAction);
  server.on("/api/backup", HTTP_GET, apiBackup);
  server.on("/api/restore", HTTP_POST, apiRestore);
  server.on("/api/wifi", HTTP_POST, apiWifi);
  server.on("/api/wifi/scan", HTTP_GET, apiWifiScan);
  server.on("/api/devices", HTTP_GET, apiDevices);
  server.on("/api/unpair", HTTP_POST, apiUnpair);
  server.onNotFound(handleNotFound);
  server.begin();
  serverStarted = true;
}

// ---------- Wi-Fi ----------
void startSetupHotspot(unsigned long now) {
#if WOKWI
  (void)now;
  showAlert(AL_WIFI, "Simulator", "The simulator joins Wokwi-GUEST by itself. The app is at localhost:8180 on your PC.", now);
#else
  uint8_t mac[6];
  WiFi.macAddress(mac);
  snprintf(apName, sizeof(apName), "Momo-%02X%02X", mac[4], mac[5]);
  randomDigits(apPass, 8);
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(apName, apPass);
  dns.start(53, "*", WiFi.softAPIP());
  apMode = true;
  apUntil = now + 15UL * 60 * 1000;
  startServer();
  Serial.printf("Wi-Fi setup hotspot: %s (password on Momo's screen)\n", apName);
  goScreen(SC_WIFI, now);
#endif
}

void stopSetupHotspot() {
  if (!apMode) return;
  dns.stop();
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  apMode = false;
  Serial.println("Wi-Fi setup hotspot closed");
}

void startWifi(unsigned long now) {
  WiFi.mode(WIFI_STA);
  WiFi.setHostname("momo");
#if WOKWI
  WiFi.begin("Wokwi-GUEST", "", 6);
  Serial.println("Wi-Fi: connecting to Wokwi-GUEST");
#else
  if (wifiSsid[0] != '\0') {
    WiFi.begin(wifiSsid, wifiPass);
    Serial.printf("Wi-Fi: connecting to %s\n", wifiSsid);
  } else {
    bool anyPhone = false;
    for (int i = 0; i < MAX_PHONES; i++) anyPhone |= phones[i].used;
    Serial.println("Wi-Fi: not set up yet (Menu > Phone & Wi-Fi > Wi-Fi setup)");
    if (!anyPhone) startSetupHotspot(now);   // very first start: open the setup hotspot
  }
#endif
  startServer();
}

void checkSpaceStation(unsigned long now) {
  HTTPClient http;
  http.setTimeout(4000);
  if (!http.begin("http://api.open-notify.org/iss-now.json")) return;
  int code = http.GET();
  if (code == 200) {
    String body = http.getString();
    JsonDocument doc;
    if (deserializeJson(doc, body.c_str()) == DeserializationError::Ok) {
      float lat = atof(doc["iss_position"]["latitude"] | "0");
      float lon = atof(doc["iss_position"]["longitude"] | "0");
      float km = distanceKm(cfg.lat, cfg.lon, lat, lon);
      if (km < 3000) Serial.printf("Space station is %.0f km away\n", km);
      if (km < 1500 && (issAlertedAt == 0 || now - issAlertedAt > 30UL * 60 * 1000)) {
        issAlertedAt = now;
        showAlert(AL_SPACE, "Space station!", "The ISS is flying over your area right now. Look up!", now);
      }
    }
  }
  http.end();
}

bool fetchWeather(unsigned long now) {
  char url[300];
  snprintf(url, sizeof(url),
           "https://api.open-meteo.com/v1/forecast?latitude=%.3f&longitude=%.3f"
           "&current=temperature_2m,relative_humidity_2m,weather_code"
           "&daily=weather_code,temperature_2m_max,temperature_2m_min&forecast_days=3&timezone=auto",
           cfg.lat, cfg.lon);
  NetworkClientSecure client;
  client.setInsecure();   // public weather data, nothing private is sent
  HTTPClient http;
  http.setTimeout(6000);
  if (!http.begin(client, url)) return false;
  int code = http.GET();
  bool ok = false;
  if (code == 200) {
    String body = http.getString();
    ok = parseOpenMeteo(body.c_str(), now);
  }
  http.end();
  Serial.printf("Weather from the internet: %s\n", ok ? "ok" : "failed");
  return ok;
}

void updateNetwork(unsigned long now) {
  if (serverStarted) server.handleClient();
  if (apMode) {
    dns.processNextRequest();
    if ((long)(now - apUntil) > 0) stopSetupHotspot();
  }
  bool up = WiFi.status() == WL_CONNECTED;
  if (up != wifiConnected) {
    wifiConnected = up;
    if (up) {
      copyText(ipText, sizeof(ipText), WiFi.localIP().toString().c_str());
      Serial.printf("Wi-Fi connected. Momo app: http://%s\n", ipText);
      if (!mdnsStarted) mdnsStarted = MDNS.begin("momo");
      if (mdnsStarted) MDNS.addService("http", "tcp", 80);
      ntpDone = false;
      weatherAt = now + 3000;
      issAt = now + 20000;
      if (apMode) apUntil = now + 120000;   // the phone can now switch back to home Wi-Fi
      wifiJoined(now);
    } else {
      Serial.println("Wi-Fi disconnected");
    }
  }
  if (!wifiConnected) return;
  if (!ntpDone && (long)(now - ntpCheckAt) >= 0) {
    ntpCheckAt = now + 2000;
    static bool asked = false;
    static int32_t askedOffset = 0;
    if (!asked || askedOffset != cfg.utcOffset) {
      configTime(cfg.utcOffset, 0, "pool.ntp.org", "time.google.com");
      asked = true;
      askedOffset = cfg.utcOffset;
    }
    time_t s = time(nullptr);
    if (s > 1700000000) {
      struct tm t;
      localtime_r(&s, &t);
      timeFromNetwork(t, now);
      ntpDone = true;
    }
  }
  if ((cfg.weatherOn || weatherForced) && (long)(now - weatherAt) >= 0) {
    // Refresh every x22.wxEvery minutes (set in the app). Source: 0 auto = the phone's
    // weather while it is fresh, else the internet; 1 phone only; 2 internet only.
    unsigned long every = (unsigned long)x22.wxEvery * 60000UL;
    bool phoneFresh = wx.valid && wx.fromPhone && now - wx.at < 2 * every;
    bool useNet = x22.wxSource == 2 || (x22.wxSource == 0 && (!phoneFresh || weatherForced));
    if (useNet) {
      bool ok = fetchWeather(now);
      if (ok) weatherArrived(now);
      weatherAt = now + (ok ? every : min(every, 5UL * 60 * 1000));
    } else {
      weatherAt = now + every;
    }
    weatherForced = false;
  }
  if (cfg.issOn && (long)(now - issAt) >= 0) {
    issAt = now + 120000;
    checkSpaceStation(now);
  }
}

#else   // ---------------- test build without Wi-Fi (emulator) ----------------

void startWifi(unsigned long now) { (void)now; Serial.println("Wi-Fi: off in this build"); }
void startSetupHotspot(unsigned long now) { showAlert(AL_WIFI, "No Wi-Fi", "This build has no Wi-Fi.", now); }
void stopSetupHotspot() {}
void updateNetwork(unsigned long now) { (void)now; }

#endif
