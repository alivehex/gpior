#ifndef _SPI_H_
#define _SPI_H_

extern int SPI_open(int ch, unsigned int clock_freq_hz);
extern void SPI_enable(int ch);
extern void SPI_disable(int ch);
extern unsigned char SPI_write(int ch, unsigned char byte) ;
#define SPI_read(ch)	SPI_write(ch, 0xFF)
extern int SPI_close(int ch);

#endif // ifndef _SPI_H_
