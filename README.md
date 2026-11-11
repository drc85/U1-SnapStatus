# Bambu WLED Status Bar – Testversion

Bambu-Anpassung von **U1 SnapStatus** für einen **Gledopto-Controller mit klassischem ESP32**.

Die Leiste zeigt die Drucker-Statusanimationen. Ihre Beleuchtung ist immer an das gemeldete Kammerlicht gekoppelt – ohne zusätzlichen Schalter:

- **Kammerlicht an:** Die Leiste zeigt den aktuellen Druckstatus.
- **Kammerlicht aus:** Die Leiste bleibt dunkel.
- **Keine MQTT-Verbindung oder noch kein Lichtstatus:** Die Leiste bleibt dunkel.
- Bei ausgeschaltetem Drucker wird die Leiste nach Erkennung des Verbindungsabbruchs dunkel; das kann etwas dauern.

## 1. Firmware herunterladen

Dateiname: **`Bambu-WLED-Gledopto-ESP32-test.bin`**

Die fertige Datei wurde für den ersten Test direkt im Projektgespräch bereitgestellt. Es gibt noch kein öffentliches GitHub-Release mit dieser Bambu-Firmware. Der Quellcode allein ist keine Datei für den WLED-Update-Dialog.

Diese Datei ist ein **OTA-Update für den klassischen ESP32**. Sie ist keine vollständige USB-Erstinstallationsdatei und nicht für ESP8266, ESP32-C3, ESP32-S2 oder ESP32-S3 bestimmt.

## 2. WLED aktualisieren

1. Die Weboberfläche deines bestehenden WLED-Controllers öffnen.
2. Unter **Config → Security & Updates** zunächst die Konfiguration sichern (**Backup configuration**). Falls Presets verwendet werden, auch diese sichern.
3. **Manual OTA Update** öffnen. Je nach WLED-Version heißt der Punkt etwas anders.
4. Die heruntergeladene `Bambu-WLED-Gledopto-ESP32-test.bin` auswählen und hochladen.
5. Die Stromversorgung bis zum abgeschlossenen Update und Neustart angeschlossen lassen.
6. WLED danach erneut öffnen. Falls noch die alte Oberfläche angezeigt wird, die Seite vollständig neu laden.

Die vorhandenen LED-Einstellungen sollten beim OTA-Update erhalten bleiben. Prüfe danach unter **Config → LED Preferences** den LED-Typ, den Datenpin und die LED-Anzahl. Übernimm die Werte deiner funktionierenden Snapmaker-Konfiguration.

## 3. Bambu-Drucker einrichten

Controller und Drucker müssen sich im selben lokalen Netzwerk erreichen können.

1. Am Drucker **IP-Adresse**, **Seriennummer** und **LAN-Zugangscode** nachsehen. Die Menüpunkte hängen vom Druckermodell und der Firmware ab.
2. In WLED **Config → Usermods → BambuLab** öffnen.
3. Diese Felder ausfüllen:

| Feld | Eingabe |
| --- | --- |
| Printer IP | Lokale IP-Adresse des Bambu-Druckers, ohne `http://` |
| Serial number | Seriennummer des Druckers |
| LAN access code | LAN-Zugangscode des Druckers, nicht das Bambu-Kontopasswort |

4. Speichern. Falls die Verbindung nicht startet, den Controller einmal neu starten.
5. Das Kammerlicht am Drucker einmal aus- und wieder einschalten.

Die Lichtkopplung ist fest aktiv. Ein Snapmaker-API-Key oder Moonraker wird nicht benötigt.

Die Verbindung läuft über lokales TLS-MQTT auf Port **8883**. Ob die benötigten Statusmeldungen im verwendeten Cloud-/LAN-Modus verfügbar sind, hängt vom Druckermodell und dessen Firmware ab. Die Kompatibilität mit dem konkreten Drucker muss noch getestet werden; ein bestimmter Modus wird hier nicht als Voraussetzung zugesichert.

## 4. Erster Funktionstest

1. In WLED das **Infofenster** öffnen:
   - **Bambu MQTT** sollte **Connected** anzeigen.
   - **Bambu chamber light** sollte **On** oder **Off** anzeigen.
2. Im Standby das Kammerlicht ausschalten: Die externe Leiste muss dunkel werden.
3. Kammerlicht einschalten: Die Leiste muss wieder den aktuellen Status anzeigen.
4. Einen Druck starten und die Statusanzeige prüfen.
5. Während des Drucks das Kammerlicht aus- und einschalten: Die Leiste soll anschließend den aktuellen Fortschritt anzeigen.
6. Drucker ausschalten: Nach Erkennung des Verbindungsabbruchs soll die Leiste dunkel werden.

Der Controller läuft mit seiner eigenen Stromversorgung. Ein Eingriff in die Druckerelektrik ist nicht erforderlich.

## Statusanimationen

| Druckerstatus | Anzeige |
| --- | --- |
| Start des Controllers | Bisherige 16-LED-Startanimation, sofern das Kammerlicht bereits als eingeschaltet gemeldet wird |
| Standby | Weißes Pulsieren |
| Vorbereitung | Oranges Lauflicht |
| Drucken | Grüner Fortschrittsbalken, verbleibender Bereich weiß |
| Pause | Blaues Lauflicht |
| Fehler | Rotes Blinken |
| Fertig | Etwa 20 Sekunden grünes Blinken, danach Standby |

Ein ausgeschaltetes Kammerlicht hat immer Vorrang vor der Statusanimation.

## Wenn die Leiste dunkel bleibt

- **MQTT: Disconnected:** IP, Seriennummer, LAN-Zugangscode und lokale Erreichbarkeit prüfen.
- **Lichtstatus: Unknown:** Es wurde noch kein unterstützter Lichtstatus empfangen. Kammerlicht einmal aus-/einschalten und die Meldung erneut prüfen.
- **Connected und On, aber keine LEDs:** WLED muss eingeschaltet sein und eine Helligkeit größer null haben. LED-Typ, Anzahl, Datenpin und Stromversorgung prüfen.
- Derzeit wird der Lichtknoten **`chamber_light`** mit **`on`/`off`** ausgewertet. Abweichende Lichtknoten neuer Druckermodelle müssen gegebenenfalls ergänzt werden.

Der LAN-Zugangscode wird als normale WLED-Konfiguration gespeichert. Konfigurationssicherungen daher nicht öffentlich teilen.

## Stand der Testversion

- Firmware für klassischen ESP32 erfolgreich kompiliert.
- Firmwaregröße: **1.503.552 Byte**, passend zum verwendeten 1,5-MiB-OTA-Slot.
- Image-Prüfsumme und Validierungshash mit esptool geprüft.
- Einbindung des Bambu-Usermods im fertigen Programm bestätigt.
- Verarbeitung von Teilmeldungen, Lichtzuständen und Druckstatus in Host-Tests geprüft.
- **Noch kein Test auf einem echten Controller oder Bambu-Drucker.**

Ungenutzte Matrix- und Partikeleffekte wurden für diese Firmware ausgeschlossen, damit sie in den OTA-Speicher passt. Die Statusanimationen bleiben enthalten.

SHA-256 der Update-Datei:

```text
44422656dfe659905d70b3a612203457a4de58cbb8f5337e3264a209599b44dc
```

## Quellcode

- [Bambu-Usermod](usermod_v2_bambu_status/usermod_v2_bambu_status.cpp)
- [Build-Versionen und technische Details](usermod_v2_bambu_status/BUILD.md)
- [Originale Snapmaker-Version auf main](https://github.com/drc85/U1-SnapStatus/tree/main)

Diese Anleitung gehört zum Branch **`bambu-chamber-light-sync`**.
