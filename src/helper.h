#ifndef HELPER_H
#define HELPER_H
    #include <Arduino.h>
    #include <Wire.h>

    #define I2C_SDA 32
    #define I2C_SCL 33

    bool initSERIAL();
    bool initI2C();
#endif