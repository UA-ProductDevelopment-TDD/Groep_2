#include <Wire.h> 
#include "Adafruit_VL53L0X.h"
#include <Servo.h>        // Standaard Arduino Servo library
#include <ArduinoBLE.h>   // Specifiek voor UNO R4 WiFi

// --- CONTROLE VARIABELEN ---
String inputBuffer = ""; 
unsigned long laatsteOntvangstTijd = 0;
bool startScanVlag = false; 

// --- BLE DEFINITIES ---
// Gebruik dezelfde UUID's als in je originele code
BLEService uartService("6E400001-B5A3-F393-E0A9-E50E24DCCA9E");
BLEStringCharacteristic txCharacteristic("6E400003-B5A3-F393-E0A9-E50E24DCCA9E", BLENotify, 512);
BLEStringCharacteristic rxCharacteristic("6E400002-B5A3-F393-E0A9-E50E24DCCA9E", BLEWrite, 512);

#define LOX0_ADDRESS 0x30
#define LOX1_ADDRESS 0x31
#define LOX2_ADDRESS 0x32
#define SHT_LOX0 2 
#define SHT_LOX1 4 
#define SHT_LOX2 3 

// --- HARDWARE OBJECTEN ---
Adafruit_VL53L0X lox0, lox1, lox2;
VL53L0X_RangingMeasurementData_t measure0, measure1, measure2; 
Servo myservo;  
int servoPin = 8; 

const int MIN_HOEK = 0;     
const int MAX_HOEK = 180;    
const int RUST_STAND = 90;   
const int STAPGROOTTE = 4;
int huidigePositie = 90; 

void sendBLEData(String data) {
  if (BLE.connected()) {
    txCharacteristic.writeValue(data);
  }
}

void verwerkBericht(String bericht) {
  bericht.trim();
  if (bericht.length() == 0) return;

  Serial.print("\n>>> Ontvangen commando: [");
  Serial.print(bericht);
  Serial.println("]");

  if (bericht == "SCAN") {
    startScanVlag = true; 
  }
  if (bericht == "RESET") {
    myservo.write(RUST_STAND);
    huidigePositie = RUST_STAND;
  }

  if (bericht.startsWith("J2")){
    int eerstePuntkomma = bericht.indexOf(';');
    int tweedePuntkomma = bericht.indexOf(';', eerstePuntkomma + 1);
    int derdePuntkomma  = bericht.indexOf(';', tweedePuntkomma + 1);
    int vierdePuntkomma  = bericht.indexOf(';', derdePuntkomma + 1);

    int J2_speed = bericht.substring(derdePuntkomma + 1, vierdePuntkomma).toInt();
    int J2_angle = bericht.substring(vierdePuntkomma + 1).toInt();

    if (J2_angle >= 170 && J2_angle <= 190){
      beweegServo(-J2_speed/10);
    }
    if (J2_angle > 350 || J2_angle < 10){
      beweegServo(J2_speed/10);
    }
  }
}

void setID() {
  pinMode(SHT_LOX0, OUTPUT);
  pinMode(SHT_LOX1, OUTPUT);
  pinMode(SHT_LOX2, OUTPUT);
  
  digitalWrite(SHT_LOX0, LOW);
  digitalWrite(SHT_LOX1, LOW);
  digitalWrite(SHT_LOX2, LOW);
  delay(10);
  
  digitalWrite(SHT_LOX0, HIGH);
  if(!lox0.begin(LOX0_ADDRESS)) { Serial.println(F("Failed LOX0")); while(1); }
  delay(10);
  digitalWrite(SHT_LOX1, HIGH);
  if(!lox1.begin(LOX1_ADDRESS)) { Serial.println(F("Failed LOX1")); while(1); }
  delay(10);
  digitalWrite(SHT_LOX2, HIGH);
  if(!lox2.begin(LOX2_ADDRESS)) { Serial.println(F("Failed LOX2")); while(1); }
}

void setup() {
  Serial.begin(115200);
  
  // Initialiseer BLE
  if (!BLE.begin()) {
    Serial.println("Starten van BLE mislukt!");
    while (1);
  }

  BLE.setLocalName("UNO_R4_LIDAR_BLE");
  BLE.setAdvertisedService(uartService);
  uartService.addCharacteristic(txCharacteristic);
  uartService.addCharacteristic(rxCharacteristic);
  BLE.addService(uartService);
  BLE.advertise();

  setID();
  
  myservo.attach(servoPin);
  myservo.write(180 - RUST_STAND); 
  
  Serial.println("Bluetooth device active, waiting for connections...");
}

void voerScanUit() {
  Serial.println("--- SCAN START ---");
  myservo.write(MIN_HOEK); 
  delay(400);

  for (int hoek = MIN_HOEK; hoek <= MAX_HOEK; hoek += STAPGROOTTE) {
    myservo.write(hoek);
    delay(20); 

    lox2.rangingTest(&measure2, false); 
    lox0.rangingTest(&measure0, false); 
    lox1.rangingTest(&measure1, false); 

    String packet = String((hoek*0.55) - 180 + 45) + ";" + String(measure0.RangeMilliMeter / 10) + ";" +
                    String((hoek*0.55)- 90 + 45) + ";" + String(measure1.RangeMilliMeter / 10) + ";" +
                    String((hoek*0.55) + 45) + ";" + String(measure2.RangeMilliMeter / 10);

    sendBLEData(packet);
    BLE.poll(); // Houd de verbinding levend tijdens de loop
  }

  myservo.write(180 - RUST_STAND);
}

void beweegServo(int stap) {
  huidigePositie = constrain(huidigePositie + stap, 0, 180);
  myservo.write(huidigePositie);
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    while (central.connected()) {
      // Lezen van data
      if (rxCharacteristic.written()) {
        String value = rxCharacteristic.value();
        verwerkBericht(value);
      }

      // Start scan als vlag is gezet
      if (startScanVlag) {
        voerScanUit();
        startScanVlag = false; 
      }
    }
  }
}