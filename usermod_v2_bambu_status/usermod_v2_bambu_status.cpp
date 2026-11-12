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

  String ps = "IDLE";
  uint32_t printError = 0;

  static const char _name[];

  void fillAll(uint32_t c) {
    uint16_t n = strip.getLengthTotal();
    for (uint16_t i = 0; i < n; i++) strip.setPixelColor(i, c);
  }

  void receiveStatus(char*, byte* payload, unsigned int length) {
    // Bambu sends partial reports. Only replace fields present in this message.
    DynamicJsonDocument filter(512);
    filter["print"]["gcode_state"] = true;
    filter["print"]["print_error"] = true;
    filter["print"]["lights_report"][0]["node"] = true;
    filter["print"]["lights_report"][0]["mode"] = true;
    DynamicJsonDocument doc(2048);
    if (deserializeJson(doc, payload, length,
                        DeserializationOption::Filter(filter))) return;
    JsonObject report = doc["print"];
    if (report.isNull()) return;
    if (report.containsKey("gcode_state")) {
      ps = report["gcode_state"].as<String>();
    }
    if (report.containsKey("print_error")) {
      printError = report["print_error"].as<uint32_t>();
    }

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
    // Chamber light is the master switch, including for error indications.
    if (!client.connected() || !lightKnown || !lightOn) {
      fillAll(0);
      return;
    }
    if (printError != 0 || ps == "FAILED") {
      bool blink = ((millis() / 200) % 2) == 0;
      fillAll(blink ? RGBW32(255, 0, 0, 0) : 0);
    } else if (ps == "IDLE" || ps == "STANDBY" || ps == "RUNNING") {
      fillAll(RGBW32(200, 225, 255, 0)); // Cool white with a subtle blue tint.
    } else {
      fillAll(RGBW32(255, 120, 0, 0)); // Preparation, pause, finished and other states.
    }
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
