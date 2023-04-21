#include "display.h"

TaskHandle_t Task1;

Adafruit_SSD1306 oled(128, 64, &Wire, -1);
GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> epaper(GxEPD2_213_BN(SS, 17, 16, 4));

bool initOLED() {
    Serial.println("INIT OLED - Start");
    if(!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    oled.clearDisplay();
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(0, 10);
    oled.println("Hello BMS");
    oled.display();
    Serial.println("INIT OLED - DONE");
    return true;
}
bool initEPAPER() {
    xTaskCreatePinnedToCore( initEPAPERTask, "Task1", 10000, NULL, 0, &Task1, 0);
    return true;
}

void epaperHelloWorld() {
    const char HelloWorld[] = "Hello BMS!";

    epaper.setRotation(1);
    epaper.setFont(&FreeMonoBold9pt7b);
    epaper.setTextColor(GxEPD_BLACK);
    int16_t tbx, tby; uint16_t tbw, tbh;
    epaper.getTextBounds(HelloWorld, 0, 0, &tbx, &tby, &tbw, &tbh);
    // center bounding box by transposition of origin:
    uint16_t x = ((epaper.width() - tbw) / 2) - tbx;
    uint16_t y = ((epaper.height() - tbh) / 2) - tby;
    Serial.printf("width: %u, height: %u", epaper.width(), epaper.height());
    // full window mode is the initial mode, set it anyway
    epaper.setFullWindow();
    epaper.fillScreen(GxEPD_WHITE);
    epaper.drawRect(0, 0, epaper.width(), epaper.height(), GxEPD_BLACK);
    epaper.drawLine(0, 0, epaper.width(), epaper.height(), GxEPD_BLACK);
    epaper.drawLine(0, epaper.height(), epaper.width(), 0, GxEPD_BLACK);
    int margin = 50;
    epaper.fillRect(margin, margin, epaper.width()-margin*2, epaper.height()-margin*2, GxEPD_WHITE);
    epaper.drawRect(margin, margin, epaper.width()-margin*2, epaper.height()-margin*2, GxEPD_BLACK);
    epaper.setCursor(x, y);
    epaper.print(HelloWorld);
    Serial.println("pre display");
    epaper.display(false); // full update
    Serial.println("helloWorld done");
}

void initEPAPERTask( void * pvParameters ) {
    Serial.println("INIT EPAPER - Start");
    epaper.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
    epaperHelloWorld();
    epaper.hibernate();
    Serial.println("epaper hibernating");
    delay(2000);
    Serial.println("INIT EPAPER - DONE");
    vTaskDelete(Task1);
}