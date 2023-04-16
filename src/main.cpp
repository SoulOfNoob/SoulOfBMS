#include <Arduino.h>
#include <jbdbms.h>
#include <HardwareSerial.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
////#include <SoftwareSerial.h>

#define I2C_SDA 32
#define I2C_SCL 33
#define BMS_RX_PIN 15
#define BMS_TX_PIN 14


//// HardwareSerial &BMSSerial = Serial1;
//// HardwareSerial &BMSSerial(1);
JbdBms jbdbms(Serial1);
Adafruit_SSD1306 display(128, 64, &Wire, -1);

void printBMSStatus(JbdBms::Status_t status);
void printCellsStatus(JbdBms::Cells_t cells);
void testOLED(int counter);

int counter = 0;

void setup() {
    Serial.begin(115200);
    Serial1.begin(9600, SERIAL_8N1, BMS_RX_PIN, BMS_TX_PIN);
    jbdbms.begin();
    Wire.begin(I2C_SDA, I2C_SCL);

    if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println(F("SSD1306 allocation failed"));
        for(;;);
    }
    delay(2000);
    display.clearDisplay();
  
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);
    
    display.println("Finished setup");
    display.display(); 

    Serial.println("Finished setup");
    Serial.println("-----------------------------");
}

void loop() {
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

    Serial.printf("Voltage V: %.2f\n", V);
    Serial.printf("Current A: %.2f\n", A);
    Serial.printf("Power W: %.2f\n", W);
    Serial.printf("Fault: %u\n", status.fault);
    Serial.printf("MosfetStatus: %u\n", status.mosfetStatus);
    Serial.println("------------------");

    display.clearDisplay();
  
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(0, 10);

    display.printf("Voltage V: %.2f\n", V);
    display.printf("Current A: %.2f\n", A);
    display.printf("Power W: %.2f\n", W);

    display.display(); 
}