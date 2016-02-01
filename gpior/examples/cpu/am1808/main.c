// #include "hex_gpio.h"
// #include "spi.h"
// #include "sd.h"
// #include "mtd.h"
// #include "vfs.h"
// #include "crc16.h"
// #include "i2c.h"
#include "e2prom.h"

#include <stdlib.h>
#include <string.h>


// #undef TRACE_LEVEL
// #undef TRACE_TITLE
// #undef TRACE_TITLE
// #define TRACE_LEVEL	TRACE_LEVEL_DEBUG
// #define TRACE_NAME  TRACE_NAME_DISABLE
// #define TRACE_TITLE "MAIN"
// #include "trace.h"

// int cp(const char *src_path, const char *dst_path);

int main() {
// 	unsigned char buffer[1024];
// 	//unsigned int res;
// 	//unsigned int i, j;
// 	//unsigned long lcrc;
// 	//char buffer[512];
// 	//unsigned short crctable[512];
// 	//unsigned short crc;
// 	//mtd_dev_t mmc;
// 	//FILE *fh;
// 	const char HelloMOTO[] = "Hello MOTO!";
// 	
// 	/* init UART fot STDOUT */
// 	stdio_init();
// 	
// 	TRACE_DEBUG_WP("\r\n**************** AM1808 GPIOR ****************\r\n");
// 	
// 	GPIO_open();
// 	
// 	while(1) {
// 		/* open device with 2bytes sub-addres and 0xA0 for device address */
// 		E2PROM_open(2, 0xA0);
// 		
// 		/* disable the write protect for writing */
// 		E2PROM_wp_disable();
// 		
// 		/* write string start with address 0 */
// 		E2PROM_write(0x0, (unsigned char *)HelloMOTO, sizeof(HelloMOTO));
// 		
// 		/* enable the write protect */
// 		E2PROM_wp_enable();
// 		
// 		/* read back the string */		
// 		E2PROM_read(0x0, buffer, sizeof(HelloMOTO));
// 		
// 		/* close device */
// 		E2PROM_close();
// 	}

// #if (0)
// 	while (1) {
// 		I2C_open(I2C_HIGHSPEED_MODE);
// 		I2C_start();
// 		I2C_write_byte(0xAA);
// 		I2C_write_byte(0xFF);
// 		I2C_read_byte(FALSE);
// 		I2C_stop();
// 		I2C_close();
// 	}
// #endif
// 	
// #if (0)
// 	SPI_open(0, 10000000);
// 	
// 	while (1) {
// 		SPI_enable(0);
// 		SPI_write(0, 0xAA);
// 		SPI_write(0, 0xFF);
// 		SPI_write(0, 0x00);
// 		SPI_disable(0);
// 	}
// #endif

// #if (0)	
// 	while (1) {
// 		if (SD_open() == 0) {
// 			memset(buffer, 0, sizeof(buffer));
// 			
// 			SD_read_block(0, buffer);
// 			SD_read_block(1, buffer + 512);
// 			
// 			SD_read_many_blocks(0, buffer, 2);
// 			
// 			SD_write_block(384, buffer);
// 			SD_write_block(385, buffer + 512);
// 			
// 			SD_write_many_blocks(384, buffer, 2);
// 			
// 			//SD_write_block(1, buffer);
// 			SD_close();
// 		}
// 	}
// #endif

// #if (0)
// 	memset(&mmc, 0, sizeof(mmc));
// 	
// 	/* MMC */
// 	mmc.name = "mmc";
// 	mmc.fstype = FS_TYPE_FAT;
// 	mmc.MTD_read_block	= SD_read_many_blocks;
// 	mmc.MTD_write_block = SD_write_many_blocks;
// 	mmc.MTD_device_init = SD_open;
// 	mmc.MTD_device_deinit = SD_close;
// 	mtd_register_dev(&mmc);
// 	
// 	while (1) {
// 		mount("mmc", "/mmc", FALSE);

// 		cp("/mmc/bl.bin", "/mmc/gpior.bin");
// 		
// 		umount("/mmc");
// 		
// 		delay_ms(1000);
// 	}
// #endif
// 	
// 	/* CRC test */
// // 	memset(buffer, 0xFF, sizeof(buffer));
// // 	crc = crc16(0, buffer, 512);
// // 	TRACE_DEBUG("crc=0x%x", crc);

// // 	memset(buffer, 0xFF, sizeof(buffer));
// // 	crc = crc16(0, buffer, 512);
// // 	TRACE_DEBUG("crc=0x%x", crc);
// 	while (1);
}

/*
** @ copy file
**/
// int cp(const char *src_path, const char *dst_path) {
// 	FILE *fsrc;
// 	FILE *fdst;
// 	char *fbuf;
// 	uint32 count;
// 	
// 	
// 	/* open source file */
// 	fsrc = fopen(src_path, "r");
// 	if(fsrc == NULL) {
// 		TRACE_ERROR("cp [open %s failed]", src_path);
// 		return -1;
// 	}
// 	
// 	/* open copy file */
// 	fdst = fopen(dst_path, "w");
// 	if(fdst == NULL) {
// 		TRACE_ERROR("cp [open %s failed]", dst_path);
// 		fclose(fsrc);
// 		return -1;
// 	}
// 	
// 	TRACE_DEBUG("cp %s to %s [%dKB]... ", src_path, dst_path, fsrc->fsize / 1024);
// 	
// 	if(fsrc->fsize) {
// 		/* malloc the room */
// 		fbuf = malloc(fsrc->fsize);
// 		if(fbuf == NULL) {
// 			TRACE_ERROR_WP("[internal error]");
// 			fclose(fsrc);
// 			fclose(fdst);
// 			return -1;
// 		}
// 		
// 		/* read the file */
// 		count = fread(fbuf, 1, fsrc->fsize, fsrc); 
// 		if (count != fsrc->fsize) {
// 			TRACE_ERROR_WP("[read failed %dKB]", count / 1024);
// 			fclose(fsrc);
// 			fclose(fdst);
// 			free(fbuf);
// 			return -1;
// 		}
// 		
// 		/* write to file */
// 		//TRACE_DEBUG("write %s", dst_path);
// 		count = fwrite(fbuf, 1, fsrc->fsize, fdst);
// 		if(count != fsrc->fsize) {
// 			TRACE_ERROR_WP("[write failed %dKB]", count / 1024);
// 			fclose(fsrc);
// 			fclose(fdst);
// 			free(fbuf);
// 			return -1;
// 		}
// 		free(fbuf);
// 	}
// 	/* close file */
// 	fclose(fsrc);
// 	fclose(fdst);
// 	TRACE_DEBUG_WP("[ok]");

// 	return 0;	
// }

