#ifndef _HEX_I2C_
#define _HEX_I2C_


#include "types.h"

extern int I2C_open(int ch, unsigned int baudrate, uint32 slave_addr, void (*I2C_CB)(uint32));
extern int I2C_setup_reception(int ch, int lenb);
extern int I2C_rxdclr(int ch);
extern int I2C_close(int ch);
extern void I2C_disable_rxint(int ch);
extern void I2C_enable_rxint(int ch);

#endif /* ifndef _HEX_I2C_ */
