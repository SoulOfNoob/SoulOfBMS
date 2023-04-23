
#include <Arduino.h>

#include "helper.h"
#include "logging.h"
#include "display.h"
#include "bluetooth.h"
#include "bms.h"

void setup() {
    
    initSERIAL();
    initI2C();
    initBMS();
    initBT();
    initRTC();
    initSD();
    initOLED();
    initEPAPER();
    
    delay(1000);
    Serial.println("Finished setup");
    Serial.println("-------------------------------");
}

void loop() {
    bool test = getDeviceConnected();
    loopBT();
    if (!test) {
        printRTC();
        printBMSStatus();
        delay(1000);
    }
}