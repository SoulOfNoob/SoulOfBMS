#ifndef BLUETOOTH_H
#define BLUETOOTH_H
    #include <Arduino.h>

    #include <BLEServer.h>
    #include <BLEDevice.h>
    #include <BLEUtils.h>

    #include "bms.h"

    #define TASK_INTERVAL_BT

    class MyBluetooth {
        public:
            static void initBT();
            static void setupBT();
            static void loopBT();
        private:
            static void taskCallbackBT( void * pvParameters );
    };
    
#endif