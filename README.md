# Groep_2

RobotDawg (WarDog)
Web-based Control, Vision AI & Radar System
RobotDawg (ook WarDog genoemd) is een experimenteel robotplatform dat een Petoi Bittle X quadruped robot combineert met een webgebaseerde HUD-interface, real-time Vision AI, Bluetooth-besturing en een actieve ToF-radar scanner.
Het project focust op de integratie van sensing, visuele AI en fysieke robotbesturing binnen één centrale webinterface.
________________________________________
Systeemoverzicht
Het systeem bestaat uit drie hoofdcomponenten:
1.	Web Interface (Laptop / Browser)
o	Heads-Up Display (HUD)
o	Virtuele joystick en D-pad
o	Live Vision AI stream
o	Radarvisualisatie
o	Web Bluetooth communicatie
2.	Scanner Unit (Arduino UNO R4 WiFi)
o	3× VL53L0X Time-of-Flight sensoren
o	Servo motor voor 180° pan-scan
o	Bluetooth Low Energy (UART)
3.	Vision AI Camera (XIAO ESP32-S3 / Grove Vision AI v2)
o	MJPEG videostream
o	AI-objectdetectie
o	Model getraind via SenseCraft AI Studio
________________________________________
Functionaliteiten
Vision AI Livestream
•	Real-time MJPEG videostream
•	Objectdetectie met bounding boxes
•	Detectiescore wordt doorgestuurd naar de webinterface
•	AI-score bepaalt of acties (zoals FIRE) worden toegestaan
Web-based HUD Interface
De interface is ontworpen als een futuristische HUD en bevat:
Besturing
•	Linker joystick (WALK): lopen, draaien, achteruit
•	Rechter D-pad (AIM / Scanner):
o	Links/rechts: pan van de scanner
o	Omhoog/omlaag: houding van Bittle X in 9 discrete stappen
Actieknoppen
•	FIRE: afschieten van een schijf via servo-sequentie
•	RUST: zet Bittle in rusthouding
•	RESET: reset scanner en robotpositie
Telemetrie
•	Live joystickwaarden (X, Y, snelheid, hoek)
•	Bluetooth status van beide devices
•	AI probability bar met instelbare threshold
Radarvisualisatie
•	Actieve ToF-radar op basis van drie sensoren
•	Punten worden weergegeven op een radarscherm
•	Punten vervagen na verloop van tijd
•	Ondersteuning voor handmatige en automatische scans
________________________________________
Bluetooth Communicatie
De webinterface verbindt met twee afzonderlijke BLE-devices via UART-over-BLE:
Device	Functie
Device 1	Scanner unit (ToF + servo)
Device 2	Petoi Bittle X (beweging & houding)
Communicatie verloopt via tekstgebaseerde commando’s en is rate-limited om stabiliteit te garanderen.
________________________________________
Scanner Unit – Arduino UNO R4 WiFi
Hardware
•	Arduino UNO R4 WiFi
•	3× Adafruit VL53L0X ToF-sensoren
•	1× Servo motor
•	I2C-bus met XSHUT-pinnen
Functionaliteit
•	Automatische 180° scan (SCAN)
•	Handmatige pan-besturing (J2)
•	Gelijktijdige metingen met drie sensoren
•	Verzending van data via BLE
Uitgaand datapakket
hoek2;afstand2;hoek0;afstand0;hoek1;afstand1
Afstanden worden verzonden in centimeters (mm / 10).
Belangrijke aandachtspunten
•	I2C-adressering gebeurt via XSHUT-pinnen
•	Rate limiting voorkomt instabiliteit van de I2C-bus
________________________________________
Vision AI Camera
•	XIAO ESP32-S3 met Grove Vision AI v2
•	Gebaseerd op Seeed Arduino SSCMA library
•	Camera web server voor MJPEG streaming
•	AI-model getraind via SenseCraft AI Studio
________________________________________
Configuratie
Camera IP instellen (Web Interface)
In joysticks2_FINAAL.html moet het IP-adres van de Vision AI camera overeenkomen met het lokale netwerk.
const ip = "192.168.137.248";
________________________________________
FIRE-actie (afschieten schijf)
De FIRE-knop stuurt een servo-sequentie naar Device 2.
//sendBLE_2("ksit\n");
sendBLE_2("m0 -80 0 110 0 -80\n");
Betekenis van de waarden:
•	-80 : startpositie
•	110 : schieten
•	-80 : terugkeren naar startpositie
________________________________________
Bewegingen van Bittle aanpassen
De mapping van joystickhoek naar Petoi-commando’s gebeurt in de linker joystick-logica.
Voorbeelden:
•	kwkF – vooruit lopen
•	kwkL – links stappen
•	kwkR – rechts stappen
•	kcrL – draaien links
•	kcrR – draaien rechts
•	kbk – achteruit
Deze commando’s kunnen vrij aangepast worden naar andere Petoi skills of gaits.
________________________________________
WiFi instellen (Vision AI Camera)
In de camera_web_server sketch voor de XIAO ESP32-S3:
const char* ssid = "JULLIE_WIFI_NAAM";
const char* password = "JULLIE_WIFI_WACHTWOORD";
Publiceer geen echte wachtwoorden in een publieke repository.
________________________________________
Getting Started
Vereisten
Hardware
•	Laptop met Chromium-based browser
•	Petoi Bittle X
•	Arduino UNO R4 WiFi
•	3× VL53L0X ToF-sensoren
•	Servo motor
•	XIAO ESP32-S3 met Grove Vision AI v2
Software
•	Arduino IDE
•	Seeed Arduino SSCMA library
•	ArduinoBLE library
•	VL53L0X library
________________________________________
Installatie & Opstart
1.	Flash de camera_web_server sketch naar de XIAO ESP32-S3
2.	Noteer het IP-adres van de camera
3.	Stel het IP-adres in in joysticks2_FINAAL.html
4.	Flash UNO_BLE_afstand_rev25.ino naar de Arduino UNO R4 WiFi
5.	Zet Bittle X aan en plaats hem op een vlak oppervlak
6.	Open joysticks2_FINAAL.html in de browser
7.	Klik op Connect Devices en verbind beide Bluetooth-units
________________________________________
Basisbediening
•	Gebruik de linker joystick om Bittle te laten lopen
•	Gebruik het D-pad voor scannerpan en houding
•	Controleer of Vision AI beeld zichtbaar is
•	FIRE is alleen actief bij voldoende AI-detectiescore
________________________________________
Beperkingen en aandachtspunten
•	Web Bluetooth werkt enkel via HTTPS of localhost
•	Alle devices moeten zich op hetzelfde netwerk bevinden
•	Bluetooth rate limiting is essentieel voor stabiliteit
•	Correcte I2C-adressering is noodzakelijk voor de scanner
________________________________________
Team
Project ontwikkeld door Groep 2
RobotDawg / WarDog – experimenteel robotplatform voor integratie van robotbesturing, sensing en Vision AI.

PRODUCTARCHITECTUUR
<img width="788" height="873" alt="image" src="https://github.com/user-attachments/assets/82a0cc86-147d-4679-a387-672ef43d0bf5" />
<img width="1163" height="432" alt="image" src="https://github.com/user-attachments/assets/de409080-eccb-481d-8a68-1b75e1fbb586" />
