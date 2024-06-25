#ifndef BMS_H
#define BMS_H
    #include <Arduino.h>
    #include <driver/adc.h>
    #include <soc/adc_channel.h>
    #include <esp_adc_cal.h>
    #include "driver/rtc_io.h"

    // BMS Lib
    #include <jbdbms.h>
    #define BMS_RX_PIN 25
    #define BMS_TX_PIN 26

    #define REED_PIN 12
    #define BT_SWITCH_PIN 27

    #define ADC_VMAX 3300
    #define NOMINAL_CELL_VOLTAGE 3.7

    #define TASK_INTERVAL_BMS 1000
    #define TASK_INTERVAL_VBAT 1000
    

    class MyBMS {
        public:
            typedef struct shared_bms_data {
                String charging_state = "Booting";
                JbdBms::Status_t status;
                JbdBms::Cells_t cells;
                JbdBms::Hardware_t hardware;
                float avgCellVolt;
                float V;
                float A;
                float W;
                float remaining_time_h_cur;
                float remaining_time_h_nom;
                float remaining_capacity_Ah;
                float remaining_capacity_Wh;
                float total_capacity_Ah;
                float total_capacity_Wh;
                float temp_01;
                float temp_02;
                float vBatFloat;
                int vBatRaw;
                bool lid_open;
                bool bt_enabled;
            } shared_bms_data_t;

            static void init(shared_bms_data_t *myBMSData);
            static void readBMSStatus();
            static void readVbat();
            static void taskUpdateLidState( void * pvParameters );
            static void taskUpdateBTState( void * pvParameters );
            static uint16_t avgAnalogRead(adc1_channel_t channel, uint16_t samples = 8);

        private:
            static shared_bms_data_t *_myBMSData;
            static void initBMS();
            static void initVbat();
            static void taskCallbackBMS( void * pvParameters );
            static void taskCallbackVbat( void * pvParameters );
    };

    
#endif