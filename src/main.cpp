#define FEATURE_BT

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


/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
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

void setup() {
    Wire.begin(I2C_SDA, I2C_SCL);
    Serial.begin(115200);
    delay(2000); // wait for serial

    ++bootCount;
    Serial.println("Boot number: " + String(bootCount));

    print_wakeup_reason();

    rtc_gpio_pullup_en(GPIO_NUM_12);
    rtc_gpio_pulldown_dis(GPIO_NUM_12);
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_12, HIGH);
    
    MyBMS::initBMS(&myBMSData);
    MyLogger::initLogger(&myBMSData);
    MyDisplays::initDisplays(&myBMSData);

    #ifdef FEATURE_BT
        if(myBMSData.bt_enabled) {
            MyBluetooth::initBT();
        }
    #endif

    Serial.println("Finished setup");
    Serial.println("-------------------------------");
}

void loop() {
    
}