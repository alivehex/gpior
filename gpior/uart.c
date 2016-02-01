#include "gpior.h"

#if (GPIOR_UART)

static int UART_DELAY_VALUE = 0;

#define UART_DELAY()	GPIOR_DELAYUS(UART_DELAY_VALUE)

#define UART_WRITE_BIT(bit) (bit) ? (GPIOR_SETIO(UART_PORT, UART_TXD_PIN)) : (GPIOR_CLRIO(UART_PORT, UART_TXD_PIN));\
							UART_DELAY()

#define UART_READ_BIT(bit)	(GPIOR_GETIO(UART_PORT, UART_RXD_PIN)) ? (bit) : 0;UART_DELAY()

#define BIT_MASK(bit)	(1u << bit)

/*
 * config UART TXD and RXD
 */
static __inline void UART_GPIO_INIT(void) {
	UART_PLAT_INIT();
	GPIOR_CFGIO_OUTPUT(UART_PORT, UART_TXD_PIN);
	GPIOR_CFGIO_INPUT(UART_PORT, UART_RXD_PIN);
	GPIOR_SETIO(UART_PORT, UART_TXD_PIN);
}

/*
 * @ UART open
 * @ baudrate, N, 8, 1, LSB
 */
int uart_open(int ch, uint32 baudrate) {
	if(baudrate > GPIOR_MAX_FREQHZ)
		return -1;	
 	UART_GPIO_INIT();
	UART_DELAY_VALUE = 1000000 / baudrate - 1;
	return 0;
}

/*
 * @ UART write byte
 */
void uart_write_byte(int ch, uint8 c) {
	uint8 cache[8];
	int i;

	for(i = 0; i < 8; i ++) { 
		if(c & (1 << i))
			cache[i] = 1;
		else 
			cache[i] = 0;
	}
	/* 1 start bit */
	UART_WRITE_BIT(0);
	/* 8 bits data */
	UART_WRITE_BIT(cache[0]);
	UART_WRITE_BIT(cache[1]);
	UART_WRITE_BIT(cache[2]);
	UART_WRITE_BIT(cache[3]);
	UART_WRITE_BIT(cache[4]);
	UART_WRITE_BIT(cache[5]);
	UART_WRITE_BIT(cache[6]);
	UART_WRITE_BIT(cache[7]);
	/* 1 stop bit */
	UART_WRITE_BIT(1);
}

/*
 * @ UART read byte
 */
uint8 uart_read_byte(int ch) {
	uint8 cache = 0;

	/* 1 start bit */
	while(GPIOR_GETIO(UART_PORT, UART_RXD_PIN));
	UART_DELAY();
	/* 8 bits data */
	cache |= UART_READ_BIT(BIT_MASK(0));
	cache |= UART_READ_BIT(BIT_MASK(0));
	cache |= UART_READ_BIT(BIT_MASK(0));
	cache |= UART_READ_BIT(BIT_MASK(0));
	cache |= UART_READ_BIT(BIT_MASK(0));
	cache |= UART_READ_BIT(BIT_MASK(0));
	cache |= UART_READ_BIT(BIT_MASK(0));
	cache |= UART_READ_BIT(BIT_MASK(0));
	/* 1 stop bit */
	return cache;
}

/*
 * @ close UART channel
 */
int uart_close(int ch) {
	GPIOR_CFGIO_INPUT(UART_PORT, UART_TXD_PIN);
	GPIOR_CFGIO_INPUT(UART_PORT, UART_RXD_PIN);
	return 0;
}

#endif // if (GPIOR_UART)
