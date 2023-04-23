#include "bms.h"

JbdBms jbdbms = JbdBms();

bool initBMS() {
    Serial1.begin(9600, SERIAL_8N1, BMS_RX_PIN, BMS_TX_PIN);
    return true;
}

void printBMSStatus() {
    JbdBms::Hardware_t hardwareStatus;
    JbdBms::Cells_t cellStatus;
    JbdBms::Status_t bmsStatus;
    
    float avgCellVolt = 0;

    // Serial.println("getHardware(): ");
    // if (jbdbms.getHardware(hardwareStatus)) {
    //     Serial.printf("Hardware ID: %u\n", hardwareStatus.id);
    // } else {
    //     Serial.println("jbdbms.getHardware() failed");
    // }

    //Serial.println("getCells(): ");
    if (jbdbms.getCells(cellStatus)) {
        for (size_t i = 0; i < 6; i++)
        {
            avgCellVolt += float(cellStatus.voltages[i]);
        }
        avgCellVolt = avgCellVolt / 6 / 1000;
    } else {
        Serial.println("jbdbms.getCells() failed");
    }

    //Serial.println("getStatus(): ");
    if (jbdbms.getStatus(bmsStatus)/* && jbdbms.getCells(cellStatus)*/) {
        float V = float(bmsStatus.voltage) / 100; // convert mV to V
        float A = float(bmsStatus.current) / 100 * -1; // convert mA to A
        // ToDo: Ugly!, find other way
        A+= 0.000001; // avoid -0.00A
        float W = V * (A);

        // oled.clearDisplay();

        // oled.setTextSize(1);
        // oled.setTextColor(WHITE);
        // oled.setCursor(0, 10);
    

        if(A < -0.02) {
            Serial.println("Charging");
            // oled.println("Charging");
        } else if (A > 0.02) {
            Serial.println("Discharging");
            // oled.println("Discharging");
        } else {
            Serial.println("Idle");
            // oled.println("Idle");
        }

        // oled.printf("V: %.2fV (%.2fV)\n", V, avgCellVolt);
        // oled.printf("A: %.2fA (%.2fW)\n", A, W);
        // oled.printf("Temp: %.1fC | %.1fC\n", float(bmsStatus.temperatures[0].lo)/10, float(bmsStatus.temperatures[1].lo)/10);

        Serial.printf("V: %.2fV (%.2fV)\n", V, avgCellVolt);
        Serial.printf("A: %.2fA\n", A);
        Serial.printf("W: %.2fW\n", W);
        Serial.printf("Fault: %u\n", bmsStatus.fault);
        Serial.printf("Mosfet: %u\n", bmsStatus.mosfetStatus);
        Serial.println("--------------------");
        Serial.printf("Temp: %.1fC | %.1fC\n", float(bmsStatus.temperatures[0].lo)/10, float(bmsStatus.temperatures[1].lo)/10);
        Serial.println("--------------------");

        // oled.display(); 
    } else {
        Serial.println("jbdbms.getStatus() failed");
    }
    delay(1000);
}