#include "bms.h"

TaskHandle_t TaskHandleBMS;
TaskHandle_t TaskHandleVbat;

JbdBms jbdbms = JbdBms();

MyBMS::shared_bms_data_t *MyBMS::_myBMSData;

esp_adc_cal_characteristics_t *adc_chars = new esp_adc_cal_characteristics_t;

uint16_t MyBMS::avgAnalogRead(adc1_channel_t channel, uint16_t samples) {
  uint32_t sum = 0;
  for (int x=0; x<samples; x++) {
    sum += adc1_get_raw(channel);
  }
  sum /= samples;
  return esp_adc_cal_raw_to_voltage(sum, adc_chars);
}

void MyBMS::init(shared_bms_data_t *myBMSData) {
    _myBMSData = myBMSData;

    xTaskCreate( taskCallbackBMS, "TaskHandleBMS", 10000, NULL, 2, &TaskHandleBMS );
    xTaskCreate( taskCallbackVbat, "TaskHandleVbat", 10000, NULL, 2, &TaskHandleVbat );
}

void MyBMS::initBMS() {
    Serial1.begin(9600, SERIAL_8N1, BMS_RX_PIN, BMS_TX_PIN);
}

void MyBMS::initVbat() {
    adc1_config_width(ADC_WIDTH_BIT_11);
    adc1_config_channel_atten((adc1_channel_t) ADC1_GPIO35_CHANNEL, ADC_ATTEN_DB_11);
    esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_11, ADC_VMAX, adc_chars);
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
    _myBMSData->vBatRaw = avgAnalogRead((adc1_channel_t) ADC1_GPIO35_CHANNEL, 8);
    _myBMSData->vBatFloat = (float) _myBMSData->vBatRaw * 2 / 1000;   // measured mV to actual V (*2 due to voltagedivider)
}

void MyBMS::taskCallbackBMS( void * pvParameters ) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = TASK_INTERVAL_BMS / portTICK_PERIOD_MS;
    initBMS();
    for( ;; )
    {
        readBMSStatus();
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

void MyBMS::taskCallbackVbat( void * pvParameters ) {
    TickType_t xLastWakeTime = xTaskGetTickCount();
    const TickType_t xFrequency = TASK_INTERVAL_VBAT / portTICK_PERIOD_MS;
    initVbat();
    for( ;; )
    {
        readVbat();
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}