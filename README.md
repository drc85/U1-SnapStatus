#Snapmaker U1 WLED Status Bar 
-----------------------------
U1Snapstatus V1.0
---------------------------
WLED-based LED status bar for the Snapmaker U1.

Based on the WLED v2 usermod *Klipper Percentage*  
(https://github.com/Aircoookie/WLED) with Klipper API integration.

Modified and extended by me **drc85**
NOTE - all normal wled functions are disabled so there are nor crashes!!!

---

<img width="416" height="179" alt="image" src="https://github.com/user-attachments/assets/edb1f97b-8a48-471c-828a-3d9becb80497" />


 ---
 
## 🚦 LED States

## 🌊 Startup Animation (16 LEDs)

**⚪ Idle**  
White breathing (smooth pulse)

**🟡 Preparing / Preheat**  
Orange running light (→)  
Pattern: 50% – 50% – 75% – 100% -100% – 50%

**🟢 Printing**  
Green progress bar  
- New LED → blinks 2×  
- Existing LEDs → stay solid  
- Remaining LEDs → dim white

**🔵 Paused**  
Blue running light (same pattern as preheat)

**🔴 Error**  
Fast red blinking (on/off)

**🟢 Complete**  
Fast green blinking (~20s) → returns to Idle

---


## 🔧 Requirements

- WS2812 (or similar) LED strip  
 → **16 LEDs for ~27cm width**
- ESP32 (or compatible)
- Optional: ready-to-use USB WLED controller  
 → https://de.aliexpress.com/item/1005010502929150.html
- 3 x 5mm - m3 pointed flathead screws
- double sided 3M VHB tape
- led mounting case two options availible
- - one version with mount  for under the buildplate,
- - the other version is a stand lone version , works also on the bottom or top of the printer or in any room with the same wifi!
 → https://makerworld.com/de/models/2686608-snapmaker-u1-led-status-bar-wled-esp32-gledopto#profileId-2976228
- paxx firmware
→ https://github.com/paxx12/SnapmakerU1-Extended-Firmware

  
---

## 🚀 Installation

NOTE: 
-FOR KLIPPER - you have to be on Paxx FW 
→ https://github.com/paxx12/SnapmakerU1-Extended-Firmware


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

  
  <img width="420" height="261" alt="Screenshot 2026-04-19 194245" src="https://github.com/user-attachments/assets/a6e5ad3b-25d1-4d37-b5a5-b3bbbb56d3e9" />

**Enter:**
- Printer IP  
- API Key *(optional, leave empty if not used)
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
<img width="420" height="180" alt="image" src="https://github.com/user-attachments/assets/ea522f5e-bd8b-4361-b8d0-978f5f514507" />
<img width="420" height="180" alt="image" src="https://github.com/user-attachments/assets/8d13583c-fad0-4b21-b87c-3d6b429655d4" />


---

### 6. Cable routing
- Route the cable through the ride side wall (with the build plate cable)
-when the sidewall is removed u can see flat cable goin through the bottom, u can use this hole so no drilling is required!!
- for buildplate moutn fix the calbe ont he buildplate cable 
-for the stand version if u place it on the bottom of the printer u can just pull it through!
https://www.youtube.com/watch?v=GgDL4Bqjk_Y
<img width="420" height="180" alt="image" src="https://github.com/user-attachments/assets/5919114d-338d-4bf7-b61f-66079326c8c5" />

