#include <Wire.h> 
#include "Adafruit_VL53L0X.h"
#include <ESP32Servo.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h> 

// --- BLE DEFINITIES ---
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" 

// Voorwaartse declaratie van de scan functie zodat de callback deze kent
void voerScanUit();

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    deviceConnected = true;
    Serial.println("Device connected via BLE");
  };

  void onDisconnect(BLEServer *pServer) {
    deviceConnected = false;
    Serial.println("Device disconnected from BLE");
    pServer->startAdvertising(); 
  }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    // 1. Haal de ontvangen waarde op
    String rxValue = pCharacteristic->getValue();

    // 2. Controleer of er daadwerkelijk data is ontvangen
    if (rxValue.length() > 0) {
      Serial.print("Ontvangen (String): ");
      Serial.println(rxValue.c_str()); // Toon de tekst in de console

      // 3. De actie-logica (bijv. de scan starten)
      if (rxValue == "SCAN") {
        Serial.println("Actie: Scan functie wordt uitgevoerd...");
        voerScanUit();
      }
    }
  }
};

// --- LOX SENSOR DEFINITIES ---
#define LOX0_ADDRESS 0x30
#define LOX1_ADDRESS 0x31
#define LOX2_ADDRESS 0x32

#define SHT_LOX0 1 
#define SHT_LOX1 3 
#define SHT_LOX2 2 

Adafruit_VL53L0X lox0 = Adafruit_VL53L0X(); 
Adafruit_VL53L0X lox1 = Adafruit_VL53L0X(); 
Adafruit_VL53L0X lox2 = Adafruit_VL53L0X(); 

VL53L0X_RangingMeasurementData_t measure0; 
VL53L0X_RangingMeasurementData_t measure1; 
VL53L0X_RangingMeasurementData_t measure2; 

// --- SERVO DEFINITIES ---
Servo myservo;  
int servoPin = 8; 
const int MIN_HOEK = 45; 
const int MAX_HOEK = 135; 
int positie = MIN_HOEK; 
const int STAPGROOTTE = 2; 
bool gaatVooruit = true; 

void sendBLEData(String data) {
  if (deviceConnected) {
    pTxCharacteristic->setValue(data.c_str()); 
    pTxCharacteristic->notify(); 
  }
}

// Functie voor sensor adressen toewijzen
void setID() {
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
  Serial.println(F("Sensoren online."));
}

void setup() {
  Serial.begin(115200);

  // BLE Setup
  BLEDevice::init("XIAO_LIDAR_S3_BLE"); 
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  pTxCharacteristic->addDescriptor(new BLE2902()); 
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE);
  pRxCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  pServer->getAdvertising()->start();

  // Hardware Setup
  pinMode(SHT_LOX0, OUTPUT);
  pinMode(SHT_LOX1, OUTPUT);
  pinMode(SHT_LOX2, OUTPUT);
  setID(); 

  ESP32PWM::allocateTimer(0);
  myservo.setPeriodHertz(50);
  myservo.attach(servoPin, 1000, 2000); 
  myservo.write(180 - MIN_HOEK); // Startpositie
}

// --- DE NIEUWE SCAN FUNCTIE ---
void voerScanUit() {
  Serial.println("Scan gestart...");
  bool scanBezig = true;
  positie = MIN_HOEK;
  gaatVooruit = true;

  while (scanBezig) {
    // 1. Servo beweging
    if (gaatVooruit) {
      positie += STAPGROOTTE;
      if (positie >= MAX_HOEK) {
        positie = MAX_HOEK;
        gaatVooruit = false; 
      }
    } else {
      positie -= STAPGROOTTE;
      if (positie <= MIN_HOEK) {
        positie = MIN_HOEK;
        scanBezig = false; // Stop de loop als we weer bij het begin zijn
      }
    }
    
    myservo.write(180 - positie);
    delay(15); 

    // 2. Metingen
    lox2.rangingTest(&measure2, false); 
    lox0.rangingTest(&measure0, false); 
    lox1.rangingTest(&measure1, false); 

    // 3. Data versturen
    String print1 = String(positie - 180) + ";" + String(measure2.RangeMilliMeter / 10);
    String print2 = String(positie - 90) + ";" + String(measure0.RangeMilliMeter / 10);
    String print3 = String(positie) + ";" + String(measure1.RangeMilliMeter / 10);

    sendBLEData(print1 + ";" + print2 + ";" + print3);
    delay(2);
  }
  Serial.println("Scan voltooid.");
}

void loop() {
  // BLE Verbinding onderhouden
  if (!deviceConnected && oldDeviceConnected) {
    delay(500);                   
    pServer->startAdvertising();
    oldDeviceConnected = false;
  }
  if (deviceConnected && !oldDeviceConnected) {
    oldDeviceConnected = true;
  }
  
  // Geen actieve scan code hier, dat gaat nu via de callback!
}