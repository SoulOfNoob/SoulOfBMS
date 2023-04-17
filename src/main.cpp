#include <Arduino.h>
#include <jbdbms.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "BluetoothSerial.h"

#define I2C_SDA 32
#define I2C_SCL 33
#define BMS_RX_PIN 15
#define BMS_TX_PIN 14

JbdBms jbdbms(Serial1);
Adafruit_SSD1306 display(128, 64, &Wire, -1);
BluetoothSerial SerialBT;

String device_name = "ESP32-BT-Slave";

bool activeConnection = false;

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param);
void printBMSStatus(JbdBms::Status_t status);
void printCellsStatus(JbdBms::Cells_t cells);
void testOLED(int counter);

int counter = 0;

void setup() {
    Serial.begin(115200);
    Serial1.begin(9600, SERIAL_8N1, BMS_RX_PIN, BMS_TX_PIN);
    SerialBT.register_callback(callback);
    SerialBT.begin(device_name); //Bluetooth device name
    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.printf("The device with name \"%s\" is started.\nNow you can pair it with Bluetooth!\n", device_name.c_str());

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    delay(2000);

    jbdbms.begin();

    Serial.println("Finished setup");
    Serial.println("-----------------------------");
}

void loop() {
    if(activeConnection) {
        if (SerialBT.available()) {      // If anything comes in Serial (USB),
            Serial1.write(SerialBT.read());   // read it and send it out Serial1 (pins 0 & 1)
        }
        if (Serial1.available()) {     // If anything comes in Serial1 (pins 0 & 1)
            SerialBT.write(Serial1.read());   // read it and send it out Serial (USB)
        }
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

    display.clearDisplay();
  
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);

    if(A < -0.02) {
        Serial.println("Charging");
        display.println("Charging");
    } else if (A > 0.02) {
        Serial.println("Discharging");
        display.println("Discharging");
    } else {
        Serial.println("Idle");
        display.println("Idle");
    }

    display.printf("Voltage V: %.2f\n", V);
    display.printf("Current A: %.2f\n", A);
    display.printf("Power W: %.2f\n", W);

    Serial.printf("Voltage V: %.2f\n", V);
    Serial.printf("Current A: %.2f\n", A);
    Serial.printf("Power W: %.2f\n", W);
    Serial.printf("Fault: %u\n", status.fault);
    Serial.printf("MosfetStatus: %u\n", status.mosfetStatus);
    Serial.println("------------------");

    display.display(); 
}

void callback(esp_spp_cb_event_t event, esp_spp_cb_param_t *param){
    if(event == ESP_SPP_SRV_OPEN_EVT){
        Serial.println("Client Connected");
        activeConnection = true;
    }else if(event == ESP_SPP_CLOSE_EVT){
        Serial.println("Client DisConnected");
        activeConnection = false;
    }else if(event == ESP_SPP_DATA_IND_EVT){
        Serial.println("Data Received");
        activeConnection = false;
    }else if(event == ESP_SPP_WRITE_EVT){
        Serial.println("Data Written");
        activeConnection = false;
    } else {
        Serial.print("BT Event: ");
        Serial.println(event);
    }
}