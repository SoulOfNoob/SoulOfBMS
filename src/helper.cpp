#include "helper.h"

bool initSERIAL() {
    Serial.begin(115200);
    return true;
}
bool initI2C() {
    Wire.begin(I2C_SDA, I2C_SCL);
    return true;
}