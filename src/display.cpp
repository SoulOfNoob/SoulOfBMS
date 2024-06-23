#include "display.h"

TaskHandle_t TaskHandleOLED;
TaskHandle_t TaskHandleEPAPER;

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> epaper(GxEPD2_213_BN(SS, 17, 16, 4));

MyBMS::shared_bms_data_t *MyDisplays::_myBMSData;

RTC_DATA_ATTR bool epaper_initialized = false;

const char epaper_headline[] = "Defqon.1";
const char epaper_subline[] = "Camp PokeCenter";

int count = 0;

void MyDisplays::initDisplays(MyBMS::shared_bms_data_t *myBMSData) {
    _myBMSData = myBMSData;
    xTaskCreate( taskCallbackOLED, "TaskHandleOLED", 10000, NULL, 1, &TaskHandleOLED );
    xTaskCreate( taskCallbackEPAPER, "TaskHandleEPAPER", 10000, NULL, 1, &TaskHandleEPAPER );
}

void MyDisplays::initOLED() {
    Serial.println("INIT OLED - START");
    if(!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.display();
    Serial.println("INIT OLED - DONE");
}
void MyDisplays::initEPAPER() {
    if(!epaper_initialized) {
        Serial.println("INIT EPAPER - START");
        epaper.init(0, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse

        epaper.setRotation(1);
        epaper.setFont(&FreeMonoBold12pt7b);
        epaper.setTextColor(GxEPD_BLACK);
        int16_t tbx, tby; uint16_t tbw, tbh;
        epaper.getTextBounds(epaper_headline, 0, 0, &tbx, &tby, &tbw, &tbh);
        // center bounding box by transposition of origin:
        uint16_t x = ((epaper.width() - tbw) / 2) - tbx;
        uint16_t y = ((epaper.height() - tbh) / 2) - tby;
        Serial.printf("width: %u, height: %u", epaper.width(), epaper.height());
        // full window mode is the initial mode, set it anyway
        epaper.setFullWindow();
        epaper.fillScreen(GxEPD_WHITE);
        epaper.setCursor(x-70, y-50);
        epaper.print(epaper_headline);

        epaper.setFont(&FreeMonoBold9pt7b);
        epaper.setCursor(x-70, y-30);
        epaper.print(epaper_subline);

        //updateEPAPER(true);

        Serial.println("INIT EPAPER - pre display");
        epaper.display(false); // full update
        Serial.println("INIT EPAPER - headline done");
        epaper.hibernate();
        epaper_initialized = true;
        Serial.println("INIT EPAPER - DONE");
    }
}

void MyDisplays::updateOLED() {
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextWrap(false);
    oled.setTextColor(WHITE);
    oled.setCursor(0, 10);
    oled.printf("%u%% %s (%.2fV)\n", _myBMSData->status.currentCapacity, _myBMSData->charging_state.c_str(), _myBMSData->vBatFloat);
    oled.printf("Remaining: %.0fWh\n", _myBMSData->remaining_capacity_Wh);
    oled.printf("Time Left: %.2fh\n", _myBMSData->remaining_time_h_cur);
    oled.printf("A: %.2fA (%.2fW)\n", _myBMSData->A, _myBMSData->W);
    oled.printf("V: %.2fV (%.2fV)\n", _myBMSData->V, _myBMSData->avgCellVolt);
    oled.printf("Temp: %.1fC | %.1fC\n", _myBMSData->temp_01, _myBMSData->temp_02);
    oled.display(); 
}

void MyDisplays::updateEPAPER(bool initialUpdate = false) {
    epaper.fillRect(0, 40, epaper.width(), epaper.height()-40, GxEPD_WHITE);
    epaper.setFont(&FreeMonoBold9pt7b);
    epaper.setTextColor(GxEPD_BLACK);

    epaper.setCursor(0, 80);    
    epaper.print("Remaining test");

    epaper.setCursor(0, 100);    
    epaper.print(_myBMSData->status.currentCapacity);       
    epaper.print("% - ");
    epaper.print(_myBMSData->remaining_time_h_cur);
    epaper.print("h (");
    epaper.print(_myBMSData->W);
    epaper.print("W)");

    epaper.setCursor(0, 120);
    epaper.print("Temp: ");
    epaper.print(_myBMSData->temp_01);
    epaper.print("°C | ");
    epaper.print(_myBMSData->temp_02);
    epaper.print("°C");
    
    if (!initialUpdate)
    {
        Serial.println("LOOP EPAPER - pre display");
        epaper.display(true); // partial update
        Serial.println("LOOP EPAPER - partial done");
        epaper.hibernate();
        //epaper.powerOff();
        Serial.println("LOOP EPAPER - Hibernating");
    }
    
}

void MyDisplays::taskCallbackOLED( void * pvParameters ) {
    TickType_t xLastWakeTime = xTaskGetTickCount ();
    const TickType_t xFrequency = TASK_INTERVAL_OLED / portTICK_PERIOD_MS;

    initOLED();
    for( ;; )
    {
        if(_myBMSData->lid_open) {
            updateOLED();
        } else {
            oled.clearDisplay();
            oled.display();
        }
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

void MyDisplays::taskCallbackEPAPER( void * pvParameters ) {
    initEPAPER();
    epaper.powerOff(); //ToDo: remove when fixing partial update
    vTaskDelete(NULL);
    // TickType_t xLastWakeTime;
    // const TickType_t xFrequency = TASK_INTERVAL_EPAPER / portTICK_PERIOD_MS;
    // xLastWakeTime = xTaskGetTickCount ();
    // for( ;; )
    // {
    //     updateEPAPER();
    //     vTaskDelayUntil( &xLastWakeTime, xFrequency );
    // }
}