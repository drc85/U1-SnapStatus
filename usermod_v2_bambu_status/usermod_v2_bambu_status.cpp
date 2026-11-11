#include "wled.h"
#include <WiFiClientSecure.h>
#include <PubSubClient.h>

class bambu_status : public Usermod {
private:
  WiFiClientSecure transport;
  PubSubClient client{transport};
  String printerIp, serial, accessCode;
  bool lightKnown = false;
  bool lightOn = false;
  unsigned long lastConnect = 0;
  bool reconnectNow = true;

  float progress = 0.0f;
  String ps = "idle";

  unsigned long completeUntil = 0;
  bool completeTriggered = false;

  unsigned long lastOk = 0;

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

  // 🔥 NEU: 50-50-75-100-100-50
  void runningLight(uint8_t r, uint8_t g, uint8_t b) {
    uint16_t n = strip.getLengthTotal();
    if (n == 0) return;
    uint16_t pos = (millis() / 150) % n;

    for (uint16_t i = 0; i < n; i++) {
      int d = (int)i - (int)pos;

      if (d == 0 || d == 1) {
        strip.setPixelColor(i, RGBW32(r,g,b,0)); // 100%, 100%
      }
      else if (d == -1) {
        strip.setPixelColor(i, RGBW32(r*3/4,g*3/4,b*3/4,0)); // 75%
      }
      else if (d == -2 || d == 2 || d == 3) {
        strip.setPixelColor(i, RGBW32(r/2,g/2,b/2,0)); // 50%
      }
      else {
        strip.setPixelColor(i, 0);
      }
    }
  }

  // ✔ unverändert (inkl. blink fix)
  void drawProgressBar(float p) {
    uint16_t n = strip.getLengthTotal();
    uint16_t lit = max((uint16_t)1, (uint16_t)(p * n));

    static int lastLit = -1;
    static int blinkIndex = -1;
    static unsigned long blinkUntil = 0;

    if (lastLit != -1 && (int)lit - 1 != lastLit) {
      blinkIndex = lit - 1;
      blinkUntil = millis() + 400;
    }
    lastLit = lit - 1;

    bool blink = ((millis()/100)%2)==0;

    for (uint16_t i = 0; i < n; i++) {
      if (i < lit) {
        if ((int)i == blinkIndex && millis() < blinkUntil) {
          strip.setPixelColor(i, blink ? RGBW32(0,255,0,0) : 0);
        } else {
          strip.setPixelColor(i, RGBW32(0,255,0,0));
        }
      } else {
        strip.setPixelColor(i, RGBW32(180,180,180,0));
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
    lastOk = millis();
  }

  void receiveStatus(char*, byte* payload, unsigned int length) {
    // Bambu sends partial reports. Only replace fields present in this message.
    DynamicJsonDocument filter(512);
    filter["print"]["gcode_state"] = true;
    filter["print"]["mc_percent"] = true;
    filter["print"]["print_error"] = true;
    filter["print"]["lights_report"][0]["node"] = true;
    filter["print"]["lights_report"][0]["mode"] = true;
    DynamicJsonDocument doc(2048);
    if (deserializeJson(doc, payload, length,
                        DeserializationOption::Filter(filter))) return;
    JsonObject report = doc["print"];
    if (report.isNull()) return;
    lastOk = millis();

    if (report.containsKey("mc_percent")) {
      progress = constrain(report["mc_percent"].as<float>() / 100.0f, 0.0f, 1.0f);
    }
    if (report.containsKey("gcode_state")) {
      String state = report["gcode_state"].as<String>();
      if (state == "RUNNING") ps = "printing";
      else if (state == "PREPARE") ps = "preparing";
      else if (state == "PAUSE") ps = "paused";
      else if (state == "FINISH") ps = "complete";
      else if (state == "FAILED") ps = "error";
      else ps = "idle";

      if (ps == "complete") {
        if (!completeTriggered) {
          completeUntil = millis() + 20000;
          completeTriggered = true;
        }
      } else {
        completeTriggered = false;
        completeUntil = 0;
      }
    }
    if (report.containsKey("print_error") &&
        report["print_error"].as<uint32_t>() != 0) ps = "error";

    for (JsonObject light : report["lights_report"].as<JsonArray>()) {
      if (light["node"].as<String>() != "chamber_light") continue;
      String mode = light["mode"].as<String>();
      if (mode == "on" || mode == "off") {
        lightKnown = true;
        lightOn = mode == "on";
      }
    }
  }

  void updateBambuConnection() {
    bool configured = !printerIp.isEmpty() && !serial.isEmpty() && !accessCode.isEmpty();
    if (!configured || WiFi.status() != WL_CONNECTED) {
      client.disconnect();
      lightKnown = false;
      return;
    }
    if (!client.connected()) {
      lightKnown = false;
      if (!reconnectNow && millis() - lastConnect < 10000) return;
      reconnectNow = false;
      lastConnect = millis();
      client.setServer(printerIp.c_str(), 8883);
      String id = "BambuStatus-" + String((uint32_t)ESP.getEfuseMac(), HEX);
      if (!client.connect(id.c_str(), "bblp", accessCode.c_str())) return;
      String reportTopic = "device/" + serial + "/report";
      if (!client.subscribe(reportTopic.c_str())) {
        client.disconnect();
        return;
      }
      // Request the initial snapshot. Some firmware only allows passive reports;
      // light sync remains dark until an actual chamber light report arrives.
      String requestTopic = "device/" + serial + "/request";
      client.publish(requestTopic.c_str(),
        "{\"pushing\":{\"sequence_id\":\"0\",\"command\":\"pushall\"}}");
    }
    client.loop();
  }

public:
  void setup() override {
    startupStart = millis();
    lastOk = millis();
    // Bambu printers use a self-signed TLS certificate.
    transport.setInsecure();
    transport.setTimeout(2);
    transport.setHandshakeTimeout(2);
    client.setSocketTimeout(2);
    client.setBufferSize(16384);
    client.setCallback([this](char* topic, byte* payload, unsigned int length) {
      receiveStatus(topic, payload, length);
    });
  }

  void loop() override {
    updateBambuConnection();
  }

  void handleOverlayDraw() override {

    // Gate the final output, leaving animation/status updates intact.
    if (!client.connected() || !lightKnown || !lightOn) {
      fillAll(0);
      return;
    }

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

    if (ps == "preparing" || (ps == "printing" && progress < 0.01f)) {
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
    top["Printer IP"] = printerIp;
    top["Serial number"] = serial;
    top["LAN access code"] = accessCode;
  }

  bool readFromConfig(JsonObject &root) override {
    JsonObject top = root[FPSTR(_name)];
    if (top.isNull()) return false;

    String newIp = top["Printer IP"] | "";
    String newSerial = top["Serial number"] | "";
    String newCode = top["LAN access code"] | "";
    if (newIp != printerIp || newSerial != serial || newCode != accessCode) {
      client.disconnect();
      lightKnown = false;
      reconnectNow = true;
    }
    printerIp = newIp;
    serial = newSerial;
    accessCode = newCode;
    return true;
  }

  void addToJsonInfo(JsonObject &root) override {
    JsonObject info = root["u"];
    if (info.isNull()) info = root.createNestedObject("u");
    JsonArray connection = info.createNestedArray("Bambu MQTT");
    connection.add(client.connected() ? "Connected" : "Disconnected");
    JsonArray light = info.createNestedArray("Bambu chamber light");
    light.add(!client.connected() || !lightKnown ? "Unknown" : (lightOn ? "On" : "Off"));
  }

  uint16_t getId() override { return 0xBEEF; }
};

const char bambu_status::_name[] PROGMEM = "BambuLab";
static bambu_status usermod_bambu;
REGISTER_USERMOD(usermod_bambu);
