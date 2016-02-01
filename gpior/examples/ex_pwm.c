#include "gpior.h"


/*
 * @ PWM example
 */
int PWM_example(int freq_hz, int duty) {
 	pwm_open(0, freq_hz, duty, 1);
	pwm_enable(0);
	return 0;
}
