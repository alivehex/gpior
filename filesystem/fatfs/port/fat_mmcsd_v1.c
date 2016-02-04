/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "types.h"
#include "diskio.h"
#include "hex_mmc.h"
#include "mtd.h"
#include "ffconf.h"


/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
#define ATA		2
#define USB		1
#define MMC		0


static mtd_dev_t *FSDEV[_DRIVES];

int ff_save_mtddev(int index, mtd_dev_t *dev) {
	if (index >= _DRIVES)
		return -1;
	FSDEV[index] = dev;
	return 0;
}

/* USB functions */
DSTATUS USB_disk_initialize(void) {
	return RES_PARERR;
}

DSTATUS USB_disk_status(void) {
	return RES_PARERR;
}

DSTATUS USB_disk_read(BYTE * buff, int sector, int count) {
	return RES_PARERR;
}

DSTATUS USB_disk_write(const BYTE * buff, int sector, int count) {
	return RES_PARERR;
}

DSTATUS USB_disk_ioctl(WORD ctrl, BYTE * buff) {
	return RES_PARERR;
}

/* MMC functions */
DSTATUS MMC_disk_initialize(void) {
	if(SDMMC_open() == 0) {
		return RES_OK;
	} 
	else {
	 	return RES_ERROR;
	}
}

DSTATUS MMC_disk_status(void) {
	return RES_OK;
}

DRESULT MMC_disk_read(BYTE * buff, int sector, int count) {
	SDMMC_read(sector, (unsigned char *)buff, count);
	return RES_OK;
}

DRESULT MMC_disk_write(const BYTE * buff, int sector, int count) {
	SDMMC_write(sector, (unsigned char *)buff, count);
	return RES_OK;
}

DRESULT MMC_disk_ioctl(WORD ctrl, void * buff) {
	DRESULT res = RES_OK;	
#if (0)
	switch(ctrl) {
	 	case CTRL_SYNC:			
		break;
		case GET_SECTOR_SIZE:
			*(unsigned int *)buff = mmc->info.csd.capacity;
		break;
		case GET_SECTOR_COUNT:
			*(unsigned int *)buff = mmc->info.csd.capacity /
				mmc->status.cur_block_len;
		break;
		case GET_BLOCK_SIZE:
			*(unsigned int *)buff = mmc->status.cur_block_len;
		break; 
	}
#endif
	return res;
}


/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */

DSTATUS disk_initialize (
	BYTE drv				/* Physical drive nmuber (0..) */
)	{
	mtd_dev_t *dev = FSDEV[drv];
// 	switch (drv) {
// 	case ATA :
// 		return RES_PARERR;
// 	case MMC :		
// 		return MMC_disk_initialize();
// 	case USB :
// 		return RES_PARERR;
// 	}
// 	return RES_PARERR;
	if (dev->MTD_device_init() == 0) {
		return RES_OK;
	}
	else {
		return RES_PARERR;
	}
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
) {
// 	switch (drv) {
// 	case ATA :
// 		return RES_PARERR;
// 	case MMC :
// 		return MMC_disk_status();
// 	case USB :
// 		return RES_PARERR;
// 	}
// 	return RES_PARERR;
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
) {	
// 	switch (drv) {
// 	case ATA :
// 		return RES_PARERR;
// 	case MMC :
// 		return MMC_disk_read(buff, (int)sector, (int)count);
// 	case USB :
// 		return RES_PARERR;
// 	}
// 	return RES_PARERR;
	mtd_dev_t *dev = FSDEV[drv];
	
	if (dev->MTD_read_block(sector, (char *)buff, count) == 0) {
		return RES_OK;
	} 
	else {
		return RES_PARERR;
	}
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */

#if _READONLY == 0
DRESULT disk_write (
	BYTE drv,			/* Physical drive nmuber (0..) */
	const BYTE *buff,	/* Data to be written */
	DWORD sector,		/* Sector address (LBA) */
	BYTE count			/* Number of sectors to write (1..255) */
)	{
// 	switch (drv) {
// 	case ATA :
// 		return RES_PARERR;
// 	case MMC :
// 		return MMC_disk_write(buff, sector, count);
// 	case USB :
// 		return RES_PARERR;
// 	}
// 	return RES_PARERR;
	mtd_dev_t *dev = FSDEV[drv];
	
	if (dev->readonly == TRUE) {
		return RES_PARERR;
	}
	if (dev->MTD_write_block(sector, (char *)buff, count) == 0) {
		return RES_OK;
	} 
	else {
		return RES_PARERR;
	}
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)	{
// 	switch (drv) {
// 	case ATA :
// 		return RES_PARERR;
// 	case MMC :
// 		return MMC_disk_ioctl(ctrl, buff);
// 	case USB :
// 		return RES_PARERR;
// 	}
// 	return RES_PARERR;
	return RES_OK;
}

