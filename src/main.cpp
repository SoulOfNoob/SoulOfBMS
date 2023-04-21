
#include <Arduino.h>

#include "helper.h"
#include "logging.h"
#include "display.h"
#include "bms.h"

void setup() {
    initSERIAL();
    initI2C();
    initRTC();
    initSD();
    initBMS();
    initOLED();
    initEPAPER();

    delay(2000);
    Serial.println("Finished setup");
    Serial.println("-----------------------------");
}

void loop() {
    printRTC();
    printBMSStatus();
    delay(1000);
}