#ifndef BMS_H
#define BMS_H
    #include <Arduino.h>
    #include <driver/adc.h>
    #include <soc/adc_channel.h>

    // BMS Lib
    #include <jbdbms.h>
    #define BMS_RX_PIN 25
    #define BMS_TX_PIN 26
    #define REED_PIN 12
    #define BT_SWITCH_PIN 27
    #define TASK_INTERVAL_BMS 500
    

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
                int vBatInt;
                bool lid_open;
                bool bt_enabled;
            } shared_bms_data_t;

            static void initBMS(shared_bms_data_t *myBMSData);
            static void readBMSStatus();
            static void readVbat();
            static void taskUpdateLidState( void * pvParameters );
            static void taskUpdateBTState( void * pvParameters );

        private:
            static shared_bms_data_t *_myBMSData;
            static void taskCallbackBMS( void * pvParameters );
    };

    
#endif