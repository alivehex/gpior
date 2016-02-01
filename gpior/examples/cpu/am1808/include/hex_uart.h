#ifndef _HEX_UART_H_
#define _HEX_UART_H_

#include "types.h"

extern int UART_open(int ch, uint32 baudrate, unsigned int lbuf_sizeb);
extern int UART_close(int ch);
extern int UART_write(int ch, uint8 *buffer, uint32 lenb);
extern int UART_read(int ch, uint8 *buffer, uint32 lenb);
extern void UART_putc(int ch, char c);
extern int UART_getc(int ch);


#endif // ifndef _UART_H_
