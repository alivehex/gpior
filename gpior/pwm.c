#include "gpior.h"

#if (GPIOR_PWM)

static int PWM_PERIOD = 0;
static int PWM_DUTY = 0;
static int PWM_ACTLV = 0;

static __inline void PWM_GPIO_INIT() {
	PWM_PLAT_INIT();
	GPIOR_CFGIO_OUTPUT(PWM_PORT, PWM_PIN);
}

/*
 * @ open PWM channel with frequency
 * @ freq_hz: freqency in HZ
 * @ duty: [0, 100]
 */
int pwm_open(int ch, int freq_hz, int duty, int active_level) {
	if(freq_hz > 1000000)
		return -1;
	PWM_GPIO_INIT();	
	PWM_ACTLV = active_level;
	PWM_PERIOD = 1000000 / freq_hz; 
	PWM_DUTY = PWM_PERIOD * duty / 100;
 	return 0;
}

/*
 * @ pwm set duty
 */
int pwm_set_duty(int ch, int duty) {
 	return 0;
}

/*
 * @ PWM running, never returned
 */
int pwm_enable(int ch) {
	int tem;

	tem = PWM_PERIOD - PWM_DUTY;
	while(1) {
		(PWM_ACTLV == 1) ? (GPIOR_SETIO(PWM_PORT, PWM_PIN)) : (GPIOR_CLRIO(PWM_PORT, PWM_PIN));
		GPIOR_DELAYUS(PWM_DUTY);
		(PWM_ACTLV == 0) ? (GPIOR_SETIO(PWM_PORT, PWM_PIN)) : (GPIOR_CLRIO(PWM_PORT, PWM_PIN));
		GPIOR_DELAYUS(tem);	
	}
}

/*
 * @ Disable PWM output
 */
int pwm_disable(int ch) {
 	return 0;
}

/*
 * @ Close PWM 
 */
int pwm_close(int ch) {
	GPIOR_CFGIO_INPUT(PWM_PORT, PWM_PIN);
	return 0;
}

#endif // if (GPIOR_PWM)
