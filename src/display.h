#ifndef DISPLAY_H
#define DISPLAY_H

    #include <Arduino.h>

    // OLED Libs
    #include <Adafruit_GFX.h>
    #include <Adafruit_SSD1306.h>

    // ePaper Libs
    #include <GxEPD2_BW.h>
    #include <Fonts/FreeMonoBold9pt7b.h>

    #include "bms.h"

    #define TASK_INTERVAL_OLED 500 // 1 sec
    #define TASK_INTERVAL_EPAPER 1000 * 60 * 5 // 15 min

    class MyDisplays {
        public:
            static void initDisplays(MyBMS::shared_bms_data_t *myBMSData);

            static void initOLED();
            static void initEPAPER();

            static void epaperHelloWorld();

            static void updateOLED();
            static void updateEPAPER();

        private:
            static MyBMS::shared_bms_data_t *_myBMSData;
            static void taskCallbackOLED( void * pvParameters );
            static void taskCallbackEPAPER( void * pvParameters );
    };

    

#endif