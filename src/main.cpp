#include <Arduino.h>
#include <HardwareSerial.h>
//#include "BluetoothSerial.h"

// OLED Libs
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// ePaper Libs
#include <GxEPD2_BW.h>
#include <Fonts/FreeMonoBold9pt7b.h>

// BMS Lib
#include <jbdbms.h>

#define I2C_SDA 32
#define I2C_SCL 33
#define BMS_RX_PIN 15
#define BMS_TX_PIN 14

JbdBms jbdbms = JbdBms();
// BluetoothSerial SerialBT;
Adafruit_SSD1306 oled(128, 64, &Wire, -1);
GxEPD2_BW<GxEPD2_213_BN, GxEPD2_213_BN::HEIGHT> epaper(GxEPD2_213_BN(/*CS=5*/ SS, /*DC=*/ 17, /*RST=*/ 16, /*BUSY=*/ 4)); // DEPG0213BN 128x250, SSD1680, TTGO T5 V2.4.1, V2.3.1 // Works

String device_name = "SoulOfPower-BT";

TaskHandle_t Task1;

bool activeConnection = false;

// void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
void printBMSStatus(JbdBms::Status_t status);
void printCellsStatus(JbdBms::Cells_t cells);
void epaperHelloWorld();
void Task1code( void * pvParameters );

int counter = 0;

void setup() {
    Serial.begin(115200);
    Serial1.begin(9600, SERIAL_8N1, BMS_RX_PIN, BMS_TX_PIN);
    epaper.init(115200); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
    //SerialBT.register_callback(callback);
    //SerialBT.begin(device_name);
    Wire.begin(I2C_SDA, I2C_SCL);

    if(!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }

    xTaskCreatePinnedToCore( Task1code, "Task1", 10000, NULL, 0, &Task1, 0);

    delay(2000);

    Serial.println("Finished setup");
    Serial.println("-----------------------------");
}

void loop() {
    if(activeConnection) {
        // if (SerialBT.available()) {      // If anything comes in Serial (USB),
        //     Serial1.write(SerialBT.read());   // read it and send it out Serial1 (pins 0 & 1)
        // }
        // if (Serial1.available()) {     // If anything comes in Serial1 (pins 0 & 1)
        //     SerialBT.write(Serial1.read());   // read it and send it out Serial (USB)
        // }
    } else {
        JbdBms::Status_t bmsStatus;
        JbdBms::Cells_t cellStatus;

        if (jbdbms.getStatus(bmsStatus)) {
            printBMSStatus(bmsStatus);
            if (jbdbms.getCells(cellStatus)) {
                printCellsStatus(cellStatus);
            } else {
                Serial.println("jbdbms.getCells() failed");
            }
            
        } else {
            Serial.println("jbdbms.getStatus() failed");
        }
        delay(1000);
    }
}

void printCellsStatus(JbdBms::Cells_t cells) {
    for (size_t i = 0; i < 6; i++)
    {
        Serial.printf("Cell %u: %.2f\n", i+1, float(cells.voltages[i]) / 1000);
    }
    
    Serial.println("------------------");
}

void printBMSStatus(JbdBms::Status_t status) {
    float V = float(status.voltage) / 100; // convert mV to V
    float A = float(status.current) / 100 * -1; // convert mA to A
    float W = V * (A);

    oled.clearDisplay();
  
    oled.setTextSize(1);
    oled.setTextColor(WHITE);
    oled.setCursor(0, 10);

    if(A < -0.02) {
        Serial.println("Charging");
        oled.println("Charging");
    } else if (A > 0.02) {
        Serial.println("Discharging");
        oled.println("Discharging");
    } else {
        Serial.println("Idle");
        oled.println("Idle");
    }

    oled.printf("V: %.2fV\n", V);
    oled.printf("A: %.2fA\n", A);
    oled.printf("W: %.2fW\n", W);

    Serial.printf("V: %.2fV\n", V);
    Serial.printf("A: %.2fA\n", A);
    Serial.printf("W: %.2fW\n", W);
    Serial.printf("Fault: %u\n", status.fault);
    Serial.printf("Mosfet: %u\n", status.mosfetStatus);
    Serial.println("------------------");
    for (size_t i = 0; i < 2; i++)
    {
        Serial.printf("Temp %u: %.1fC\n", i+1, float(status.temperatures[i].lo)/10);
        oled.printf("Temp %u: %.1fC\n", i+1, float(status.temperatures[i].lo)/10);
    }
    
    Serial.println("------------------");

    oled.display(); 
}

void epaperHelloWorld() {
    const char HelloWorld[] = "Hello World!";

    Serial.println("helloWorld");
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

void Task1code( void * pvParameters ) {
    Serial.print("Task1 running on core ");
    Serial.println(xPortGetCoreID());
    //epaper.init(115200, true, 2, false); // USE THIS for Waveshare boards with "clever" reset circuit, 2ms reset pulse
    epaperHelloWorld();
    epaper.hibernate();
    Serial.println("epaper hibernating");
    delay(20000);
    vTaskDelete(Task1);
}

// void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
//     if(event == ESP_SPP_SRV_OPEN_EVT){
//         Serial.println("Client Connected");
//         activeConnection = true;
//     }else if(event == ESP_SPP_CLOSE_EVT){
//         Serial.println("Client DisConnected");
//         activeConnection = false;
//     }else if(event == ESP_SPP_DATA_IND_EVT){
//         Serial.println("Data Received");
//         activeConnection = false;
//     }else if(event == ESP_SPP_WRITE_EVT){
//         Serial.println("Data Written");
//         activeConnection = false;
//     } else {
//         Serial.print("BT Event: ");
//         Serial.println(event);
//     }
// }
