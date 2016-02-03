#ifndef _SD_H_
#define _SD_H_


extern int SD_open(void);
extern int SD_read_block(int block_addr, unsigned char *buffer);
extern int SD_read_many_blocks(int block_addr, unsigned char *buffer, int nblock);
extern int SD_write_block(int block_addr, unsigned char *buffer);
extern int SD_write_many_blocks(int block_addr, unsigned char *buffer, int nblock);
extern void SD_close(void);


#endif /* ifndef _SD_H_ */
