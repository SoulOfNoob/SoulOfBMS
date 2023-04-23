#ifndef HELPER_H
#define HELPER_H
    #include <Arduino.h>
    #include <Wire.h>

    #include "bms.h"

    #define I2C_SDA 32
    #define I2C_SCL 33

    class MyHelper {
        public:
            static void initHelper(MyBMS::shared_bms_data_t *myBMSData);
            static void initSERIAL();
            static void initI2C();
        private:
            static MyBMS::shared_bms_data_t *_myBMSData;
    };
#endif