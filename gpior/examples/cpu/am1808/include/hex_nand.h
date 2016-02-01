#ifndef _HEX_NAND_H_
#define _HEX_NAND_H_


#include "types.h"


/* NAND device parameters */ 
typedef struct _NAND_SIZE_INFO {
	uint32		device_id;
	uint32		data_bytes_per_page;
	uint32 		data_bytes_per_block;
	uint32 		data_bytes_per_device;
	uint32		spare_bytes_per_page;
	uint32		pages_per_block;
	uint32		blocks_per_device;			
} NAND_INFO_T;

extern NAND_INFO_T *NAND_open(void);
extern NAND_INFO_T *NAND_getinfo(void);
extern int NAND_read(NAND_INFO_T *ninfo, unsigned int page_addr, unsigned char *buffer);
extern int NAND_read_spare(NAND_INFO_T *ninfo, unsigned int page_addr, unsigned char *buffer, unsigned int len);
extern int NAND_write(NAND_INFO_T *ninfo, unsigned int page_addr, unsigned char *buffer);
extern int NAND_write_spare(NAND_INFO_T *ninfo, unsigned int page_addr, unsigned char *buffer, unsigned int len);
extern int NAND_erase(NAND_INFO_T *ninfo, unsigned int blkNum);

extern void EnableNANDHWECCHandle(void);
extern void DisableNANDHWECCHandle(void);
extern void EnableNANDBLECCMode(void);
extern void DisableNANDBLECCMode(void);

#endif

