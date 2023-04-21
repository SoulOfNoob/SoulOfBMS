#ifndef BMS_H
#define BMS_H
    #include <Arduino.h>

    // BMS Lib
    #include <jbdbms.h>
    #define BMS_RX_PIN 25
    #define BMS_TX_PIN 26

    bool initBMS();
    void printBMSStatus();
#endif