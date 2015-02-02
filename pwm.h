// Copyright (c) 2015, Smart Projects Holdings Ltd
// All rights reserved.
// See LICENSE file for license details.

#ifndef PWM_H_
#define PWM_H_

void
Pwm_reader_setup(uint8_t pin, uint8_t intr);

uint8_t
Pwm_reader_get();

void
Pwm_reader_loop();

#endif /* PWM_H_ */
