#ifndef _HEX_MMC_H_
#define _HEX_MMC_H_


#include "types.h"


extern void *SDMMC_open(void);
extern int SDMMC_read(int blockNum, unsigned char *buffer, int numBlocks);
extern int SDMMC_write(int blockNum, unsigned char *buffer, int numBlocks);
extern int SDMMC_close(void *handle);


#endif /* ifndef _HEX_MMC_H_ */
