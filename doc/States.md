# States

## OLED

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

    - Transition in
      - Resume Logger task
    - Running
      - Wait for Update notification
        - Read RTC
        - Write Update to File
    - Transition out
      - Suspend Logger task

## Bluetooth

    - Transition in
      - Suspend BMS task
      - Resume BT task
    - Running
      - Pipe BT -> Serial1
    - Transition out
      - Suspend BT task
      - Resume BMS task

## BMS

    - Transition in
      - Resume BMS task
    - Running
      - Read BMS values
      - Write BMS values to memory
      - Notify OLED
      - Notify Logger
      - Wait for next execution
    - Transition out
      - Suspend BMS task

## EPAPER

    - Transition in
      - Resume EPAPER task
    - Running
      - Read Logfile
      - Update EPAPER
      - Wait for next execution
    - Transition out
      - Suspend EPAPER task
