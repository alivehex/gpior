#include "gpior.h"

#undef TRACE_LEVEL
#undef TRACE_TITLE
#undef TRACE_TITLE
#define TRACE_LEVEL	TRACE_LEVEL_DEBUG
#define TRACE_NAME  TRACE_NAME_DISABLE
#define TRACE_TITLE "RF EXAMPLE"
#include "trace.h"


/*
 * @ RF RXD example
 */
int RF_example(void) {
	uint8 buffer[32];
	uint8 pipe;
	int i;

	nrf_open();

	while(1) {
#if (!RF_INT_MODE)
		nrf_read(buffer, &pipe);
		TRACE_DEBUG("read RF [pipe%d]:", pipe);
#endif // if (!RF_INT_MODE)
		nrf_write(buffer, TRUE);
		for(i = 0; i < 32; i ++) {
		 	TRACE_DEBUG_WP("0x%x ", buffer[i]);
		}
		GPIOR_DELAYUS(2000000);
	}
}

