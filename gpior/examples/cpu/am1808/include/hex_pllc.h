#ifndef _HEX_PLLC_H_
#define _HEX_PLLC_H_


#include "types.h"


#define OSCIN_FREQ	24000000


typedef enum _CLKTYPE {
	PLL0 = 0,
	PLL0_SYSCLK1,
	PLL0_SYSCLK2,
	PLL0_SYSCLK3,
	PLL0_SYSCLK4,
	PLL0_SYSCLK5,
	PLL0_SYSCLK6,
	PLL0_SYSCLK7,
	
	PLL1,
	PLL1_SYSCLK1,
	PLL1_SYSCLK2,
	PLL1_SYSCLK3,
} PLLC_CLK_T;


/*
 * Init PLLC from POWER-ON state 
 */
extern void PLLC_init(void);

/*
 * @ return system clock freq
 * @ t: PLLC_CLK_T
 */
extern uint32 PLLC_getfreq(PLLC_CLK_T t);


#endif /* ifndef _HEX_PLLC_H_ */
