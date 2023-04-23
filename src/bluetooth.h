#ifndef BLUETOOTH_H
#define BLUETOOTH_H
    #include <Arduino.h>

    #include <BLEServer.h>
    #include <BLEDevice.h>
    #include <BLEUtils.h>

    bool initBT();
    void loopBT();
    bool getDeviceConnected();
    
#endif