#ifndef BMS_H
#define BMS_H
    #include <Arduino.h>

    // BMS Lib
    #include <jbdbms.h>
    #define BMS_RX_PIN 25
    #define BMS_TX_PIN 26
    #define TASK_INTERVAL_BMS 500

    class MyBMS {
        public:
            typedef struct shared_bms_data {
                JbdBms::Status_t status;
                JbdBms::Cells_t cells;
                JbdBms::Hardware_t hardware;
                String rtc_date;
                String charging_state;
                float avgCellVolt;
                float V;
                float A;
                float W;
            } shared_bms_data_t;

            static void initBMS(shared_bms_data_t *myBMSData);
            static void readBMSStatus();
            static void printBMSStatus();

        private:
            static shared_bms_data_t *_myBMSData;
            static void taskCallbackBMS( void * pvParameters );
    };

    
#endif