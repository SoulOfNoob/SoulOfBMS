#include "logging.h"

SPIClass SDSPI(VSPI);
DS3231 myRTC;

bool century = false;
bool h12Flag;
bool pmFlag;

TaskHandle_t TaskHandleLogger;

MyBMS::shared_bms_data_t *MyLogger::_myBMSData;

void MyLogger::initLogger(MyBMS::shared_bms_data_t *myBMSData) {
    _myBMSData = myBMSData;
    xTaskCreate( taskCallbackLogger, "TaskHandleLogger", 10000, NULL, 1, &TaskHandleLogger );
}

void MyLogger::initSD() {
    Serial.println("INIT SD - Start");
    //SPI.begin(SDCARD_SCLK, SDCARD_MISO, SDCARD_MOSI);
    SDSPI.begin(SDCARD_SCLK, SDCARD_MISO, SDCARD_MOSI);
    bool rlst = SD.begin(SDCARD_CS, SDSPI);

    String sizeString = "SD Size:" + String(SD.cardSize() / 1024.0 / 1024.0 / 1024.0) + "G";
    Serial.println(rlst ? sizeString : "SD:N/A");
    Serial.println("INIT SD - Done");
}
void MyLogger::initRTC() {
    Serial.println("INIT RTC - Start");
    Serial.println("INIT RTC - Done");
}

void MyLogger::readRTC() {
    char buffer[40];
    sprintf(
        buffer, 
        "%u.%u.%u %u:%u:%u", 
        myRTC.getDate(), 
        myRTC.getMonth(century), 
        myRTC.getYear(), 
        myRTC.getHour(h12Flag, pmFlag), 
        myRTC.getMinute(), 
        myRTC.getSecond()
    );
    _myBMSData->rtc_date = buffer;
}

void MyLogger::printRTC() {
    Serial.println(_myBMSData->rtc_date);
    Serial.println("--------------------");
}

void MyLogger::taskCallbackLogger( void * pvParameters ) {
    initRTC();
    initSD();

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = TASK_INTERVAL_LOGGER / portTICK_PERIOD_MS;
    xLastWakeTime = xTaskGetTickCount ();
    for( ;; )
    {
        readRTC();
        printRTC();

        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}