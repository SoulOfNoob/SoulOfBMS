#define FEATURE_BT

#include <Arduino.h>

#include "helper.h"
#include "logging.h"
#include "display.h"
#include "bluetooth.h"
#include "bms.h"

MyBMS::shared_bms_data_t myBMSData;

void setup() {
    MyHelper::initHelper(&myBMSData);
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