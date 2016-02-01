#include "gpior.h"
#include "stdlib.h"

#if (!GPIOR_I2S)
#error I2S DRIVER IS MISSING!
#elif (!GPIOR_FATFS)
#error FATFS DRIVER IS MISSING
#else

#undef TRACE_LEVEL
#undef TRACE_TITLE
#undef TRACE_TITLE
#define TRACE_LEVEL	TRACE_LEVEL_DEBUG
#define TRACE_NAME  TRACE_NAME_DISABLE
#define TRACE_TITLE "WAV"
#include "trace.h"

/*
 * @ I2S example
 */
int I2S_example(const char *fpath) {
#if (1)
	uint16 *buffer;
	uint32 nbytes;
	FRESULT res;
	FATFS fat;
	FIL fwav;

	f_mount(0, &fat);
	res = f_open(&fwav, fpath, FA_READ);
	if(res != FR_OK) {
		TRACE_ERROR("open file %s error", fpath);
		return -1;
	}
	buffer = malloc(fwav.fsize);
	if(buffer == NULL) {
	 	TRACE_ERROR("call to internal error");
		f_close(&fwav);
		return -1;
	}
	f_read(&fwav, buffer, fwav.fsize, &nbytes);
	i2s_open(16, 16000, I2S_CH_STEREO);
	i2s_write(buffer, nbytes);
	f_close(&fwav);
	free(buffer);
#else

	short buffer[4];
	
	buffer[0] = 32767;
	buffer[1] = -32768;
	buffer[2] = -32768;
	buffer[3] = 32767;
	i2s_open();
	while(1) {
		i2s_write((uint16 *)buffer, 8);
	}
#endif
	return 0;
}

#endif 
