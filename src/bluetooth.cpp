#include "bluetooth.h"

// identifiers from xiaoxiang bms bluetooth module
#define SERVICE_UART_UUID "0000ff00-0000-1000-8000-00805f9b34fb"
#define UART_RX_CHARACTERISTIC_UUID "0000ff01-0000-1000-8000-00805f9b34fb"
#define UART_TX_CHARACTERISTIC_UUID "0000ff02-0000-1000-8000-00805f9b34fb"
 
BLEServer *pServer;
BLEService *pService;
BLECharacteristic *pRxCharacteristic;
BLECharacteristic *pTxCharacteristic;

// The limit is taken from BLECharacteristic.cpp/indicate/notify doc, it says:
// An indication is a transmission of up to the first 20 bytes of the characteristic value.  An indication
// will block waiting a positive confirmation from the client.
#define Serial1_BUF_SIZE_MAX 20
uint8_t *Serial1ReadBuffer = new uint8_t[Serial1_BUF_SIZE_MAX];

bool deviceConnected = false;

bool getDeviceConnected() {
    return deviceConnected;
}

//Setup callbacks onConnect and onDisconnect
class ServerConnectionCallback: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    Serial.println("Device Connected");
    deviceConnected = true;
  };
  void onDisconnect(BLEServer* pServer) {
    Serial.println("Device Disonnected");
    deviceConnected = false;
    pServer->getAdvertising()->start();
  }
};

class UartTxBMSCallback : public BLECharacteristicCallbacks
{
    void onWrite(BLECharacteristic *pCharacteristic) {
        const std::string &value = pCharacteristic->getValue();
        const size_t length = value.length();
        const char *data = value.data();
        if (length > 0)
        {
            for (size_t i = 0; i < length; ++i)
            {
                Serial1.print(data[i]);
            }
        }
    }
};

void loopBT(){
    if(deviceConnected) {
        int uartBytesAvailable = Serial1.available();
        if (uartBytesAvailable)
        {
            if (pServer->getConnectedCount() > 0) {
                // allow uart buffer to accumulate (don't send byte by byte)
                if (uartBytesAvailable == 1) {
                    delay(50);
                    uartBytesAvailable = Serial1.available();
                }

                const size_t sizeSerial1Read = Serial1.readBytes(Serial1ReadBuffer, min(Serial1_BUF_SIZE_MAX, uartBytesAvailable));
                pRxCharacteristic->setValue(Serial1ReadBuffer, sizeSerial1Read);
                pRxCharacteristic->notify(); // may use indicate instead but that will require client to send ack
            } else {
                // Discard UART data if no connection
                while (Serial1.available())
                {
                    Serial1.read();
                }
            }
        }
    }
}

bool initBT() {
    Serial.println("INIT BT - START");
    
    BLEDevice::init("SoulOFBMS-BLE");
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new ServerConnectionCallback());
    pService = pServer->createService(SERVICE_UART_UUID);
    pRxCharacteristic = pService->createCharacteristic(
        UART_RX_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY);
    pTxCharacteristic = pService->createCharacteristic(
        UART_TX_CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_WRITE);
    
    pTxCharacteristic->setCallbacks(new UartTxBMSCallback());
    
    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UART_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x06); // functions that help with iPhone connections issue
    pAdvertising->setMinPreferred(0x12);
    BLEDevice::startAdvertising();


    Serial.println("INIT BT - DONE");
    return true;
}