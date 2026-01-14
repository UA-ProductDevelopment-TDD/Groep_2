# RobotDawg (WarDog)

## Web-based Control, Vision AI & Radar System

RobotDawg (ook WarDog genoemd) is een experimenteel robotplatform dat een Petoi Bittle X quadruped robot combineert met een webgebaseerde HUD-interface, real-time Vision AI, Bluetooth-besturing en een actieve ToF-radar scanner.

Het project focust op de integratie van sensing, visuele AI en fysieke robotbesturing binnen één centrale webinterface.

---

## Systeemoverzicht

Het systeem bestaat uit drie hoofdcomponenten:

1. Web Interface (Laptop / Browser)
   - Heads-Up Display (HUD)
   - Virtuele joystick en D-pad
   - Live Vision AI stream
   - Radarvisualisatie
   - Web Bluetooth communicatie

2. Scanner Unit (Arduino UNO R4 WiFi)
   - 3× VL53L0X Time-of-Flight sensoren
   - Servo motor voor 180° pan-scan
   - Bluetooth Low Energy (UART)

3. Vision AI Camera (XIAO ESP32-S3 / Grove Vision AI v2)
   - MJPEG videostream
   - AI-objectdetectie
   - Model getraind via SenseCraft AI Studio

---

## Functionaliteiten

### Vision AI Livestream
- Real-time MJPEG videostream
- Objectdetectie met bounding boxes
- Detectiescore wordt doorgestuurd naar de webinterface
- AI-score bepaalt of acties (zoals FIRE) worden toegestaan

### Web-based HUD Interface
- Linker joystick (WALK): lopen, draaien, achteruit
- Rechter D-pad (AIM / Scanner):
  - Links/rechts: pan van de scanner
  - Omhoog/omlaag: houding van Bittle X in 9 discrete stappen
- Actieknoppen: FIRE, RUST, RESET
- Telemetrie en AI probability bar

### Radarvisualisatie
- Actieve ToF-radar met drie sensoren
- Punten vervagen na verloop van tijd
- Ondersteuning voor handmatige en automatische scans

---

## Bluetooth Communicatie

De webinterface verbindt met twee BLE-devices:

- Device 1: Scanner unit
- Device 2: Petoi Bittle X

Communicatie gebeurt via UART-over-BLE met tekstgebaseerde commando’s.

---

## Configuratie

### Camera IP (Web Interface)

In joysticks2_FINAAL.html:

const ip = "192.168.137.248";

### FIRE-actie

sendBLE_2("m0 -80 0 110 0 -80\n");

- -80: startpositie
- 110: schieten
- -80: terug naar start

### WiFi Vision AI Camera

In camera_web_server sketch:

const char* ssid = "JULLIE_WIFI_NAAM";
const char* password = "JULLIE_WIFI_WACHTWOORD";

---

## Getting Started

1. Flash camera_web_server naar XIAO ESP32-S3
2. Noteer camera IP
3. Pas IP aan in joysticks2_FINAAL.html
4. Flash UNO_BLE_afstand_rev25.ino naar Arduino UNO R4 WiFi
5. Zet Bittle X aan
6. Open joysticks2_FINAAL.html in browser
7. Verbind beide Bluetooth devices

---

## Beperkingen

- Web Bluetooth vereist HTTPS of localhost
- Alle apparaten moeten op hetzelfde netwerk zitten
- Correcte I2C-adressering is essentieel

---

## Team

Groep 2 – RobotDawg / WarDog
