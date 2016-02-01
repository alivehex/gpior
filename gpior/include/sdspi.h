#ifndef _SDSPI_H_
#define _SDSPI_H_

#include "types.h"

/*
 * @ SD-card init into SPI mode
 */
int sd_open(int ch);

/*
 * @ sd card close
 */
int sd_close(int ch);

/*
 * @ read single block by block address
 */
int sd_read_block(int ch, uint32 sector, uint8 * buffer); 

/*
 * @ write single block by block address
 */
int sd_write_block(int ch, uint32 sector, uint8 * buffer);


#endif // ifndef _SPISD_H_
