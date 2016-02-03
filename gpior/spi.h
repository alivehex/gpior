#ifndef _SPI_H_
#define _SPI_H_

#include <stdint.h>
#include <stdio.h>

#include "types.h"

typedef enum {
	SPI_SS_0 = 0,
	SPI_SS_1,
	SPI_SS_2,
	SPI_SS_3,
} SpiSlaveSelect;

void spi_open(SpiSlaveSelect ch, uint32_t clock_freq_hz);
void spi_close(SpiSlaveSelect ch);
void spi_ss_low(SpiSlaveSelect ch);
void spi_ss_high(SpiSlaveSelect ch);
uint8_t spi_write_byte(SpiSlaveSelect SpiSlaveSelect, uint8_t byte);
void spi_write(SpiSlaveSelect ch, uint8_t *output, uint8_t* input, int length);

#define spi_read_byte(ch)	spi_write_byte(ch, 0xff)

#endif // ifndef _SPI_H_
