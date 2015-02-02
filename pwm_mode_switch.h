// Copyright (c) 2015, Smart Projects Holdings Ltd
// All rights reserved.
// See LICENSE file for license details.

#ifndef PWM_MODE_SWITCH_H_
#define PWM_MODE_SWITCH_H_

#include "Arduino.h"

// Modes from pwm signal.
/** PWM signal is measured on 10microseconds and can be from 1..255.
 *
 * Example:
 *
 *  PWM not present -> Change to mode 0
 * /         min_pwm
 * |        /    Change to mode 1
 * |        |   /         Change to mode 2             n = mode_count
 * |        |  /         /          Change to mode 3  /   max_pwm
 * |        |  |         |         /                  |  /
 * 011 .. 11|1111|----|2222|----|3333|-- ........ --|nnnn|nnnnnn
 *
 */

class Pwm_mode_switch {
public:

    Pwm_mode_switch(uint8_t mode_count, uint8_t min_pwm, uint8_t max_pwm);

    // pwm==0 means pwm disconnected.
    // returns true if pwm changed the current mode.
    bool
    Set_pwm(uint8_t pwm);

    // Return a number in range [0..mode_count].
    // 0 means pwm disconnected.
    uint8_t
    Read_mode();

    // Return number of times the switch was flipped between mode 1 and any other mode.
    uint8_t
    Read_flip_count();

    bool
    Mode_changed();
private:

    // To record a flip, switch must be flipped within .5 seconds.
    static constexpr unsigned long FLIP_TIME = 500000;

    uint8_t current_mode = 0;
    uint8_t last_mode = 0;

    uint8_t flip_count = 0;
    unsigned long last_flip_time = 0;

    uint8_t mode_count;
    uint8_t max_pwm;
    uint8_t min_pwm;
};

#endif /* PWM_MODE_SWITCH_H_ */
