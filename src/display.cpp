#include "display.h"

TaskHandle_t TaskHandleOLED;
TaskHandle_t TaskHandleEPAPER;

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> epaper(GxEPD2_213_BN(SS, 17, 16, 4));

MyBMS::shared_bms_data_t *MyDisplays::_myBMSData;

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
    Serial.println("INIT EPAPER - START");
    epaper.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
    epaperHelloWorld();
    epaper.hibernate();
    Serial.println("epaper hibernating");
    Serial.println("INIT EPAPER - DONE");
}

void MyDisplays::epaperHelloWorld() {
    const char headline[] = "Defqon.1";
    const char subline[] = "Team PowerRadler";

    epaper.setRotation(1);
    epaper.setFont(&FreeMonoBold12pt7b);
    epaper.setTextColor(GxEPD_BLACK);
    int16_t tbx, tby; uint16_t tbw, tbh;
    epaper.getTextBounds(headline, 0, 0, &tbx, &tby, &tbw, &tbh);
    // center bounding box by transposition of origin:
    uint16_t x = ((epaper.width() - tbw) / 2) - tbx;
    uint16_t y = ((epaper.height() - tbh) / 2) - tby;
    Serial.printf("width: %u, height: %u", epaper.width(), epaper.height());
    // full window mode is the initial mode, set it anyway
    epaper.setFullWindow();
    epaper.fillScreen(GxEPD_WHITE);
    // epaper.drawRect(0, 0, epaper.width(), epaper.height(), GxEPD_BLACK);
    // epaper.drawLine(0, 0, epaper.width(), epaper.height(), GxEPD_BLACK);
    // epaper.drawLine(0, epaper.height(), epaper.width(), 0, GxEPD_BLACK);
    // int margin = 50;
    // epaper.fillRect(margin, margin, epaper.width()-margin*2, epaper.height()-margin*2, GxEPD_WHITE);
    // epaper.drawRect(margin, margin, epaper.width()-margin*2, epaper.height()-margin*2, GxEPD_BLACK);
    epaper.setCursor(x-70, y-50);
    epaper.print(headline);

    epaper.setFont(&FreeMonoBold9pt7b);
    epaper.setCursor(x-70, y-30);
    epaper.print(subline);

    Serial.println("pre display");
    epaper.display(false); // full update
    Serial.println("headline done");
}

void MyDisplays::updateOLED() {
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(0, 10);
    oled.println(_myBMSData->charging_state);
    oled.printf("Remaining: %u%%\n", _myBMSData->status.currentCapacity);
    oled.printf("Time Left: %.2fh\n", _myBMSData->remaining_time_h_cur);
    oled.printf("V: %.2fV (%.2fV)\n", _myBMSData->V, _myBMSData->avgCellVolt);
    oled.printf("A: %.2fA (%.2fW)\n", _myBMSData->A, _myBMSData->W);
    oled.printf("Temp: %.1fC | %.1fC\n", _myBMSData->temp_01, _myBMSData->temp_02);
    oled.display(); 
}

void printEpaperValue(const char *text, const float value, const char *unit, int16_t y)
{
    epaper.setCursor(0, y);
    epaper.print(text);
    epaper.print(value);
    epaper.print(unit);
}

void MyDisplays::updateEPAPER() {
    epaper.fillRect(0, 40, epaper.width(), epaper.height()-40, GxEPD_WHITE);
    epaper.setFont(&FreeMonoBold9pt7b);

    epaper.setCursor(0, 80);    
    epaper.print("Remaining");

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

    Serial.println("pre display");
    epaper.display(true); // partial update
    Serial.println("partial done");
}

void MyDisplays::taskCallbackOLED( void * pvParameters ) {
    initOLED();
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = TASK_INTERVAL_OLED / portTICK_PERIOD_MS;
    xLastWakeTime = xTaskGetTickCount ();
    for( ;; )
    {
        updateOLED();
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

void MyDisplays::taskCallbackEPAPER( void * pvParameters ) {
    initEPAPER();
    TickType_t xLastWakeTime;
    const TickType_t xFrequency = TASK_INTERVAL_EPAPER / portTICK_PERIOD_MS;
    xLastWakeTime = xTaskGetTickCount ();
    for( ;; )
    {
        updateEPAPER();
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}