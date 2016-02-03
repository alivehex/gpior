#ifndef __VS1003_H__
#define __VS1003_H__

extern int VS1003_open(void);
extern void VS1003_write_data(char *buf, int len);
extern unsigned short VS1003_ram_test(unsigned int time); 
extern void VS1003_sin_test(unsigned int time);
extern unsigned short VS1003_read_reg(uint16 address);
extern void VS1003_write_reg(uint16 address, uint16 value);

#endif
