# Test firmware build record

Target: classic ESP32, Gledopto controller previously running the Snapmaker binary.
OTA application image only; upload through WLED Manual OTA Update.

- WLED commit: 259bf3c0f84b76c86c99b1eb26fd86e5a7afd221
- Board: esp32dev, DIO, 4 MB, WLED_ESP32_4MB_1MB_FS.csv (0x180000 application slots)
- Full official Espressif Arduino ESP32 2.0.17 framework (ESP-IDF 4.4.7)
- Compiler: Espressif GCC 8.4.0 esp-2021r2-patch5
- NeoPixelBus: a0919d1c10696614625978dd6fb750a1317a14ce
- ESPAsyncWebServerWLED: v2.4.2
- AsyncTCP: v3.4.7
- PubSubClient: v2.8
- Only the Bambu usermod is selected (custom_usermods = bambu_status).
- WiFiClientSecure comes from the full official framework, not a separate patched library.
- Static blue-white for IDLE/STANDBY/RUNNING, static orange otherwise, red blinking for errors.
No startup animation, progress bar, breathing or completion timer.

Additional flags: ARDUINO_ARCH_ESP32, ESP32, ARDUINO_USB_CDC_ON_BOOT=0,
CONFIG_ASYNC_TCP_USE_WDT=0, CONFIG_ASYNC_TCP_STACK_SIZE=8192,
WLED_DISABLE_ALEXA, WLED_DISABLE_BLYNK, WLED_DISABLE_HUESYNC,
WLED_DISABLE_INFRARED, WLED_DISABLE_DMX, WLED_DISABLE_ESPNOW,
WLED_DISABLE_MQTT (only WLED native MQTT; the Bambu PubSubClient is independent),
WLED_DISABLE_2D, WLED_DISABLE_PARTICLESYSTEM1D,
WLED_DISABLE_PARTICLESYSTEM2D, WLED_DISABLE_PIXELFORGE.
Matrix/particle features were excluded to fit the existing OTA slot.

Binary: Bambu-WLED-Gledopto-ESP32-test.bin
Size: 1498624 bytes
SHA256: 4b85620ab8cab000d9d33af2143aa6ea01491fac99dfd70f08507308619896ba
esptool image checksum and validation hash: valid.
WLED post-link verification confirmed the Bambu usermod is present.

Physical testing is still required. Back up the WLED configuration before OTA.
Set BambuLab > Printer IP, Serial number and LAN access code, then save.
The strip remains dark until connected and an actual chamber_light=on report arrives.
The Info window displays the MQTT connection and last known light state.
