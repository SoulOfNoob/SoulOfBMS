#include "bms.h"

TaskHandle_t TaskHandleBMS;

JbdBms jbdbms = JbdBms();

MyBMS::shared_bms_data_t *MyBMS::_myBMSData;

void MyBMS::initBMS(shared_bms_data_t *myBMSData) {
    _myBMSData = myBMSData;
    xTaskCreate( taskCallbackBMS, "TaskHandleBMS", 10000, NULL, 2, &TaskHandleBMS );
}

void MyBMS::readBMSStatus() {
    if (!jbdbms.getHardware(_myBMSData->hardware)) {
        Serial.println("jbdbms.getHardware() failed");
    }
    
    if (!jbdbms.getCells(_myBMSData->cells)) {
        Serial.println("jbdbms.getCells() failed");
    } else {
        float avgCellVolt = 0;
        for (size_t i = 0; i < 6; i++) {
            avgCellVolt += float(_myBMSData->cells.voltages[i]);
        }
        _myBMSData->avgCellVolt = avgCellVolt / 6 / 1000;
    }

    if (!jbdbms.getStatus(_myBMSData->status)) {
        Serial.println("jbdbms.getStatus() failed");
    } else {
        _myBMSData->V = float(_myBMSData->status.voltage) / 100; // convert mV to V
        _myBMSData->A = float(_myBMSData->status.current) / 100 * -1; // convert mA to A
        // ToDo: Ugly!, find other way
        _myBMSData->A+= 0.000001; // avoid -0.00A
        _myBMSData->W = _myBMSData->V * (_myBMSData->A);

        if(_myBMSData->A < -0.02) {
            _myBMSData->charging_state = "Charging";
        } else if (_myBMSData->A > 0.02) {
            _myBMSData->charging_state = "Discharging";
        } else {
            _myBMSData->charging_state = "Idle";
        }
        if (_myBMSData->charging_state != "Idle") {
            float remaining_capacity_Ah = float(_myBMSData->status.remainingCapacity) / 100;
            float remaining_capacity_Wh = remaining_capacity_Ah * _myBMSData->V;

            _myBMSData->remaining_time_h_cur = remaining_capacity_Wh / _myBMSData->W;
        } else {
            _myBMSData->remaining_time_h_cur = 0;
        }
    }
}

void MyBMS::printBMSStatus() {
    Serial.println(_myBMSData->charging_state);
    Serial.println(_myBMSData->remaining_time_h_cur);
    Serial.printf("V: %.2fV (%.2fV)\n", _myBMSData->V, _myBMSData->avgCellVolt);
    Serial.printf("A: %.2fA\n", _myBMSData->A);
    Serial.printf("W: %.2fW\n", _myBMSData->W);
    Serial.printf("Fault: %u\n", _myBMSData->status.fault);
    Serial.printf("Mosfet: %u\n", _myBMSData->status.mosfetStatus);
    Serial.println("--------------------");
    Serial.printf("Temp: %.1fC | %.1fC\n", float(_myBMSData->status.temperatures[0].lo)/10, float(_myBMSData->status.temperatures[1].lo)/10);
    Serial.println("--------------------");
}

void MyBMS::taskCallbackBMS( void * pvParameters ) {
    Serial1.begin(9600, SERIAL_8N1, BMS_RX_PIN, BMS_TX_PIN);

    TickType_t xLastWakeTime;
    const TickType_t xFrequency = TASK_INTERVAL_BMS / portTICK_PERIOD_MS;
    xLastWakeTime = xTaskGetTickCount ();
    for( ;; )
    {
        readBMSStatus();
        printBMSStatus();

        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}