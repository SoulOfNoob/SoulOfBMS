#define FEATURE_BT

#include <Arduino.h>
#include <Wire.h>

#include "logging.h"
#include "display.h"
#include "bluetooth.h"
#include "bms.h"

#define I2C_SDA 32
#define I2C_SCL 33

#define REED_PIN 13

MyBMS::shared_bms_data_t myBMSData;

void setup() {
    pinMode(REED_PIN, INPUT_PULLUP);

    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.begin(115200);
    delay(2000); // wait for serial
    
    MyLogger::initLogger(&myBMSData);
    MyDisplays::initDisplays(&myBMSData);
    MyBMS::initBMS(&myBMSData);

    #ifdef FEATURE_BT
        MyBluetooth::initBT();
    #endif

    Serial.println("Finished setup");
    Serial.println("-------------------------------");
}

void loop() {
    
}