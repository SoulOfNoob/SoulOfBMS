#include <Arduino.h>
#include <Wire.h>

#include "logging.h"
#include "display.h"
#include "bluetooth.h"
#include "bms.h"
#include "driver/rtc_io.h"

#define I2C_SDA 32
#define I2C_SCL 33

#define REED_PIN 12

RTC_DATA_ATTR int bootCount = 0;

MyBMS::shared_bms_data_t myBMSData;

TaskHandle_t TaskUpdateLidState;
TaskHandle_t TaskUpdateBTState;

void print_wakeup_reason(){
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch(wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0 : Serial.println("Wakeup caused by external signal using RTC_IO"); break;
    case ESP_SLEEP_WAKEUP_EXT1 : Serial.println("Wakeup caused by external signal using RTC_CNTL"); break;
    case ESP_SLEEP_WAKEUP_TIMER : Serial.println("Wakeup caused by timer"); break;
    case ESP_SLEEP_WAKEUP_TOUCHPAD : Serial.println("Wakeup caused by touchpad"); break;
    case ESP_SLEEP_WAKEUP_ULP : Serial.println("Wakeup caused by ULP program"); break;
    default : Serial.printf("Wakeup was not caused by deep sleep: %d\n",wakeup_reason); break;
  }
}

void taskUpdateLidState( void * pvParameters ) {
    myBMSData.lidOpen = digitalRead(REED_PIN);
    if(!myBMSData.lidOpen && !myBMSData.btEnabled) {
        Serial.println("Going to sleep now");
        delay(2000);
        esp_deep_sleep_start();
    }
    vTaskDelete(NULL);
}

void taskUpdateBTState( void * pvParameters ) {
    myBMSData.btEnabled = !digitalRead(BT_SWITCH_PIN);
    esp_sleep_enable_timer_wakeup(100);
    esp_deep_sleep_start();
    //ESP.restart(); // ToDo: init BT on runtime without reboot
    vTaskDelete(NULL);
}

void IRAM_ATTR LidISR() {
    // BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // const UBaseType_t xArrayIndex = 2;
    // vTaskNotifyGiveIndexedFromISR(xTaskGetHandle("TaskHandleOLED"), xArrayIndex, &xHigherPriorityTaskWoken );
    xTaskCreate( taskUpdateLidState, "TaskUpdateLidState", 10000, NULL, 2, &TaskUpdateLidState );
}

void IRAM_ATTR BTISR() {
    // BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    // const UBaseType_t xArrayIndex = 1;
    // vTaskNotifyGiveIndexedFromISR(xTaskGetHandle("TaskHandleOLED"), xArrayIndex, &xHigherPriorityTaskWoken );
    xTaskCreate( taskUpdateBTState, "TaskUpdateBTState", 10000, NULL, 2, &TaskUpdateBTState );
}

void setupInterrupts() {
    pinMode(REED_PIN, INPUT_PULLUP);
    pinMode(BT_SWITCH_PIN, INPUT_PULLUP);

    rtc_gpio_pullup_en((gpio_num_t) REED_PIN);
    rtc_gpio_pulldown_dis((gpio_num_t) REED_PIN);
    esp_sleep_enable_ext0_wakeup((gpio_num_t) REED_PIN, HIGH);

    attachInterrupt(REED_PIN, LidISR, CHANGE);
    attachInterrupt(BT_SWITCH_PIN, BTISR, CHANGE);

    myBMSData.lidOpen = digitalRead(REED_PIN);
    myBMSData.btEnabled = !digitalRead(BT_SWITCH_PIN);
}

void setup() {
    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.begin(115200);
    delay(2000); // wait for serial

    ++bootCount;
    Serial.println("Boot number: " + String(bootCount));

    print_wakeup_reason();

    setupInterrupts();
    
    MyBMS::init(&myBMSData);
    MyLogger::init(&myBMSData);
    MyDisplays::init(&myBMSData);

    if(myBMSData.btEnabled) {
        MyBluetooth::init();
    }

    Serial.println("Finished setup");
    Serial.println("-------------------------------");
}

void loop() {
    vTaskDelay(portMAX_DELAY);
}