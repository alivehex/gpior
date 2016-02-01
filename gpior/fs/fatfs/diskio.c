/*-----------------------------------------------------------------------*/
/* Low level disk I/O module skeleton for FatFs     (C)ChaN, 2007        */
/*-----------------------------------------------------------------------*/
/* This is a stub disk I/O module that acts as front end of the existing */
/* disk I/O modules and attach it to FatFs module with common interface. */
/*-----------------------------------------------------------------------*/

#include "diskio.h"
#include "gpior.h"

#if (GPIOR_FATFS)

#if	(GPIOR_SD)
#include "sd.h"
#elif (GPIOR_SDSPI)
#include "sdspi.h"
#else
#error SD-CARD WITH NO SPI OR SD DRIVER! 
#endif

/*-----------------------------------------------------------------------*/
/* Correspondence between physical drive number and physical drive.      */
#define ATA		2
#define USB		1
#define MMC		0


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
	if(sd_open(GPIOR_SD_CH) == 0) {
		return RES_OK;
	} else {
	 	return RES_ERROR;
	}
}
DSTATUS MMC_disk_status(void) {
	return RES_OK;
}
DRESULT MMC_disk_read(BYTE * buff, int sector, int count) {
	while(count --) {
	 	if(sd_read_block(GPIOR_SD_CH, sector, buff) != 0) {
		 	return RES_ERROR;
		}
		buff += 512;
		sector ++;
	}
	return RES_OK;
}
DRESULT MMC_disk_write(const BYTE * buff, int sector, int count) {
	while(count --) {
	 	if(sd_write_block(GPIOR_SD_CH, sector, (uint8 *)buff) != 0) {
		 	return RES_ERROR;
		}
		buff += 512;
		sector ++;
	}
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
	
	switch (drv) {
	case ATA :
		return RES_PARERR;
	case MMC :		
		return MMC_disk_initialize();
	case USB :
		return RES_PARERR;
	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Return Disk Status                                                    */

DSTATUS disk_status (
	BYTE drv		/* Physical drive nmuber (0..) */
) {
	switch (drv) {
	case ATA :
		return RES_PARERR;
	case MMC :
		return MMC_disk_status();
	case USB :
		return RES_PARERR;
	}
	return RES_PARERR;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */

DRESULT disk_read (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE *buff,		/* Data buffer to store read data */
	DWORD sector,	/* Sector address (LBA) */
	BYTE count		/* Number of sectors to read (1..255) */
) {	
	switch (drv) {
	case ATA :
		return RES_PARERR;
	case MMC :
		return MMC_disk_read(buff, (int)sector, (int)count);
	case USB :
		return RES_PARERR;
	}
	return RES_PARERR;
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
	switch (drv) {
	case ATA :
		return RES_PARERR;
	case MMC :
		return MMC_disk_write(buff, sector, count);
	case USB :
		return RES_PARERR;
	}
	return RES_PARERR;
}
#endif /* _READONLY */



/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */

DRESULT disk_ioctl (
	BYTE drv,		/* Physical drive nmuber (0..) */
	BYTE ctrl,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)	{
	switch (drv) {
	case ATA :
		return RES_PARERR;
	case MMC :
		return MMC_disk_ioctl(ctrl, buff);
	case USB :
		return RES_PARERR;
	}
	return RES_PARERR;
}

#endif // if (GPIOR_FATFS)

