# States

## Main

    - Initialize
      - init Helper
      - init Displays
      - init BMS
      - init BT
    - Transition in
      - 
    - Running
      - 
    - Transition out
      - Hibernate (Final)

## OLED

    - Initialize
      - Create OLED task
    - Transition in
      - Clear display
      - Set cursor
      - Set text size
      - Resume OLED task
    - Running
      - Wait for Update notification 
        - Update Display
    - Transition out
      - Suspend OLED task
      - Clear display

## Logger

    - Initialize
      - Create Logger task
    - Transition in
      - Resume Logger task
    - Running
      - Wait for Update notification
        - Read RTC
        - If enough time since last log
          - Write Update to File
    - Transition out
      - Suspend Logger task

## Bluetooth

    - Initialize
      - Create BT task
    - Transition in
      - Suspend BMS task
      - Resume BT task
    - Running
      - Pipe BT -> Serial1
    - Transition out
      - Suspend BT task
      - Resume BMS task

## BMS

    - Initialize
      - Create BMS task
    - Transition in
      - Resume BMS task
    - Running
      - Read BMS values
      - Lock memory
      - Write BMS values to memory
      - Unlock memory
      - Notify OLED
      - Notify Logger
      - Wait for next execution
    - Transition out
      - Suspend BMS task

## EPAPER

    - Initialize
      - Create EPAPER task
    - Transition in
      - Resume EPAPER task
    - Running
      - Read Logfile
      - Update EPAPER
      - Wait for next execution
    - Transition out
      - Suspend EPAPER task
