// Copyright (c) 2015, Smart Projects Holdings Ltd
// All rights reserved.
// See LICENSE file for license details.

#ifndef LED_H_
#define LED_H_

#include "Arduino.h"

class Led {
public:
    Led(uint8_t pin, bool initial_state_on);

    void
    On();

    void
    Off();

    // Blink the led. Should call Loop() in the loop().
    // on  - On time in milliseconds
    // off - Off time in milliseconds
    void
    Blink(unsigned int on, unsigned int off = 0, unsigned int times = 1, unsigned int pause = 0);

    void Loop();
private:
    void
    Turn_on();

    void
    Turn_off();

    unsigned long off_time = 0;
    unsigned long on_time = 0;
    unsigned long off_interval = 0;
    unsigned long on_interval = 0;
    unsigned int blink_count = 0;
    unsigned long pause_time = 0;
    unsigned int current_blink_count = 0;
    uint8_t pin;
    bool is_on;
};

#endif /* LED_H_ */
