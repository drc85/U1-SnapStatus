#include "wled.h"
#include <HTTPClient.h>

class snapmaker_u1 : public Usermod {
private:
  unsigned long lastPoll = 0;
  String snapIp = "";
  String apiKey = "";
  uint16_t snapPort = 7125;

  float progress = 0.0f;
  String ps = "idle";
  bool heating = false;

  unsigned long completeUntil = 0;
  bool completeTriggered = false;

  unsigned long lastOk = 0;

  // Startup
  bool startupActive = true;
  unsigned long startupStart = 0;

  static const char _name[];

  void fillAll(uint32_t c) {
    uint16_t n = strip.getLengthTotal();
    for (uint16_t i = 0; i < n; i++) strip.setPixelColor(i, c);
  }

  void idlePulse() {
    float t = millis() * 0.0012f;
    float p = (sin(t) + 1.0f) * 0.5f;
    p = p * p;
    uint8_t v = 120 + (uint8_t)(100 * p);
    fillAll(RGBW32(v,v,v,0));
  }

  // 🔥 geändert: running light pattern
  void runningLight(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t n = strip.getLengthTotal();
    uint16_t pos = (millis() / 150) % n;

    for (uint16_t i = 0; i < n; i++) {
      int d = (int)i - (int)pos;

      if (d == 0) strip.setPixelColor(i, RGBW32(r,g,b,0));                 // 100%
      else if (d == -1) strip.setPixelColor(i, RGBW32(r*3/4,g*3/4,b*3/4,0));// 75%
      else if (d == -2 || d == 1 || d == 2)
        strip.setPixelColor(i, RGBW32(r/2,g/2,b/2,0));                      // 50%
      else strip.setPixelColor(i, 0);
    }
  }

  // 🔥 geändert: neue LED blinkt 2x
  void drawProgressBar(float p) {
    uint16_t n = strip.getLengthTotal();
    uint16_t lit = max((uint16_t)1, (uint16_t)(p * n));

    static int lastLit = -1;
    static int blinkIndex = -1;
    static unsigned long blinkUntil = 0;

    if ((int)lit - 1 != lastLit) {
      blinkIndex = lit - 1;
      blinkUntil = millis() + 400;
      lastLit = lit - 1;
    }

    bool blink = ((millis()/100)%2)==0;

    for (uint16_t i = 0; i < n; i++) {
      if (i < lit) {
        if ((int)i == blinkIndex && millis() < blinkUntil) {
          strip.setPixelColor(i, blink ? RGBW32(0,255,0,0) : 0);
        } else {
          strip.setPixelColor(i, RGBW32(0,255,0,0));
        }
      } else {
        strip.setPixelColor(i, RGBW32(180,180,180,0)); // unverändert
      }
    }
  }

  void startupWave() {
    const int midL = 7;
    const int midR = 8;

    unsigned long t = millis() - startupStart;

    fillAll(0);

    if (t < 5000) {
      int step = (t * 8) / 5000;
      strip.setPixelColor(step, RGBW32(255,255,255,0));
      strip.setPixelColor(15-step, RGBW32(255,255,255,0));
      return;
    }

    if (t < 10000) {
      int width = ((t-5000) * 8) / 5000;

      strip.setPixelColor(midL, RGBW32(255,255,255,0));
      strip.setPixelColor(midR, RGBW32(255,255,255,0));

      for (int i = 1; i <= width; i++) {
        strip.setPixelColor(midL-i, RGBW32(255,255,255,0));
        strip.setPixelColor(midR+i, RGBW32(255,255,255,0));
      }
      return;
    }

    startupActive = false;
    ps = "idle";
    lastOk = millis();
  }

  void updateSnapmakerStatus() {
    if (snapIp.isEmpty() || WiFi.status() != WL_CONNECTED) return;

    HTTPClient http;
    String url = "http://" + snapIp + ":" + String(snapPort) +
      "/printer/objects/query?print_stats&display_status&heater_bed&extruder";

    if (!http.begin(url)) return;

    if (!apiKey.isEmpty()) http.addHeader("X-Api-Key", apiKey);

    int code = http.GET();

    if (code == 200) {
      StaticJsonDocument<2048> doc;

      if (!deserializeJson(doc, http.getStream())) {

        JsonObject s = doc["result"]["status"];

        ps = s["print_stats"]["state"] | "idle";
        progress = s["display_status"]["progress"] | 0.0f;

        float bt = s["heater_bed"]["target"] | 0;
        float bc = s["heater_bed"]["temperature"] | 0;
        float et = s["extruder"]["target"] | 0;
        float ec = s["extruder"]["temperature"] | 0;

        heating = (bt > bc + 2) || (et > ec + 2);

        if (ps == "complete") {
          if (!completeTriggered) {
            completeUntil = millis() + 10000;
            completeTriggered = true;
          }
        } else {
          completeTriggered = false;
          completeUntil = 0;
        }

        lastOk = millis();
      }
    }

    http.end();
  }

public:
  void setup() override {
    startupStart = millis();
    lastOk = millis();
  }

  void loop() override {
    if (millis() - lastPoll > 1200) {
      lastPoll = millis();
      updateSnapmakerStatus();
    }

    if (!startupActive && millis() - lastOk > 8000) {
      ps = "error";
    }
  }

  void handleOverlayDraw() override {

    if (startupActive) {
      startupWave();
      return;
    }

    bool blink = ((millis()/200)%2)==0;

    if (ps == "error") {
      fillAll(blink ? RGBW32(255,0,0,0) : 0);
      return;
    }

    if (ps == "paused") {
      runningLight(0,0,255);
      return;
    }

    if (completeUntil > 0) {
      if (millis() < completeUntil) {
        fillAll(blink ? RGBW32(0,255,0,0) : 0);
        return;
      } else {
        completeUntil = 0;
        ps = "idle";
      }
    }

    if (ps == "printing" && progress < 0.01f) {
      runningLight(255,120,0);
      return;
    }

    if (heating && progress < 0.01f) {
      runningLight(255,120,0);
      return;
    }

    if (ps == "printing") {
      drawProgressBar(progress);
      return;
    }

    idlePulse();
  }

  void addToConfig(JsonObject &root) override {
    JsonObject top = root.createNestedObject(FPSTR(_name));
    top["ip"] = snapIp;
    top["port"] = snapPort;
    top["api"] = apiKey;
  }

  bool readFromConfig(JsonObject &root) override {
    JsonObject top = root[FPSTR(_name)];
    if (top.isNull()) return false;

    snapIp = top["ip"] | "";
    snapPort = top["port"] | 7125;
    apiKey = top["api"] | "";
    return true;
  }

  uint16_t getId() override { return 0xBEEF; }
};

const char snapmaker_u1::_name[] PROGMEM = "SnapmakerU1";
static snapmaker_u1 usermod_snapmaker;
REGISTER_USERMOD(usermod_snapmaker);
