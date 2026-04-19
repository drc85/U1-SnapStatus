# snapmakerU1-wled-stauts-bar
wled stauts bar for the snapmakerU1

Based on WLED v2 usermod klipper percentage (https://github.com/Aircoookie/WLED)
and Klipper API integration.

Modified and extended by me :) [drc85]

you need
-ws2812 or similar led strip with 16 led for 27cm!
-esp32 or similar 
i recommend this easy plug and play usb wled controller from aliexpress
https://de.aliexpress.com/item/1005010502929150.html
-the led moutn for under the plate
(makerworld link will follow)

instructions:


download firmware.bin and flash it to your esp32 and setup up the network connection

inside wled usermod there is a new window

<img width="529" height="261" alt="image" src="https://github.com/user-attachments/assets/6a229949-6cae-4b88-bab5-454d0fce1067" />

enter your printer ip , if necceary ur auth api, if u dont have auth - leave empty


after reboting it wil show the status like this...


## Final LED States

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
