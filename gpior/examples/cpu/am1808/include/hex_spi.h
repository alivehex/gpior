#ifndef _HEX_SPI_H_
#define _HEX_SPI_H_

#include "types.h"

extern int SPI_open(unsigned int baudrate);
extern void SPI_request(char *txbuf, char *rxbuf, int len, void (*trxend)(void));

#endif // ifndef _SPI_H_
