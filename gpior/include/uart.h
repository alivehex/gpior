#ifndef _UART_H_
#define _UART_H_

#include "types.h"

/*
 * @ UART open
 * @ baudrate, N, 8, 1, LSB
 */
int uart_open(int ch, uint32 baudrate);

/*
 * @ UART write byte
 */
void uart_write_byte(int ch, uint8 c);

/*
 * @ UART read byte
 */
uint8 uart_read_byte(int ch);

/*
 * @ close UART channel
 */
int uart_close(int ch);

#endif // ifndef _UART_H_
