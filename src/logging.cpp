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
    Serial.println("------- RTC --------");
    Serial.print("LOOP RTC - Time: ");
    Serial.println(rtc_date);
}

void MyLogger::printBMSStatusToConsole() {
    Serial.println("------- OLED -------");
    Serial.printf("%u%% %s\n", _myBMSData->status.currentCapacity, _myBMSData->charging_state.c_str());
    Serial.printf("Remaining: %.0fWh\n", _myBMSData->remaining_capacity_Wh);
    Serial.printf("Time Left: %.2fh\n", _myBMSData->remaining_time_h_cur);
    Serial.printf("A: %.2fA (%.2fW)\n", _myBMSData->A, _myBMSData->W);
    Serial.printf("V: %.2fV (%.2fV)\n", _myBMSData->V, _myBMSData->avgCellVolt);
    Serial.printf("Temp: %.1fC | %.1fC\n", _myBMSData->temp_01, _myBMSData->temp_02);
    Serial.println("------- DEBUG ------");
    Serial.printf("Fault: %u\n", _myBMSData->status.fault);
    Serial.printf("Mosfet: %u\n", _myBMSData->status.mosfetStatus);
    Serial.printf("Lid Open: %u\n", _myBMSData->lid_open);
    Serial.printf("BT Enabled: %u\n", _myBMSData->bt_enabled);
}

void MyLogger::taskCallbackLogger( void * pvParameters ) {
    TickType_t xLastWakeTime = xTaskGetTickCount ();
    const TickType_t xFrequency = TASK_INTERVAL_LOGGER / portTICK_PERIOD_MS;

    Serial.println("INIT Logger - Start");
    // Serial.begin(115200); // ToDo move to here??
    // initRTC();
    initSD();
    Serial.println("INIT Logger - Done");
    
    
    for( ;; )
    {
        // readRTC();
        // printRTC();
        if(_myBMSData->lid_open) {
            printBMSStatusToConsole();
        }

        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}