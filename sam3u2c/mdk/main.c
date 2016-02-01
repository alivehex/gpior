#include <board.h>
#include <stdio.h>
#include <utility/trace.h>
#include <pio/pio.h>
#include <pmc/pmc.h>
#include <irq/irq.h>
#include <systick/systick.h>

static void tickTack(void);
void delay_ms(unsigned int ms);
void SysTick_Handler(void);

int main(void) {
	const Pin ledPin = { (1u << 26), AT91C_BASE_PIOA, AT91C_ID_PIOA, PIO_OUTPUT_1, PIO_PULLUP };
	const Pin pinsDbgu[] = {PINS_DBGU};
    
	PMC_EnablePeripheral(AT91C_ID_PIOA);
	PMC_EnablePeripheral(AT91C_ID_DBGU);
	
	PIO_Configure(pinsDbgu, PIO_LISTSIZE(pinsDbgu));
	PIO_Configure(&ledPin, 1);
	
    DBGU_Configure(AT91C_US_PAR_NONE, 115200, BOARD_MCK);
	SysTick_Configure(1, BOARD_MCK / 1000, SysTick_Handler);	
	
	while (1) {
		//DBGU_PutChar('C');
		tickTack();
		PIO_Set(&ledPin);
		delay_ms(500);
		PIO_Clear(&ledPin);
		delay_ms(500);
	}
}

static void tickTack(void) {
	static int tickCount = 0;
	
	tickCount ++;
	printf("\r                \r");
	printf("%02d:%02d", tickCount / 60, tickCount % 60);	
}



//------------------------------------------------------------------------------
/// Handler for SysTick interrupt. Increments the timestamp counter.
//------------------------------------------------------------------------------
volatile unsigned int timestamp = 0;
void SysTick_Handler(void) {
    timestamp++;
}

void delay_ms(unsigned int ms) {
    unsigned int st = timestamp;

    while (timestamp - st < ms);
}
