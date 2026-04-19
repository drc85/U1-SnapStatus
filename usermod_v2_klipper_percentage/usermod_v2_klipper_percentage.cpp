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

  static const char _name[];
  bool enabled = true;

  uint8_t base = 40; // Hintergrund hell (weißlich)

  void fillAll(uint32_t color) {
    uint16_t count = strip.getLengthTotal();
    for (uint16_t i = 0; i < count; i++) strip.setPixelColor(i, color);
  }

  // 👉 CLEAN PROGRESS BAR
  void drawProgressBar(float p) {
    uint16_t count = strip.getLengthTotal();
    if (count == 0) return;

    p = constrain(p, 0.0f, 1.0f);

    // 👉 mindestens 1 LED immer aktiv
    uint16_t lit = max(1, (uint16_t)(p * count));

    for (uint16_t i = 0; i < count; i++) {
      if (i < lit) {
        // Fortschritt = grün
        strip.setPixelColor(i, RGBW32(0,255,0,0));
      } else {
        // Hintergrund = leicht weiß
        strip.setPixelColor(i, RGBW32(base, base, base, 0));
      }
    }
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

        JsonObject status = doc["result"]["status"];

        ps = status["print_stats"]["state"] | "idle";
        progress = status["display_status"]["progress"] | 0.0f;

        float bt = status["heater_bed"]["target"] | 0.0f;
        float bc = status["heater_bed"]["temperature"] | 0.0f;
        float et = status["extruder"]["target"] | 0.0f;
        float ec = status["extruder"]["temperature"] | 0.0f;

        heating = (bt > bc + 2.0f) || (et > ec + 2.0f);

        if (ps == "complete") {
          if (completeUntil == 0) completeUntil = millis() + 10000;
        } else {
          completeUntil = 0;
        }
      }
    }

    http.end();
  }

public:
  void setup() override {}

  void loop() override {
    if (!enabled) return;

    if (millis() - lastPoll > 1200) {
      lastPoll = millis();
      updateSnapmakerStatus();
    }
  }

  void handleOverlayDraw() override {
    if (!enabled) return;

    float pulse = (sin(millis() * 0.002f) + 1.0f) * 0.5f;
    bool blink = ((millis() / 500) % 2) == 0;

    // 🔴 Error
    if (ps == "error") {
      uint8_t v = blink ? 255 : base;
      fillAll(RGBW32(v,0,0,0));
      return;
    }

    // 🔵 Pause
    if (ps == "paused") {
      uint8_t v = base + (uint8_t)(200 * pulse);
      fillAll(RGBW32(0,0,v,0));
      return;
    }

    // 🟢 Complete
    if (completeUntil > 0) {
      if (millis() < completeUntil) {
        uint8_t v = blink ? 255 : base;
        fillAll(RGBW32(0,v,0,0));
        return;
      }
      completeUntil = 0;
    }

    // 🟡 Preheat (extended)
    if (ps == "printing" && progress < 0.01f) {
      uint8_t r = base + (uint8_t)(200 * pulse);
      uint8_t g = base + (uint8_t)(100 * pulse);
      fillAll(RGBW32(r,g,0,0));
      return;
    }

    // 🟢 Printing
    if (ps == "printing") {
      drawProgressBar(progress);
      return;
    }

    // 🟡 Heating fallback
    if (heating) {
      uint8_t r = base + (uint8_t)(200 * pulse);
      uint8_t g = base + (uint8_t)(100 * pulse);
      fillAll(RGBW32(r,g,0,0));
      return;
    }

    // 🔵 Idle
    fillAll(RGBW32(0,180,255,0));
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

  uint16_t getId() override {
    return 0xBEEF;
  }
};

const char snapmaker_u1::_name[] PROGMEM = "SnapmakerU1";

static snapmaker_u1 usermod_snapmaker;
REGISTER_USERMOD(usermod_snapmaker);
