#include "gpior.h"

#undef TRACE_LEVEL
#undef TRACE_TITLE
#undef TRACE_TITLE
#define TRACE_LEVEL	TRACE_LEVEL_DEBUG
#define TRACE_NAME  TRACE_NAME_DISABLE
#define TRACE_TITLE "DS18B20"
#include "trace.h"

/*
 * @ DS18B20 example
 */
int DS18B20_example(void) {
	DS18B20_REG_T ds18b20;
	int temp_hex;

	if(ds_init(&ds18b20) != 0) {
		TRACE_ERROR("DS18B20 error");
		return -1;
	}
	if(ds_start_con(&ds18b20) != 0) {
	 	TRACE_ERROR("start convert error");
		return -1;
	}
	GPIOR_DELAYUS(ds18b20.con_time * 1000);
	temp_hex = ds_read_temp(&ds18b20);
	if(temp_hex == -1) {
	 	TRACE_ERROR("read temp error");
		return -1;
	}
	TRACE_DEBUG("temp: %d", temp_hex / 16); 
 	return 0;
}
