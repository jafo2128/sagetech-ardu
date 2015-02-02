// Copyright (c) 2015, Smart Projects Holdings Ltd
// All rights reserved.
// See LICENSE file for license details.

#include "led.h"

Led::Led(uint8_t pin, bool initial_state_on)
:pin(pin){
    pinMode(pin, OUTPUT);
    if (initial_state_on) {
        On();
    } else {
        Off();
    }
}

void
Led::On()
{
    Turn_on();
    blink_count = 0;
}

void
Led::Off()
{
    Turn_off();
    blink_count = 0;
}

void
Led::Turn_on()
{
    digitalWrite(pin, HIGH);
    on_time = micros();
    is_on = true;
}

void
Led::Turn_off()
{
    digitalWrite(pin, LOW);
    off_time = micros();
    is_on = false;
}

void
Led::Blink(unsigned int ontime, unsigned int offtime, unsigned int n, unsigned int pause)
{
    on_interval = (unsigned long)ontime * 1000;
    off_interval = (unsigned long)offtime * 1000;
    pause_time = (unsigned long)pause * 1000;
    blink_count = n;
}

void
Led::Loop()
{
    if (blink_count == 0) {
        return;
    }

    unsigned long clock = micros();

    if (is_on) {
        if (clock - on_time > on_interval) {
            current_blink_count++;
            if (off_interval == 0) {
                blink_count = 0;
            }
            Turn_off();
        }
    } else {
        unsigned long pause;
        if (current_blink_count >= blink_count) {
            pause = pause_time;
        } else {
            pause = off_interval;
        }
        if (clock - off_time > pause) {
            if (current_blink_count >= blink_count) {
                current_blink_count = 0;
            }
            Turn_on();
        }
    }
}
