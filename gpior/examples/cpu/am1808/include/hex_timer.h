#ifndef _HEX_TIMER_H_
#define _HEX_TIMER_H_

#include "types.h"

// timer mode 
typedef enum _TIMER_MODES {
	ONE_TIME_OPT,		// one time operation, NO interrupt used
	CONT_OPT_RELOAD,	// continues operation with period reload
} TIMER_MODES_T;


/*
** TIMER1 - WDT, TIMER0 - DELAY, TIMER2 - RESERVED, TIMER3 - RESERVED
**/

extern int TIMER_open(int ch, uint64 nS, void (*timer_callback)(void));
extern int TIMER_set_counter(int ch, unsigned int low32, unsigned int high32);
extern int TIMER_get_counter(int ch, unsigned int *low32, unsigned int *high32);
extern int TIMER_enable(int ch);
extern int TIMER_disable(int ch);
extern int TIMER_close(int ch);

#endif
