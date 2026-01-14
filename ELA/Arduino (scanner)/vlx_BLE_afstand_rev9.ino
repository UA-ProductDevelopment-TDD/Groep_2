#include <Wire.h> 
#include "Adafruit_VL53L0X.h"
#include <ESP32Servo.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h> 

// --- CONTROLE VARIABELEN ---
String inputBuffer = ""; 
unsigned long laatsteOntvangstTijd = 0;
bool startScanVlag = false; 

// --- BLE DEFINITIES ---
BLEServer *pServer = NULL;
BLECharacteristic *pTxCharacteristic;
bool deviceConnected = false;
bool oldDeviceConnected = false;

#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E" 
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400003-B5A3-F393-E0A9-E50E24DCCA9E" 

// --- HARDWARE OBJECTEN ---
Adafruit_VL53L0X lox0, lox1, lox2;
VL53L0X_RangingMeasurementData_t measure0, measure1, measure2; 
Servo myservo;  
int servoPin = 8; 

// --- AANGEPASTE SCAN PARAMETERS (45 tot 135) ---
const int MIN_HOEK = 45;     // Start van de scan
const int MAX_HOEK = 135;    // Eind van de scan
const int RUST_STAND = 90;   // Middenpositie
const int STAPGROOTTE = 2;
int huidigePositie = 90; // We beginnen in het midden

void sendBLEData(String data) {
  if (deviceConnected) {
    pTxCharacteristic->setValue(data.c_str()); 
    pTxCharacteristic->notify(); 
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
    delay(15);
    huidigePositie = RUST_STAND;
  }

  if (bericht.startsWith("J2")){
    Serial.println("Bericht start met J2");
    // 1. Zoek de posities van alle puntkomma's
    int eerstePuntkomma = bericht.indexOf(';');
    int tweedePuntkomma = bericht.indexOf(';', eerstePuntkomma + 1);
    int derdePuntkomma  = bericht.indexOf(';', tweedePuntkomma + 1);
    int vierdePuntkomma  = bericht.indexOf(';', derdePuntkomma + 1);

    // 2. Snijd de stukjes tekst uit en zet ze om naar integers (getallen)
    // We slaan "J2" (index 0 tot eerste puntkomma) over.
    
    int J2_X     = bericht.substring(eerstePuntkomma + 1, tweedePuntkomma).toInt();
    int J2_Y     = bericht.substring(tweedePuntkomma + 1, derdePuntkomma).toInt();
    int J2_speed = bericht.substring(derdePuntkomma + 1, vierdePuntkomma).toInt();
    int J2_angle = bericht.substring(vierdePuntkomma + 1).toInt(); // Tot het einde van de string

    // Controle in de Seriële Monitor
    Serial.print("X: "); Serial.println(J2_X);
    Serial.print("Y: "); Serial.println(J2_Y);
    Serial.print("Speed: "); Serial.println(J2_speed);
    Serial.print("Angle: "); Serial.println(J2_angle);
    if (J2_angle >= 170 && J2_angle <= 190){
      Serial.println("Beweeg naar links");
      beweegServo(J2_speed/20);
    }
    if (J2_angle > 350 || J2_angle < 10){
      Serial.println("Beweeg naar rechts");
      beweegServo(-J2_speed/20);
    }
  }else{
    Serial.print("Bericht start niet met J2");
  }
}

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) { deviceConnected = true; Serial.println("BLE Verbonden"); }
  void onDisconnect(BLEServer *pServer) { deviceConnected = false; pServer->startAdvertising(); Serial.println("BLE Verbroken - Advertising..."); }
};

class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String rxValue = pCharacteristic->getValue();
    if (rxValue.length() > 0) {
      inputBuffer += rxValue; 
      laatsteOntvangstTijd = millis(); 
    }
  }
};

void setID() {
  pinMode(1, OUTPUT); pinMode(3, OUTPUT); pinMode(2, OUTPUT);
  digitalWrite(1, LOW); digitalWrite(3, LOW); digitalWrite(2, LOW);
  delay(10);
  digitalWrite(1, HIGH); lox0.begin(0x30); delay(10);
  digitalWrite(3, HIGH); lox1.begin(0x31); delay(10);
  digitalWrite(2, HIGH); lox2.begin(0x32);
  Serial.println("Sensoren: OK");
}

void setup() {
  Serial.begin(115200);
  BLEDevice::init("XIAO_LIDAR_S3_BLE");
  BLEDevice::setMTU(517);
  
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pTxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_TX, BLECharacteristic::PROPERTY_NOTIFY);
  pTxCharacteristic->addDescriptor(new BLE2902());
  BLECharacteristic *pRxCharacteristic = pService->createCharacteristic(CHARACTERISTIC_UUID_RX, BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR);
  pRxCharacteristic->setCallbacks(new MyCallbacks());
  pService->start();
  pServer->getAdvertising()->start();

  setID();
  ESP32PWM::allocateTimer(0);
  myservo.setPeriodHertz(50);
  myservo.attach(servoPin, 1000, 2000);
  
  // Start in ruststand
  myservo.write(180 - RUST_STAND); 
}

void voerScanUit() {
  Serial.println("--- SCAN START (45-135) ---");
  
  // 1. Spring naar de startpositie (45 graden)
  myservo.write(180 - MIN_HOEK); 
  delay(400); // Wacht tot de servo er is

  // 2. Scan van 45 naar 135 graden
  for (int hoek = MIN_HOEK; hoek <= MAX_HOEK; hoek += STAPGROOTTE) {
    myservo.write(180 - hoek);
    delay(20); 

    lox2.rangingTest(&measure2, false); 
    lox0.rangingTest(&measure0, false); 
    lox1.rangingTest(&measure1, false); 

    // Datapakket bouwen: Hoek t.o.v. sensoren
    String packet = String(hoek - 180) + ";" + String(measure2.RangeMilliMeter / 10) + ";" +
                    String(hoek - 90) + ";" + String(measure0.RangeMilliMeter / 10) + ";" +
                    String(hoek) + ";" + String(measure1.RangeMilliMeter / 10);

    sendBLEData(packet);
  }

  // 3. Terug naar ruststand (90 graden)
  myservo.write(180 - RUST_STAND);
  Serial.println("--- SCAN KLAAR: Terug naar 90 graden ---");
}

void beweegServo(int stap) {
  // Bereken de nieuwe positie
  int nieuwePositie = huidigePositie + stap;

  // Beveiliging: zorg dat de servo tussen 0 en 180 graden blijft
  if (nieuwePositie > 180) {
    nieuwePositie = 180;
  }
  if (nieuwePositie < 0) {
    nieuwePositie = 0;
  }

  // Voer de beweging uit
  huidigePositie = nieuwePositie;
  myservo.write(huidigePositie);
}

void loop() {
  // Verwerk buffer na 50ms stilte
  if (inputBuffer.length() > 0 && (millis() - laatsteOntvangstTijd > 50)) {
    verwerkBericht(inputBuffer);
    inputBuffer = ""; 
  }

  // Start scan als vlag is gezet
  if (startScanVlag) {
    voerScanUit();
    startScanVlag = false; 
  }

  // BLE status management
  if (!deviceConnected && oldDeviceConnected) { delay(500); oldDeviceConnected = false; }
  if (deviceConnected && !oldDeviceConnected) { oldDeviceConnected = true; }
}