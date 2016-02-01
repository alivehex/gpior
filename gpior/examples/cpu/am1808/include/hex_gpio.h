#ifndef _HEX_GPIO_H_
#define _HEX_GPIO_H_

#include "types.h"

/* GPIO_INPUT or GPIO_OUTPUT */
typedef enum _GPIO_TYPE {
	GPIO_INPUT,
	GPIO_OUTPUT,
} AM18_GPIO_TYPE;

/* GPIO interrupt type */
typedef enum {
 	INT_TYPE_FALLEDGE = 1,
	INT_TYPE_RISEDGE = 2,
	INT_TYPE_BOTHEDGE = 3,
} GPIO_INT_TYPE;


extern int 	GPIO_open(void);
extern void GPIO_config_direction(int port, int pin, AM18_GPIO_TYPE t);
extern void GPIO_config_dat(int port, int pin, int value);
extern int 	GPIO_read(int port, int pin);
extern void GPIO_int_enable(int port, int pin, GPIO_INT_TYPE int_type, void (*CB)(void));
extern void GPIO_int_disable(int port, int pin);
extern void GPIO_int_suspend(int port, int pin);
extern void GPIO_int_resume(int port, int pin); 

#define GPIO_CFG_OUTPUT(PORT, PIN)	GPIO_config_direction(PORT, PIN, GPIO_OUTPUT)
#define GPIO_CFG_INPUT(PORT, PIN)	GPIO_config_direction(PORT, PIN, GPIO_INPUT)	
#define GPIO_SET(PORT, PIN)	 		GPIO_config_dat(PORT, PIN, 1)
#define GPIO_CLEAR(PORT, PIN)		GPIO_config_dat(PORT, PIN, 0)
#define GPIO_GET(PORT, PIN)			GPIO_read(PORT, PIN)


#endif // ifndef _HEX_GPIO_H_
