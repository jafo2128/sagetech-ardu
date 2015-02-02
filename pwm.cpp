// Copyright (c) 2015, Smart Projects Holdings Ltd
// All rights reserved.
// See LICENSE file for license details.

#include "Arduino.h"

// make variables local to this module.
namespace {

constexpr unsigned long MIN_PWM_HIGH_TRESHOLD_USEC = 50;
constexpr unsigned long MAX_PWM_HIGH_TRESHOLD_USEC = 2550;
constexpr unsigned long MAX_PWM_LOW_TRESHOLD_USEC = 25000;

volatile uint8_t pwm_value = 0;
volatile bool pwm_value_changed = 0;
volatile unsigned long pwm_last_changed_at = 0;
volatile int8_t pwm_pin = 0;
}

void
Pwm_reader_isr()
{
    auto cur = micros();
    auto len = cur - pwm_last_changed_at;
    pwm_last_changed_at = cur;
    if (digitalRead(pwm_pin) == LOW) {
        if (len > MIN_PWM_HIGH_TRESHOLD_USEC && len < MAX_PWM_HIGH_TRESHOLD_USEC) {
            pwm_value = len / 10 + 1;
            pwm_value_changed = true;
        }
    }
}

void
Pwm_reader_setup(uint8_t pin,uint8_t intr)
{
    pwm_pin = pin;
    pinMode(pwm_pin, INPUT);
    attachInterrupt(intr, Pwm_reader_isr, CHANGE);
}

void
Pwm_reader_loop()
{
    if (pwm_value_changed) {
        pwm_value_changed = false;
        noInterrupts();
        if (micros() - pwm_last_changed_at > MAX_PWM_LOW_TRESHOLD_USEC) {
            pwm_value = 0;
        }
        interrupts();
    }
}

uint8_t
Pwm_reader_get()
{
    return pwm_value;
}
