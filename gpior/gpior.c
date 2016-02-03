#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "gpior.h"

extern void delay_ms(uint32_t ms);

/*
 * @ simple LED test
 */
void gpio_led_test(void) {
	/* any platform code should be here */

	/* config IO output */
	GPIOR_CFGIO_OUTPUT(GPIOR_LED_PORT, GPIOR_LED_PIN);
	while(1) {
	 	GPIOR_SETIO(GPIOR_LED_PORT, GPIOR_LED_PIN);
		//gpior_delay_us(0x2ffff);
		delay_ms(500);
		GPIOR_CLRIO(GPIOR_LED_PORT, GPIOR_LED_PIN);
		//gpior_delay_us(0x2ffff);
		delay_ms(500);
	}
}

/** gpio input test **/
void gpio_input_test(void) {
	GPIOR_CFGIO_INPUT_PULLUP(GPIOR_LED_PORT, GPIOR_LED_PIN);
	
	while (1) {
		printf("\r         \r");             
		if (GPIOR_GETIO(GPIOR_LED_PORT, GPIOR_LED_PIN)) {
			printf("io=high");
		} else {
			printf("io=low");
		}
		delay_ms(200);
	}
}

void gpior_delay_us(unsigned int us) {
	while (us --);
}
