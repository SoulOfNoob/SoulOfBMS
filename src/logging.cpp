#include "logging.h"

SPIClass SDSPI(VSPI);
DS3231 myRTC;

bool century = false;
bool h12Flag;
bool pmFlag;

TaskHandle_t TaskHandleLogger;
String rtc_date;

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

    String sizeString = "INIT SD - SD Size:" + String(SD.cardSize() / 1024.0 / 1024.0 / 1024.0) + "G";
    Serial.println(rlst ? sizeString : "INIT SD - SD:N/A");
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
    rtc_date = buffer;
}

void MyLogger::printRTCStatusToConsole() {
    Serial.print("LOOP RTC - Time: ");
    Serial.println(rtc_date);
    Serial.println("--------------------");
}

void MyLogger::printBMSStatusToConsole() {
    Serial.printf("State: %s\n", _myBMSData->charging_state);
    Serial.printf("Time left: %.2fh\n", _myBMSData->remaining_time_h_cur);
    Serial.printf("Capacity Left: %.2fAh\n", _myBMSData->remaining_capacity_Ah);
    Serial.printf("Capacity Left: %.2fWh\n", _myBMSData->remaining_capacity_Wh);
    Serial.printf("V: %.2fV (%.2fV)\n", _myBMSData->V, _myBMSData->avgCellVolt);
    Serial.printf("A: %.2fA\n", _myBMSData->A);
    Serial.printf("W: %.2fW\n", _myBMSData->W);
    Serial.printf("Fault: %u\n", _myBMSData->status.fault);
    Serial.printf("Mosfet: %u\n", _myBMSData->status.mosfetStatus);
    Serial.printf("Reed: %u\n", digitalRead(13));
    Serial.println("--------------------");
    Serial.printf("Temp: %.1fC | %.1fC\n", _myBMSData->temp_01, _myBMSData->temp_02);
    Serial.println("--------------------");
}

void MyLogger::taskCallbackLogger( void * pvParameters ) {
    Serial.println("INIT Logger - Start");
    // Serial.begin(115200); // ToDo move to here??
    // initRTC();
    initSD();
    Serial.println("INIT Logger - Done");

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = TASK_INTERVAL_LOGGER / portTICK_PERIOD_MS;
    xLastWakeTime = xTaskGetTickCount ();
    for( ;; )
    {
        // readRTC();
        // printRTC();
        // printBMSStatusToConsole();

        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}