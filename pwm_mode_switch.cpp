// Copyright (c) 2015, Smart Projects Holdings Ltd
// All rights reserved.
// See LICENSE file for license details.

#include "pwm_mode_switch.h"

Pwm_mode_switch::Pwm_mode_switch(uint8_t mode_count, uint8_t min_pwm, uint8_t max_pwm):
mode_count(mode_count),
max_pwm(max_pwm),
min_pwm(min_pwm)
{
}

bool
Pwm_mode_switch::Set_pwm(uint8_t value)
{
    int pwm = value;
    if (pwm == 0) {
        current_mode = 0;
    } else if (pwm >= max_pwm - mode_count) {
        current_mode = mode_count;
    } else if (pwm <= min_pwm) {
        current_mode = 1;
    } else {
        auto interval = (max_pwm - min_pwm) / (mode_count * 2 - 1);
        pwm -= min_pwm;

        auto mode = pwm / interval;

        if ((mode & 0x01) == 0) {
            // even number.
            current_mode = (mode / 2) + 1;
        }
        // else do not change mode.
    }

    return Mode_changed();
}

bool
Pwm_mode_switch::Mode_changed()
{
    return last_mode != current_mode;
}

uint8_t
Pwm_mode_switch::Read_mode()
{
    if (last_mode != current_mode) {
        auto curtime = micros();
        if (curtime - last_flip_time > FLIP_TIME) {
            flip_count = 1;
        } else {
            flip_count++;
        }
        last_flip_time = curtime;
    }
    last_mode = current_mode;
    return current_mode;
}

uint8_t
Pwm_mode_switch::Read_flip_count()
{
    return flip_count;
}
