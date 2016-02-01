#include "gpior.h"


#undef TRACE_LEVEL
#undef TRACE_TITLE
#undef TRACE_TITLE
#define TRACE_LEVEL	TRACE_LEVEL_DEBUG
#define TRACE_NAME  TRACE_NAME_ENABLE
#define TRACE_TITLE "FSEX"
#include "trace.h"


/*
 * @ FATFS example
 * @ read txx file, copy it, and output on UART-STD
 */
int FATFS_example(const char *txt_filepath) {
 	const char copy_fname[] = "copy.txt";
	FIL fh;
	FIL cfh;
	uint8 buffer[512];
	uint32 nbytes;
	uint32 wnbytes;
	FRESULT res;
	FATFS fat;
	
	TRACE_DEBUG("FATFS example");
	TRACE_DEBUG("read file %s, copy to %s, and show content\r\n", txt_filepath, copy_fname);
	f_mount(0, &fat);
	/* MMC test */
	memset(buffer, 0, sizeof(buffer));
	res = f_open(&fh, txt_filepath, FA_READ);
	if(res != FR_OK) {
		TRACE_ERROR("open file %s error", txt_filepath);
		return -1;
	}
	res = f_open(&cfh, copy_fname,  FA_CREATE_ALWAYS | FA_WRITE); 
	if(res != FR_OK) {
		TRACE_ERROR("open file %s error", copy_fname);
		f_close(&cfh);
		return -1;
	}
	do {
		f_read(&fh, buffer, sizeof(buffer) - 1, &nbytes);
		if(nbytes) {
		 	buffer[nbytes] = 0;
			TRACE_DEBUG_WP("%s", buffer);
			f_write(&cfh, buffer, nbytes, &wnbytes);
		}
	} while(nbytes);
	f_close(&fh);
	f_close(&cfh);
	return 0;
}
