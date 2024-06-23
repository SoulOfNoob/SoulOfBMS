#define FEATURE_BT

#include <Arduino.h>
#include <Wire.h>

#include "logging.h"
#include "display.h"
#include "bluetooth.h"
#include "bms.h"

#define I2C_SDA 32
#define I2C_SCL 33

#define REED_PIN 12
#define BT_SWITCH_PIN 27

MyBMS::shared_bms_data_t myBMSData;

void setup() {
    pinMode(REED_PIN, INPUT_PULLUP);
    pinMode(BT_SWITCH_PIN, INPUT_PULLUP);

    myBMSData.bt_enabled = !digitalRead(BT_SWITCH_PIN);
    myBMSData.lid_open = digitalRead(REED_PIN);

    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.begin(115200);
    delay(2000); // wait for serial
    
    MyBMS::initBMS(&myBMSData);
    MyLogger::initLogger(&myBMSData);
    MyDisplays::initDisplays(&myBMSData);

    #ifdef FEATURE_BT
        if(myBMSData.bt_enabled) {
            MyBluetooth::initBT();
        }
    #endif

    Serial.println("Finished setup");
    Serial.println("-------------------------------");
}

void loop() {
    
}