#ifndef _GPIOR_AM1808_H_
#define _GPIOR_AM1808_H_

#include "types.h"

//*----------------------------------------------------------------
//*		Low-level IO functions for GPIOR drivers
//*----------------------------------------------------------------	

/* 
 * @ set IO high
 */
#define GPIOR_SETIO(PORT, PIN)  

/* 
 * @ clear IO low 
 */
#define GPIOR_CLRIO(PORT, PIN)	

/* 
 * @ get IO level
 * @ return ZERO or NOT 
 */
#define GPIOR_GETIO(PORT, PIN)	
//((GPIO_ReadInputDataBit(PORT, PIN) == SET) ? 1 : 0)

/*
 * @ call this is easy to port
 */
#define GPIOR_PORT(PORT)		

/*
 * @ call this is easy to port
 */
#define GPIOR_PIN(PIN)			

/*
 * @ asm delay micro-seconds
 */
extern void GPIOR_DELAYUS(volatile int);

#define GPIOR_DELAY1US() \
	__asm { \
	NOP NOP NOP NOP \
	}

#endif // ifndef _GPIOR_AM1808_H_
