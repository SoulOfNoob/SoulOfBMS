#include "bms.h"

#define NOMINAL_CELL_VOLTAGE 3.7

TaskHandle_t TaskHandleBMS;
TaskHandle_t TaskUpdateLidState;
TaskHandle_t TaskUpdateBTState;

JbdBms jbdbms = JbdBms();

MyBMS::shared_bms_data_t *MyBMS::_myBMSData;

void IRAM_ATTR LidISR() {
    xTaskCreate( MyBMS::taskUpdateLidState, "TaskUpdateLidState", 10000, NULL, 2, &TaskUpdateLidState );
}

void IRAM_ATTR BTISR() {
    xTaskCreate( MyBMS::taskUpdateBTState, "TaskUpdateBTState", 10000, NULL, 2, &TaskUpdateBTState );
}

void MyBMS::initBMS(shared_bms_data_t *myBMSData) {
    _myBMSData = myBMSData;

    pinMode(REED_PIN, INPUT_PULLUP);
    pinMode(BT_SWITCH_PIN, INPUT_PULLUP);

    adc1_config_width(ADC_WIDTH_BIT_12);
    adc1_config_channel_atten(ADC1_GPIO35_CHANNEL, ADC_ATTEN_DB_11);

    _myBMSData->bt_enabled = !digitalRead(BT_SWITCH_PIN);
    _myBMSData->lid_open = digitalRead(REED_PIN);

    attachInterrupt(REED_PIN, LidISR, CHANGE);
    attachInterrupt(BT_SWITCH_PIN, BTISR, CHANGE);

    xTaskCreate( taskCallbackBMS, "TaskHandleBMS", 10000, NULL, 2, &TaskHandleBMS );
}

void MyBMS::readBMSStatus() {
    if(
        !jbdbms.getHardware(_myBMSData->hardware) ||
        !jbdbms.getCells(_myBMSData->cells) || 
        !jbdbms.getStatus(_myBMSData->status)
    ) {
        Serial.println("### jbdbms failed exiting ###"); // ToDo Throw error somehow without direct serial print
        return;
    }
    
    // Process Cells
    float totalCellVolt = 0;
    for (size_t i = 0; i < 6; i++) {
        totalCellVolt += float(_myBMSData->cells.voltages[i]);
    }
    _myBMSData->avgCellVolt = totalCellVolt / _myBMSData->status.cells / 1000;

    // Process Stauts
    _myBMSData->temp_01 = JbdBms::deciCelsius(_myBMSData->status.temperatures[0]) / 10;
    _myBMSData->temp_02 = JbdBms::deciCelsius(_myBMSData->status.temperatures[1]) / 10;

    _myBMSData->V = float(_myBMSData->status.voltage) / 100; // convert mV to V
    _myBMSData->A = float(_myBMSData->status.current) / 100; // convert mA to A
    _myBMSData->W = _myBMSData->V * (_myBMSData->A); 

    _myBMSData->remaining_capacity_Ah = float(_myBMSData->status.remainingCapacity) / 100; // remaining capacity as in BMS configured for whole pack
    _myBMSData->remaining_capacity_Wh = _myBMSData->remaining_capacity_Ah * (_myBMSData->status.cells * NOMINAL_CELL_VOLTAGE);

    _myBMSData->total_capacity_Ah = float(_myBMSData->status.nominalCapacity) / 100; // total capacity as in BMS configured for whole pack
    _myBMSData->total_capacity_Wh =  _myBMSData->total_capacity_Ah * (_myBMSData->status.cells * NOMINAL_CELL_VOLTAGE);

    if(_myBMSData->A > 0.02) {
        _myBMSData->charging_state = "Charging";
        _myBMSData->remaining_time_h_cur = (_myBMSData->total_capacity_Wh - _myBMSData->remaining_capacity_Wh) / _myBMSData->W;
    } else if (_myBMSData->A < -0.02) {
        _myBMSData->charging_state = "Discharging";
        _myBMSData->remaining_time_h_cur = (_myBMSData->remaining_capacity_Wh / _myBMSData->W) * -1;
    } else {
        _myBMSData->charging_state = "Idle";
        _myBMSData->remaining_time_h_cur = 0;
    }
}

void MyBMS::readVbat() {
    _myBMSData->vBatInt = adc1_get_raw(ADC1_GPIO35_CHANNEL);
    _myBMSData->vBatFloat = (float) _myBMSData->vBatInt * 3500 / 4095 * 2 / 1000;
}

void MyBMS::taskUpdateLidState( void * pvParameters ) {
    _myBMSData->lid_open = digitalRead(REED_PIN);
    if(!_myBMSData->lid_open && !_myBMSData->bt_enabled) {
        //Go to sleep now
        Serial.println("Going to sleep now");
        delay(2000);
        esp_deep_sleep_start();
        Serial.println("This will never be printed");
    }
    vTaskDelete(NULL);
}

void MyBMS::taskUpdateBTState( void * pvParameters ) {
    _myBMSData->bt_enabled = !digitalRead(BT_SWITCH_PIN);
    esp_sleep_enable_timer_wakeup(100);
    esp_deep_sleep_start();
    //ESP.restart(); // ToDo: init BT on runtime without reboot
    vTaskDelete(NULL);
}

void MyBMS::taskCallbackBMS( void * pvParameters ) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = TASK_INTERVAL_BMS / portTICK_PERIOD_MS;

    Serial1.begin(9600, SERIAL_8N1, BMS_RX_PIN, BMS_TX_PIN);
    for( ;; )
    {
        readBMSStatus();
        readVbat();
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}