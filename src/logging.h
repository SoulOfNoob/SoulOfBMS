
#ifndef LOGGING_H
#define LOGGING_H
    #include <Arduino.h>
    #include <Wire.h>

    // SD Libs
    #include <SD.h>
    #include <FS.h>

    // RTC Libs
    #include <DS3231.h>

    #include "bms.h"

    // SD Pins
    #define SDCARD_SCLK 14
    #define SDCARD_MISO 2
    #define SDCARD_MOSI 15
    #define SDCARD_CS 13
    #define TASK_INTERVAL_LOGGER 1000

    #define SdFile File
    #define seekSet seek

    class MyLogger {
        public:
            static void initLogger(MyBMS::shared_bms_data_t *myBMSData);

            static void initSD();
            static void initRTC();

            static void readRTC();
            
            static void printRTCStatusToConsole();
            static void printBMSStatusToConsole();

        private:
            static MyBMS::shared_bms_data_t *_myBMSData;

            static void taskCallbackLogger( void * pvParameters );
    };
#endif