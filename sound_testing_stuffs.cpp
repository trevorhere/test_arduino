
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
int digitalValue = 0;
int pin = A0;
int motor = A1;
byte value = 0;

int lightState = 0;

// See the following for generating UUIDs:
// https://www.uuidgenerator.net/

#define SERVICE_UUID        "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"


class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value.length() > 0) {
        Serial.println("*********");
//        Serial.print("New value: ");
//        for (int i = 0; i < value.length(); i++)
//          Serial.print(value[i]);
          int strVal = value[0] - '0';

          if(strVal){
            Serial.println("on");
          }else {
            Serial.println("off");
          }

        Serial.println();
        Serial.println("*********");
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


  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());
 


  // Start the service
  pService->start();

  // Start advertising
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(false);
  pAdvertising->setMinPreferred(0x0);  // set value to 0x00 to not advertise this parameter
  BLEDevice::startAdvertising();

  
  Serial.println("Waiting a client connection to notify...");
  pinMode (pin, INPUT); // Set the signal pin as input 
  pinMode(LED_BUILTIN, OUTPUT);
  pinMode(motor, OUTPUT);

}

void loop() {

    // notify changed value
    if (deviceConnected) {

        Serial.println("connected");

 
        digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
        delay(250);                       // wait for a second
        digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
        delay(250);   
        
        digitalWrite(motor, HIGH);   // turn the LED on (HIGH is the voltage level)
        Serial.println("motor on");
        delay(3000);                       // wait for a second
//        digitalWrite(motor, LOW);    // turn the LED off by making the voltage LOW Serial.println("motor on");
//        Serial.println("motor off");
//        delay(500);   


        digitalValue = digitalRead(pin);

        if(digitalValue){
            Serial.println("quiet");
             pCharacteristic->setValue("Quiet");
             pCharacteristic->notify();  
          } else {
             Serial.println("LOUD!!!");
             pCharacteristic->setValue("Loud");
             pCharacteristic->notify();  
          }

        delay(200); // bluetooth stack will go into congestion, if too many packets are sent, in 6 hours test i was able to go as low as 3ms
    }
    // disconnecting
    if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
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