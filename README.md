# Snapmaker U1 WLED Status Bar
WLED-based LED status bar for the Snapmaker U1.

Based on the WLED v2 usermod *Klipper Percentage*  
(https://github.com/Aircoookie/WLED) with Klipper API integration.

Modified and extended by me **drc85**

---

<img width="720" height="960" alt="676632991_3969606846517215_7601965848291422590_n" src="https://github.com/user-attachments/assets/52d1cf9e-55b8-4beb-8536-9b3d8c29c9f0" />


## 🔧 Requirements

- WS2812 (or similar) LED strip  
 → **16 LEDs for ~27cm width**
- ESP32 (or compatible)
- Optional: ready-to-use USB WLED controller  
 → https://de.aliexpress.com/item/1005010502929150.html
- 3 x 5mm - m3 pointed flathead screws
- double sided 3M VHB tape
- led mounting case  (two options availible - under the build plate and directly on the bottom of the printer)
 → https://makerworld.com/de/models/2686608-snapmaker-u1-led-status-bar-wled-esp32-gledopto#profileId-2976228


  <img width="529" height="261" alt="Screenshot 2026-04-19 194245" src="https://github.com/user-attachments/assets/a6e5ad3b-25d1-4d37-b5a5-b3bbbb56d3e9" />
  
---

## 🚀 Installation

### 1. Flash firmware
- Download `firmware.bin`:  
  https://github.com/drc85/snapmakerU1-wled-stauts-bar
- Flash it to your ESP32 (e.g. via esptool or web flasher)

### 2. Set up WLED
- Connect to WiFi
- Open WLED Web UI

### 3. Configure Usermod
- Navigate to **Usermods**
- New section: **SnapmakerU1**

**Enter:**
- Printer IP  
- API Key *(optional, leave empty if not used)*  

➡️ Reboot the device (setup takes max ~10 minutes)

---

### 4. Print case
- MakerWorld:  
  https://makerworld.com/de/models/2686608-snapmaker-u1-led-status-bar-wled-esp32-gledopto#profileId-2976228

**Available options:**
- Under the build plate  
- Directly on the bottom of the printer  

---

### 5. Assemble hardware
1. Place controller and LED inside  
2. Screw on the top part  
3. Add 3M VHB tape  
4. Mount it inside the printer  

---

### 6. Cable routing
- Drill a hole (bottom or back wall)  
**or**  
- Route the cable through the side (with the build plate cable)

 ---
 

## 💡 LED Status Overview

- ⚪ **Idle / Standby**  
 Solid white

- 🟡 **Preparing / Preheat / Calibration**  
 Smooth pulsing orange (dim ↔ bright, never fully off)

- 🟢 **Printing**  
 - Progress LEDs = green  
 - Remaining LEDs = full white  
 - Always at least 1 LED active (16 LEDs total)

- 🔵 **Paused**  
 Smooth pulsing blue (dim ↔ bright, never fully off)

- 🔴 **Error**  
 Flashing red (dim ↔ bright, never fully off)

- 🟢 **Complete**  
 Flashing green (~10 seconds), then returns to idle
