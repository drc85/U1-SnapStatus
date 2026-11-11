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

Animations: idle white pulse; prepare orange chase; print green progress;
pause blue chase; failure red blink; finish green blink for 20 seconds.
The existing 16-LED startup animation is retained and is also gated by the light.

## Build
This is source code, not a ready-to-flash firmware.
Use an ESP32 WLED checkout supporting REGISTER_USERMOD and ArduinoJson 6.
Copy this folder into WLED's usermods folder and enable it in that WLED
version's usermod build configuration. Build this usermod instead of the
Snapmaker one (both use the same usermod ID).
The library manifest declares PubSubClient 2.8.
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
Experimental: not compiled against a WLED checkout and not tested on a printer.
Verify build, settings persistence, initial state, partial reports, light off/on
during printing and idle, and reconnect after printer power-off before release.
