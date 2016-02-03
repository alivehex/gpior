#ifndef _PWM_H_
#define _PWM_H_

#include "types.h"

/*
 * @ Open PWM channel with frequency
 * @ freq_hz: freqency in HZ
 * @ duty: [0, 100]
 */
int pwm_open(int ch, int freq_hz, int duty, int active_level);

/*
 * @ PWM set duty
 */
int pwm_set_duty(int ch, int duty);

/*
 * @ PWM running, never returned
 */
int pwm_enable(int ch);

/*
 * @ Disable PWM output
 */
int pwm_disable(int ch);

/*
 * @ Close PWM 
 */
int pwm_close(int ch);


#endif 
