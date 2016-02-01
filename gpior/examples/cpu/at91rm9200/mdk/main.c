#include <stdio.h>
#include "iocfg.h"


#undef TRACE_LEVEL
#undef TRACE_TITLE
#undef TRACE_TITLE
#define TRACE_LEVEL	TRACE_LEVEL_DEBUG
#define TRACE_NAME  TRACE_NAME_DISABLE
#define TRACE_TITLE "MAIN"
#include "trace.h"


extern int FATFS_example(const char *txt_filepath);
extern int LCD_example(void);
extern int RFTX_example(void);
extern int MP3_example(const char *fpath);
extern int RF_example(void);
extern int EEPROM_example(void);
extern int DS18B20_example(void);
extern int PWM_example(int freq_hz, int duty);
extern int I2S_example(const char *fpath);


int main() {	
	TRACE_DEBUG_WP("---------------- GPIOR AT91RM9200 ----------------");
	
	GPIO_test();
	//LCD_example();
	//FATFS_example("sample.txt");
	//MP3_example("sample.mp3");
	//RF_example();
	//EEPROM_example();
 	//DS18B20_example();
	//PWM_example(10000, 10);
	//I2S_example("sample.wav");
	while(1);
}
