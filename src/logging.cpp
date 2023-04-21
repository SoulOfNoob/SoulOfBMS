#include "logging.h"

SPIClass SDSPI(VSPI);
DS3231 myRTC;

bool century = false;
bool h12Flag;
bool pmFlag;

bool initSD() {
    Serial.println("INIT SD - Start");
    //SPI.begin(SDCARD_SCLK, SDCARD_MISO, SDCARD_MOSI);
    SDSPI.begin(SDCARD_SCLK, SDCARD_MISO, SDCARD_MOSI);
    bool rlst = SD.begin(SDCARD_CS, SDSPI);

    String sizeString = "SD Size:" + String(SD.cardSize() / 1024.0 / 1024.0 / 1024.0) + "G";
    Serial.println(rlst ? sizeString : "SD:N/A");
    Serial.println("INIT SD - Done");
    return true;
}
bool initRTC() {
    Serial.println("INIT RTC - Start");
    Serial.println("INIT RTC - Done");
    return true;
}

void printRTC() {
    Serial.printf(
        "%u.%u.%u %u:%u:%u\n", 
        myRTC.getDate(), 
        myRTC.getMonth(century), 
        myRTC.getYear(), 
        myRTC.getHour(h12Flag, pmFlag), 
        myRTC.getMinute(), 
        myRTC.getSecond()
    );
    Serial.println("------------------");
}