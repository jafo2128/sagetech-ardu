# Sagetech ADS-B Transponder integration with UAV 

Sample code to show how to integrate ADSB transponder with autopilot.
This is Arduino sketch used to connect Sagetech ADSB transponder to Ardupilot autopilot.

#### Build instructions

The code can be built with Arduino IDE version 1.5.8+.

Code uses C++11 standard, so it must be compiled with -std=c++11 option.

Unfortunately Arduino IDE does not allow adding compiler flags in GUI. You  need to manually edit config file:
- Open file {your arduino ide path}/hardware/arduino/avr/platform.txt
- Find line containing "compiler.cpp.flags"
- Append -std=c++11 at the line end.
- Save and launch Arduino IDE. You should be able to build the sketch, now.

### Transponder mode change
Separate RC channel is used to drive the transponder On/Off.
Arduino reads the pwm signal from RC receiver and sets the transponder mode accordingly. 
  
There are four modes of operation:

| Mode | Description|
| --- | --- |
| OFF | Transponder is swithched off |
| ON | Transponder is swithched off |
| ALT (internal) | Transponder is using altitude from internal barometer |
| ALT (external) | Transponder is using altitude reported from flight controller via MAVLINK telemetry connection |

### GPS coordinates
Telemetry link from flight controller is connected to Arduino and Mavlink message GLOBAL_POSITION_INT is parsed and GPS coordinates sent to ADSB transponder.

### Status LEDs
Two LEDs are used to show current status of transponder.

Red LED is used to show transponder state:

| Red LED | Description |
| --- | --- |
| Solid | Transponder not detected. |
| Off | Transponder connected but ADSB transmitter radio is turned off. |
| 1 Blink, long pause | Transponder in ON mode. (Altitude is not being broadcast)|
| 1 Blink, short pause | Transponder in ON mode and IDENT is activated.|
| 2 Blinks, long pause | Transponder in ALT mode. (Altitude is taken from transponder internal barometer)|
| 2 Blinks, short pause | Transponder in ALT mode and IDENT is activated.|
| 3 Blinks, long pause | Transponder in ALT mode. (Altitude is taken from telemetry received from autopilot)|
| 3 Blinks, short pause | Transponder in ALT mode and IDENT is activated.|

Green LED is used to show telemetry link state:

| Green LED | Description |
| --- | --- |
| Off | Telemetry is not present or there is no valid GPS data.|
| Blinking | Valid GPS data is present. (blinks on each valid GLOBAL_POSITION_INT message)|


### IDENT mode

Transponder can be put into IDENT mode when it broadcasts SQUAWK code for 18 seconds. Can be activated only while transponder is in ON or ALT mode.

To turn on IDENT mode flip the switch between ON and ALT modes 4 times.
