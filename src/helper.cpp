#include "helper.h"

MyBMS::shared_bms_data_t *MyHelper::_myBMSData;

void MyHelper::initHelper(MyBMS::shared_bms_data_t *myBMSData) {
    initSERIAL();
    initI2C();
    pinMode(13, INPUT_PULLUP);
}

void MyHelper::initSERIAL() {
    Serial.begin(115200);
    Serial.println("INIT SERIAL - START");
    delay(3000);
    Serial.println("INIT SERIAL - DONE");
}

void MyHelper::initI2C() {
    Wire.begin(I2C_SDA, I2C_SCL);
}