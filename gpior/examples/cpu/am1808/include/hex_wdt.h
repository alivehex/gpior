#ifndef _HEX_WDT_H_
#define _HEX_WDT_H_


#include "types.h"

/* FIXME:
** This driver will use the TIMER1 as WATCHDOG
*/
extern int TMRWDT_set(uint32 sec);
extern int TMRWDT_reset(void);


#endif /* ifndef _HEX_WDT_H_ */
