#ifndef _SDSPI_H_
#define _SDSPI_H_

int sd_open(void);
void sd_close(void);
int sd_read_block(unsigned int sector, unsigned char * buffer);
int sd_write_block(unsigned int sector, unsigned char * buffer);
int sd_read_multiple_blocks(int blockIndex, char *buffer, int numOfBlocks);
int sd_write_multiple_blocks(int blockIndex, char *buffer, int numOfBlocks);

int MTD_read_block(int block_num,  char *buffer, int blocks);
int MTD_write_block(int block_num, char *buffer, int blocks);
int MTD_device_init(void);
int MTD_device_deinit(void);

#endif // ifndef _SPISD_H_
