# Bambu WLED Status Bar (experimental)
ESP32 WLED usermod adapted from U1 SnapStatus.

## Settings
WLED > Config > Usermods > **BambuLab**:
- **Printer IP**
- **Serial number**
- **LAN access code**

Chamber-light synchronization is always enabled, with no toggle.
When the reported chamber_light is off, the strip is dark. When on,
it shows the current printer animation. Before the first light report,
and when the MQTT connection is lost, the strip stays dark.
Printer shutdown is detected through MQTT disconnect/keepalive, not instantly.
Partial reports preserve the previous light and print state.

Display: static cool blue-white (RGB 200,225,255) for IDLE, STANDBY and RUNNING;
red blinking for FAILED or nonzero print_error; static orange for every other state.
No startup animation, breathing, progress bar or completion timer.
The chamber light remains the master switch, including for error indications.

## Build
This is source code, not a ready-to-flash firmware.
Use an ESP32 WLED checkout supporting REGISTER_USERMOD and ArduinoJson 6.
Copy this folder into WLED's usermods folder and enable it in that WLED
version's usermod build configuration. Build this usermod instead of the
Snapmaker one (both use the same usermod ID).
The library manifest declares PubSubClient v2.8 from its upstream Git repository.
The shipped test build uses the full official Arduino ESP32 2.0.17 framework,
which supplies WiFiClientSecure. The reduced Tasmota framework from the original
Snapmaker build does not provide the required complete TLS client.
See BUILD.md for the tested versions and firmware fingerprint.
The platformio.ini in this repository belongs to the original Snapmaker
build and does not automatically select this new usermod.

## Connection
The controller uses a separate TLS MQTT client to the printer on port 8883,
username bblp, with the LAN access code as password.
It listens to device/SERIAL/report and requests an initial pushall snapshot.
Availability of local reports/initial snapshots depends on model and firmware.
Cloud-mode compatibility and H-series light-node names require device testing.
Only chamber_light on/off reports are currently supported.
TLS is encrypted but the printer's self-signed certificate is not verified.
The access code is saved as a normal WLED usermod setting; do not share config backups.

## Validation
Compiled successfully for classic ESP32 / Gledopto (4 MB flash, DIO, 1.5 MiB OTA slot).
Binary checksum and validation hash verified by esptool.
Host tests passed for partial status reports, static colors over time, all display states,
error blinking, chamber-light priority and disconnection.
Not yet tested on a physical controller or printer.
The WLED Info window shows Bambu MQTT connection and chamber-light status.
Verify build, settings persistence, initial state, partial reports, light off/on
during printing and idle, and reconnect after printer power-off before release.
