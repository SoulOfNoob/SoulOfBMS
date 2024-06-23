#define FEATURE_BT

#include <Arduino.h>
#include <Wire.h>

#include "logging.h"
#include "display.h"
#include "bluetooth.h"
#include "bms.h"

#define I2C_SDA 32
#define I2C_SCL 33

#define REED_PIN 12
#define BT_SWITCH_PIN 27
#define TASK_INTERVAL_REBOOT 1000 * 60 * 10 // 10 minutes

MyBMS::shared_bms_data_t myBMSData;

bool bt_enabled = false;

// TaskHandle_t TaskHandleReboot;

// workaround, should be done better or sleep.
// void rebootCallback( void * pvParameters ) {
//     TickType_t xLastWakeTime;
//     const TickType_t xFrequency = TASK_INTERVAL_REBOOT / portTICK_PERIOD_MS;
//     xLastWakeTime = xTaskGetTickCount ();
//     for( ;; )
//     {
//         vTaskDelayUntil( &xLastWakeTime, xFrequency );
//         Serial.println("Scheduled reboot");
//         ESP.restart();
//     }
// }

void setup() {
    pinMode(REED_PIN, INPUT_PULLUP);
    pinMode(BT_SWITCH_PIN, INPUT_PULLUP);

    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.begin(115200);
    delay(2000); // wait for serial
    
    MyBMS::initBMS(&myBMSData);
    MyLogger::initLogger(&myBMSData);
    MyDisplays::initDisplays(&myBMSData);

    bt_enabled = !digitalRead(BT_SWITCH_PIN);

    #ifdef FEATURE_BT
        if(bt_enabled) {
            MyBluetooth::initBT();
        }
    #endif

    //xTaskCreate( rebootCallback, "TaskHandleReboot", 10000, NULL, 1, &TaskHandleReboot );

    Serial.println("Finished setup");
    Serial.println("-------------------------------");

}

void loop() {
    
}