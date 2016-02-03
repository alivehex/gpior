#include <board.h>
#include <stdio.h>
#include <dbgu/dbgu.h>
#include <pio/pio.h>
#include <pmc/pmc.h>
#include <irq/irq.h>
#include <systick/systick.h>
#include <string.h>

#include "cmd.h"
#include "gpior.h"
#include "ff.h"

static void tickTack(void);
void delay_ms(unsigned int ms);
void SysTick_Handler(void);
static void init(void);

int main(void) {
	init();
	
//	sd_open();
//	gpio_led_test();
//	gpio_input_test();
//	fatfs_test("test.txt");
//	while (1) {
//		tickTack();
//		delay_ms(1000);
//	}
	command_loop();
}

static void init(void) {
	const Pin pinsDbgu[] = {PINS_DBGU};
    
	PMC_EnablePeripheral(AT91C_ID_DBGU);
	PIO_Configure(pinsDbgu, PIO_LISTSIZE(pinsDbgu));
    DBGU_Configure(AT91C_US_PAR_NONE, 115200, BOARD_MCK);
	SysTick_Configure(1, BOARD_MCK / 1000, SysTick_Handler);
}

static void tickTack(void) {
	static int tickCount = 0;
	
	tickCount ++;
	debug("\r                \r");
	debug("%02d:%02d", tickCount / 60, tickCount % 60);	
}

static int fatfs_test(const char *txt_filepath) {
 	const char copy_fname[] = "copy.txt";
	FIL fh;
	FIL cfh;
	uint8 buffer[512];
	uint32 nbytes;
	uint32 wnbytes;
	FRESULT res;
	FATFS fat;
	
	printf("FATFS test");
	printf("read file %s, copy to %s, and show content\r\n", txt_filepath, copy_fname);
	
	f_mount(0, &fat);
	/* MMC test */
	memset(buffer, 0, sizeof(buffer));
	res = f_open(&fh, txt_filepath, FA_READ);
	if(res != FR_OK) {
		printf("open file %s error", txt_filepath);
		return -1;
	}
	res = f_open(&cfh, copy_fname,  FA_CREATE_ALWAYS | FA_WRITE); 
	if(res != FR_OK) {
		printf("open file %s error", copy_fname);
		f_close(&cfh);
		return -1;
	}
	do {
		f_read(&fh, buffer, sizeof(buffer) - 1, &nbytes);
		if(nbytes) {
		 	buffer[nbytes] = 0;
			//printf("%s", buffer);
			f_write(&cfh, buffer, nbytes, &wnbytes);
		}
	} while(nbytes);
	f_close(&fh);
	f_close(&cfh);
	printf("test ok\r\n");
	return 0;
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
