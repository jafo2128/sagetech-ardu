// Copyright (c) 2015, Smart Projects Holdings Ltd
// All rights reserved.
// See LICENSE file for license details.

#include "Arduino.h"
#include "pwm.h"
#include "led.h"
#include "pwm_mode_switch.h"
#include "common.h"

// Led config
#define TRANSPONDER_LED 10      // Led to signal that transponder radio transmitter is ON
#define MAVLINK_LED     11      // Led to signal that mavlink data is received

// PWM reader config
#define PWM_IN_PIN      19      // Pin used by Serial1. Use it for PWM input.
#define PWM_IN_INT       4      // Interrupt used by Serial1. Use it for PWM input.

// Mode switch config
#define TRANSPONDER_MODE_COUNT      4      // 4 different modes: off, on, alt_internal, alt_external
#define TRANSPONDER_MODE_PWM_MIN    100    // minimum pwm
#define TRANSPONDER_MODE_PWM_MAX    200    // max pwm

// Mavlink telemetry serial port config
#define MAVLINK_PORT_BAUD   57600   // baud rate for mavlink port

// Do not send coordinates and altitude more often than 5Hz
static constexpr unsigned long MAVLINK_PERIOD             = 200000;
unsigned long last_mavlink_change_time = 0;

// Sagetech serial port config
#define SAGETECH_PORT_BAUD  57600   // baud rate for sagetech port

// 2000 in octal. Assigned SQUAWK code which must be used at Spilve airport.
#define SQUAWK_CODE_SPILVE  0x400

// Tail number to send via ADSB. Max len = 8 chars. Only uppercase letters and digits allowed.
static const char TAIL_NUMBER[] = "6ARDU";

// Current altitude from Ardupilot.
long current_altitude_in100ft = 0;

bool passthrough_mode = true;

enum class Transponder_mode: uint8_t {
    UNKNOWN         = 0x00, // transponder not connected.
    OFF             = 0x01,
    ON              = 0x02,
    ALT_EXTERNAL    = 0x03,
    ALT_INTERNAL    = 0x04
} current_transponder_mode = Transponder_mode::UNKNOWN;

Pwm_mode_switch mode_switch(        // Initialize pwm mode switch to 4 modes from 100 to 200.
        TRANSPONDER_MODE_COUNT,
        TRANSPONDER_MODE_PWM_MIN,
        TRANSPONDER_MODE_PWM_MAX);

Sagetech_protocol sagetech_protocol;
Mavlink_protocol mavlink_protocol;

// This led is on when transponder is transmitting: ON or ALT mode.
Led transponder_active_led(TRANSPONDER_LED, true);

// This led is on if valid GPS coordinates are present.
Led mavlink_active_led(MAVLINK_LED, false);

HardwareSerial& sagetech_port = Serial2;
HardwareSerial& mavlink_port = Serial3;

HardwareSerial& debug_port = Serial;

void Set_transponder_mode(Transponder_mode mode, bool start_ident = false)
{
    uint8_t sagetech_packet[Sagetech_protocol::MAX_FRAME_LENGTH];

    // ... Create sagetech message here

    auto len = sagetech_protocol.Create_operating_message(oper, sagetech_packet);
    sagetech_port.write(sagetech_packet, len);
    if (!passthrough_mode) {
        debug_port.println();
        debug_port.print("Setting mode to ");
        debug_port.println((int)mode);
    }
}

void On_mavlink_gps(Mavlink_protocol::Message_gps_int msg)
{
    if (msg.lat == 0 && msg.lon == 0 && msg.alt == 0) {
        return; // GPS data invalid. Ignore.
    }

    mavlink_active_led.Blink(20);

    current_altitude_in100ft =  msg.alt / 30480;
    auto clock = micros();

    if ((clock - last_mavlink_change_time) < MAVLINK_PERIOD) {
        return;
    }

    last_mavlink_change_time = clock;

    if (    current_transponder_mode != Transponder_mode::ON
        &&  current_transponder_mode != Transponder_mode::ALT_INTERNAL
        &&  current_transponder_mode != Transponder_mode::ALT_EXTERNAL )
    {
        return; // send gps data only when transponder is ON
    }

    if (current_transponder_mode == Transponder_mode::ALT_EXTERNAL) {
        Set_transponder_mode(current_transponder_mode); // send altitude.
    }

    Sagetech_protocol::Gps_message gps_message;
    uint8_t sagetech_packet[Sagetech_protocol::MAX_FRAME_LENGTH];

    // ... Create sagetech message here

    auto len = sagetech_protocol.Create_gps_message(gps_message, sagetech_packet);
    sagetech_port.write(sagetech_packet, len);
//    Dump_data(sagetech_packet, len, debug_port);
}

void On_sagetech_ack(Sagetech_protocol::Acknowledge_message msg)
{
    unsigned int blink_pause;
    if (msg.ident_active) {
        blink_pause = 600;
    } else {
        blink_pause = 1400;
    }
    switch (msg.transponder_mode) {
    case Sagetech_protocol::Transponder_mode::OFF:
        transponder_active_led.Off();
        current_transponder_mode = Transponder_mode::OFF;
        break;
    case Sagetech_protocol::Transponder_mode::ON:
        transponder_active_led.Blink(20, 300, 1, blink_pause);
        current_transponder_mode = Transponder_mode::ON;
        break;
    case Sagetech_protocol::Transponder_mode::ALT:
        if (msg.altitude_source_external) {
            current_transponder_mode = Transponder_mode::ALT_EXTERNAL;
            transponder_active_led.Blink(20, 300, 3, blink_pause);
        } else {
            current_transponder_mode = Transponder_mode::ALT_INTERNAL;
            transponder_active_led.Blink(20, 300, 2, blink_pause);
        }
        break;
    }
}

void setup()
{
    // Initialize pwm reader interrupt
    Pwm_reader_setup(PWM_IN_PIN, PWM_IN_INT);

    // Initialize serial ports
    mavlink_port.begin(MAVLINK_PORT_BAUD);
    sagetech_port.begin(SAGETECH_PORT_BAUD);
    debug_port.begin(115200);

    // Set flight number (Send preflight message to transponder)
    uint8_t sagetech_packet[Sagetech_protocol::MAX_FRAME_LENGTH];

    // ... Create sagetech message here

    sagetech_port.write(sagetech_packet, len);

}

void loop()
{
    // Read telemetry data from port and feed it into mavlink protocol parser
    if (mavlink_port.available()) {
        // this will call On_mavlink_gps() eventually.
        mavlink_protocol.On_data_byte(mavlink_port.read());
    }

    // Read transponder data from port and feed it into sagetech protocol parser
    if (sagetech_port.available()) {
        auto data_byte = sagetech_port.read();
        // this will call On_sagetech_ack() eventually.
        sagetech_protocol.On_data_byte(data_byte);
        if (passthrough_mode) {
            Serial.write(data_byte);
        }
    }

    if (Serial.available()) {
        if (passthrough_mode) {
            sagetech_port.write(Serial.read());
        }
    }

    // Read pwm, caculate mode and put transponder in desired mode.
    if (mode_switch.Set_pwm(Pwm_reader_get())) {

        auto mode = mode_switch.Read_mode();

        bool ident_on = (mode_switch.Read_flip_count() == 4);

        switch (mode) {
        case 0:
        case 1:
            Set_transponder_mode(Transponder_mode::OFF);
            passthrough_mode = true;
            break;
        case 2:
            Set_transponder_mode(Transponder_mode::ON, ident_on);
            passthrough_mode = false;
            break;
        case 3:
            Set_transponder_mode(Transponder_mode::ALT_INTERNAL, ident_on);
            passthrough_mode = false;
            break;
        case 4:
            Set_transponder_mode(Transponder_mode::ALT_EXTERNAL, ident_on);
            passthrough_mode = false;
            break;
        }
    }

    Pwm_reader_loop();

    mavlink_active_led.Loop();

    transponder_active_led.Loop();
}
