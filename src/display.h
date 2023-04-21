#ifndef DISPLAY_H
#define DISPLAY_H

    #include <Arduino.h>

    // OLED Libs
    #include <Adafruit_GFX.h>
    #include <Adafruit_SSD1306.h>

    // ePaper Libs
    #include <GxEPD2_BW.h>
    #include <Fonts/FreeMonoBold9pt7b.h>

    void epaperHelloWorld();
    void initEPAPERTask( void * pvParameters );

    bool initOLED();
    bool initEPAPER();

#endif