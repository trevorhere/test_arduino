#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>



BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
BLECharacteristic* volumeCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;


int loudCounter = 0;
int digitalValue = 0;
int soundInput = A0;
int motorA = A1;
int motorB = A6;

byte value = 0;
int vibratingState = LOW;

// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define volumenCharacteristic_UUID "088a1e2a-1991-411e-8a73-f71bdcd7487c"

#define offState 1
#define onState 2
#define carState 3
#define sootheState 4
#define loudState 5

int currentState = 1;


class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *volumeCharacteristic) {
      std::string value = volumeCharacteristic->getValue();

      if (value.length() > 0) {
//        Serial.println("*********");
          int strVal = value[0] - '0';
          currentState = strVal;
          Serial.print("strVal: ");   
          Serial.println(strVal); 
//        Serial.println();
//        Serial.println("*********");
      }
    }
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};


void setup() {
  
  Serial.begin(115200);
  BLEDevice::init("ESP32");

  // Create the BLE Server
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  // Create the BLE Service
  BLEService *pService = pServer->createService(SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      CHARACTERISTIC_UUID,
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_INDICATE
                    );
                    
//  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());

  volumeCharacteristic = pService->createCharacteristic(
                    volumenCharacteristic_UUID,
                    BLECharacteristic::PROPERTY_READ   |
                    BLECharacteristic::PROPERTY_WRITE  |
                    BLECharacteristic::PROPERTY_NOTIFY |
                    BLECharacteristic::PROPERTY_INDICATE
                  );

  volumeCharacteristic->setCallbacks(new MyCallbacks());
  volumeCharacteristic->addDescriptor(new BLE2902());
 
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0); 
  BLEDevice::startAdvertising();

  
  Serial.println("Waiting a client connection to notify...");
 
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode (soundInput, INPUT);
  pinMode(motorA, OUTPUT);
  pinMode(motorB, OUTPUT);
}

void triggerOffState(){
  digitalWrite(motorA, LOW);   
  digitalWrite(motorB, LOW);
  loudCounter = -5;
}

void triggerOnState(){
  digitalWrite(motorA, HIGH);   
  digitalWrite(motorB, HIGH);
}
void triggerSootheState(){
  digitalWrite(motorA, LOW);  
  digitalWrite(motorB, HIGH);
  delay(500);      
  digitalWrite(motorA, HIGH); 
  digitalWrite(motorB, LOW); 
  delay(500);   
}
void triggerLoudState(){
   Serial.print("loudCounter: ");
   Serial.println(loudCounter);
             
  if(loudCounter > 0){
   digitalWrite(motorA, HIGH);  
   digitalWrite(motorB, HIGH);
  } else {
   digitalWrite(motorA, LOW);  
   digitalWrite(motorB, LOW);
  }

  delay(1000);      
}



void loop() {
  
    if (deviceConnected) {

      
      switch (currentState) {
        case 1:
         triggerOffState();
          break;
        case 2:
          triggerOnState();
          break;
        case 3:
          triggerOnState();
          break;
        case 4:
          triggerSootheState();
          break;
        case 5:
          triggerLoudState();
          break;
        default:
          triggerOffState();
          break;
      }

      
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(100);                       // wait for a second
        digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
        delay(100);   
   
//        digitalWrite(motorA, vibratingState);   // turn the LED on (HIGH is the voltage level)
//        digitalWrite(motorB, vibratingState);
       

        digitalValue = digitalRead(soundInput);

        if(digitalValue){
           
            
             if(loudCounter > -5){
                loudCounter -= 1;
             }

//             Serial.println("quiet");

             
             pCharacteristic->setValue("0");
             pCharacteristic->notify();  
          } else {
            
             loudCounter = 25;
//             Serial.println("LOUD!!!");
//             Serial.print("loudCounter: ");
//             Serial.println(loudCounter);
            
             pCharacteristic->setValue("1");
             pCharacteristic->notify();  
          }

//        delay(200); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        triggerOffState();
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
    }
    // connecting
    if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        Serial.println("connecting");
        oldDeviceConnected = deviceConnected;
    }
}