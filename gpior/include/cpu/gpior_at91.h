#ifndef _GPIOR_AT91_H_
#define _GPIOR_AT91_H_

#include "at91rm9200.h"
#include "lib_at91rm9200.h"
#include "types.h"

//*----------------------------------------------------------------
//*		Low-level IO functions for GPIOR drivers
//*----------------------------------------------------------------
/* 
 * @ config IO input 
 */
#define GPIOR_CFGIO_INPUT(PORT, PIN)	PORT->PIO_PER = PIN;PORT->PIO_ODR = PIN	

/* 
 * @ config IO output
 */
#define GPIOR_CFGIO_OUTPUT(PORT, PIN)	PORT->PIO_PER = PIN;PORT->PIO_OER = PIN

/* 
 * @ set IO high
 */
#define GPIOR_SETIO(PORT, PIN)  PORT->PIO_SODR = PIN

/* 
 * @ clear IO low 
 */
#define GPIOR_CLRIO(PORT, PIN)	PORT->PIO_CODR = PIN

/* 
 * @ get IO level
 * @ return ZERO or NOT 
 */
#define GPIOR_GETIO(PORT, PIN)	(PORT->PIO_PDSR & PIN)

/*
 * @ call this is easy to port
 */
#define GPIOR_PORT(PORT)	AT91C_BASE_PIO ## PORT

/*
 * @ call this is easy to port
 */
#define GPIOR_PIN(PIN)		(1u << (PIN))

/*
 * @ asm delay micro-seconds
 */
extern void GPIOR_DELAYUS(int);

#define GPIOR_DELAY1US() \
	__asm { \
	NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP\
	NOP NOP NOP NOP NOP NOP NOP NOP NOP NOP\
	}

#endif // ifndef _GPIOR_AT91_H_
