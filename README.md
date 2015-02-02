Sample code to show how to integrate ADSB transponder with autopilot.
This is Arduino sketch used to connect Sagetech ADSB transponder to Ardupilot autopilot.

Features:

1) Separate RC channel is used to drive the transponder On/Off.
Arduino reads the pwm signal from RC receiver and sets the transponder mode accordingly.

2) Mavlink message GLOBAL_POSITION_INT is parsed and GPS coordinates sent to ADSB transponder.

3) Two LEDs are used to show current status of transponder.

Unfortunately we cannot give out any details on the protocol used by Sagetech transponder. Therefore the code does not compile and is to be used for reference only.
