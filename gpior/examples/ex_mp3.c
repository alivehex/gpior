#include "gpior.h"
#include <stdlib.h>

#undef TRACE_LEVEL
#undef TRACE_TITLE
#undef TRACE_TITLE
#define TRACE_LEVEL	TRACE_LEVEL_DEBUG
#define TRACE_NAME  TRACE_NAME_ENABLE
#define TRACE_TITLE "MP3"
#include "trace.h"

/*
 * @ VS1003 MP3 example
 */
int MP3_example(const char *fpath) {
	FATFS fat;
	uint8 buffer[32];
	uint32 nbytes;
	FIL	fh;
	FRESULT res;
	
	TRACE_DEBUG("MP3 example");
	if(vs1003_open() != 0) {
		TRACE_ERROR("open VS1003 error");
		return -1;
	}

	//vs1003_sin_test(10000);
	f_mount(0, &fat);
	/* open MP3 file */
	res = f_open(&fh, fpath, FA_READ);
	if(res != FR_OK) {
		TRACE_ERROR("open file %s error", fpath);
		return -1;
	}
	/* 32 bytes one time */
	do {
		f_read(&fh, buffer, 32, &nbytes);
		if(nbytes)
			vs1003_write_data(buffer, 32);
	} while(nbytes);
	f_close(&fh);
	return 0;
}
