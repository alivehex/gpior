#ifndef _GPIOR_STM32_H_
#define _GPIOR_STM32_H_

#include "types.h"
#include "stm32f10x.h"


//*----------------------------------------------------------------
//*		Low-level IO functions for GPIOR drivers
//*----------------------------------------------------------------
/* 
 * @ config IO input 
 */
static __inline GPIOR_CFGIO_INPUT(GPIO_TypeDef * PORT, unsigned short PIN) {
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PORT, &GPIO_InitStructure);
}	

/* 
 * @ config IO output
 */
static __inline GPIOR_CFGIO_OUTPUT(GPIO_TypeDef * PORT, unsigned short PIN) {
	GPIO_InitTypeDef GPIO_InitStructure;
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = PIN;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(PORT, &GPIO_InitStructure);
}	

/* 
 * @ set IO high
 */
#define GPIOR_SETIO(PORT, PIN)  PORT->BSRR = PIN //GPIO_WriteBit(PORT, PIN, Bit_SET)

/* 
 * @ clear IO low 
 */
#define GPIOR_CLRIO(PORT, PIN)	PORT->BRR = PIN //GPIO_WriteBit(PORT, PIN, Bit_RESET)

/* 
 * @ get IO level
 * @ return ZERO or NOT 
 */
#define GPIOR_GETIO(PORT, PIN)	((PORT->IDR & PIN) ? 1 : 0)
//((GPIO_ReadInputDataBit(PORT, PIN) == SET) ? 1 : 0)

/*
 * @ call this is easy to port
 */
#define GPIOR_PORT(PORT)		GPIO ## PORT

/*
 * @ call this is easy to port
 */
#define GPIOR_PIN(PIN)			GPIO_Pin_ ## PIN

/*
 * @ asm delay micro-seconds
 */
extern void GPIOR_DELAYUS(volatile int);

#define GPIOR_DELAY1US() \
	__asm { \
	NOP NOP NOP NOP \
	}

#endif // ifndef _GPIOR_STM32_H_
