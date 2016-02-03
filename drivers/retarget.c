#include "gpior.h"
#include <stdio.h>

#if (GPIOR_UART_STDOUT)
#if (!GPIOR_UART)
#error STDOUT NEEDS UART DRIVERS!
#endif

static boolean uart_init = FALSE;

struct __FILE {
	int handle;
};

struct __FILE __stdout;
struct __FILE __stdin;
struct __FILE __stderr;


static void call_for_uart() {
	if(uart_init == FALSE) {
		if(uart_open(GPIOT_STDOUT_UARTCH, UART_BAUDRATE) == 0) {
			uart_init = TRUE;
		}
	}
}

int fgetc(FILE *f) {		
	call_for_uart();
	return uart_read_byte(GPIOT_STDOUT_UARTCH);
}

int fputc(int ch, FILE *f) {
	call_for_uart();
	uart_write_byte(GPIOT_STDOUT_UARTCH, ch);
	return ch;
}

int ferror(FILE *f) {
	return -1;
}

int __backspace(FILE *stream) {
	return 0;
}
#endif // if(GPIOR_UART_STDOUT)

