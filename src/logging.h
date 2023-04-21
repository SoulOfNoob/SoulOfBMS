
#ifndef LOGGING_H
#define LOGGING_H
    #include <Arduino.h>
    #include <Wire.h>

    // SD Libs
    #include <SD.h>
    #include <FS.h>

    // RTC Libs
    #include <DS3231.h>

    // SD Pins
    #define SDCARD_SCLK 14
    #define SDCARD_MISO 2
    #define SDCARD_MOSI 15
    #define SDCARD_CS 13

    #define SdFile File
    #define seekSet seek

    bool initSD();
    bool initRTC();
    void printRTC();
#endif