#ifndef _HEX_PWMC_H_
#define _HEX_PWMC_H_


#include "types.h"

#define PWMC_KEYCH		0
#define PWMC_LCDCH		1

extern int PWMC_open(int ch, uint32 freq_hz, uint32 levels, uint32 duty);
extern uint32 PWMC_getlevel(int ch);
extern int PWMC_reset_duty(int channel, int duty);
extern int PWMC_close(int channel);


#endif // ifdef _PWMC_H_
