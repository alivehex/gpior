#include "gpior.h"

#undef TRACE_LEVEL
#undef TRACE_TITLE
#undef TRACE_TITLE
#define TRACE_LEVEL	TRACE_LEVEL_DEBUG
#define TRACE_NAME  TRACE_NAME_DISABLE
#define TRACE_TITLE "EEPROM"
#include "trace.h"

/*
 * @ EEPROM example, for AT24C256
 */
int EEPROM_example(void) {
	uint8 buffer[] = "WE COME FOR PEACE!";
	int err;

	if(e2prom_open(2, 0xA0) != 0) {
		TRACE_DEBUG("open E2PROM error!");
		return -1;
	}
	/* write E2PROM */
	TRACE_DEBUG("write to E2PROM [%s]", buffer);
	e2prom_wp_disable()
	err = e2prom_write(0, buffer, sizeof(buffer)); 
 	e2prom_wp_enable();
	if(err != 0) {
		TRACE_ERROR("write E2PROM error");
		return -1;
	}
	/* delay 20ms */
	GPIOR_DELAYUS(20000);
	/* read back to check */
	if(e2prom_read(0, buffer, sizeof(buffer)) != 0) {
	 	TRACE_ERROR("read E2PROM error");
		return -1;
	}
	TRACE_DEBUG("read from E2PROM [%s]", buffer);
	return 0;
}
