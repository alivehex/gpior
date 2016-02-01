#ifndef _HEX_SPI2_H_
#define _HEX_SPI2_H_

extern int SPI2_open(unsigned int baudrate);
extern int SPI2_read(char *buffer, int len);
extern int SPI2_write(char *buffer, int len);
extern int SPI2_rw(char *wbuf, char *rbuf, int len); 
extern int SPI2_close(void);

#endif
