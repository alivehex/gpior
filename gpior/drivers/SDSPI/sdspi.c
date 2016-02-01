#include "crc7.h"
#include "crc16.h"
#include "iocfg.h"
#include <string.h>


#undef TRACE_LEVEL
#undef TRACE_TITLE
#undef TRACE_TITLE
#define TRACE_LEVEL	TRACE_LEVEL_NO_TRACE
#define TRACE_NAME  TRACE_NAME_DISABLE
#define TRACE_TITLE "SDSPI"
#include "trace.h"


#if (GPIOR_SDSPI)
#if (GPIOR_SD)
#error MUTIPLE SD-CARD DRIVERS: SD.C AND SDSPI.C
#endif

#if (!GPIOR_SPI)
#error SD-SPI NEEDS SPI DRIVER!
#endif

/* SD card command */
#define GO_IDLE_STATE			(0x00 | 0x40)
#define SEND_CSD				(0x09 | 0x40)
#define SEND_CID				(0x0a | 0x40)
#define STOP_TRANSMISSION		(0x0c | 0x40)
#define READ_SINGLE_BLOCK   	(0x11 | 0x40)
#define READ_MULTIPLE_BLOCK 	(0x12 | 0x40)
#define WRITE_SINGLE_BLOCK		(0x18 | 0x40)
#define WRITE_MULTIPLE_BLOCK 	(0x19 | 0x40)
#define APP_CMD	   				(0x37 | 0x40)
#define CRC_ON_OFF   			(0x3b | 0x40)
#define SEND_OP_COND   			(0x29 | 0x40)

/* timeout when enter SD card */
#define SD_SPI_MODE_TIMEOUT	(uint16)5000
/* read timeout */
#define SD_READ_TIMEOUT		(uint16)10000
/* write timeout */
#define SD_WRITE_TIMEOUT	(uint16)30000

static void sd_check_r1(unsigned char r1);

/*
 * @ write SD-SPI command
 */
static uint8 sd_write_cmd(int ch, uint8 * cmd) {
	uint8 i;
	uint8 buffer;
   
	TRACE_DEBUG("cmd[%d]", cmd[0] & ~0x40);
	SPI_enable(ch);
	/* pre-writting clocks */
	SPI_write(ch, 0xff);
	for(i = 0; i < 5; i++){ 
		SPI_write(ch, *(cmd+i));
	}
	SPI_write(ch, crc7(0, cmd, 5));
	/* pre-writting clocks */
	SPI_write(ch, 0xff);
	buffer = SPI_read(ch);
	SPI_disable(ch);
	sd_check_r1(buffer);
	return buffer;
}

/*
** @ check the r1 command response
**/
static void sd_check_r1(unsigned char r1) {
	TRACE_DEBUG_WP("res[0x%x]", r1);
	if(r1 & 0x01) {
		TRACE_DEBUG_WP("[in idle]");
	}
	if(r1 & 0x02) {
		TRACE_DEBUG_WP("[erase reset]");
	}
	if(r1 & 0x04) {
		TRACE_DEBUG_WP("[illegal cmd]");
	}
	if(r1 & 0x08) {
		TRACE_DEBUG_WP("[crc err]");
	}
	if(r1 & 0x10) {
		TRACE_DEBUG_WP("[erase seq err]");
	}
	if(r1 & 0x20) {
		TRACE_DEBUG_WP("[address err]");
	}
	if(r1 & 0x40) {
		TRACE_DEBUG_WP("[para err]");
	}
}

/*
 * @ SD-card init into SPI mode
 */
int SD_open(void) {
    uint8 stat, i;
	uint8 sd_cmd[5];
	uint16 tryLimit = 0;
	
	/* slower clock frequency at beginning */
	SPI_open(GPIOR_SD_CHANNEL, 200000);
	memset(sd_cmd, 0 ,sizeof(sd_cmd));
	sd_cmd[0] = GO_IDLE_STATE;
	
	/* atleast 74 clocks */
	SPI_enable(GPIOR_SD_CHANNEL);
	for(i = 0; i < 128; i++) {
   		SPI_write(GPIOR_SD_CHANNEL, 0xff);
   	}
	SPI_disable(GPIOR_SD_CHANNEL);
    
	tryLimit = 5;
	do {
		/* GO_IDLE_STATE, should return 0x01 */
		stat = sd_write_cmd(GPIOR_SD_CHANNEL, sd_cmd);
		   
		if(stat == 0x01) {
			break;
		}
	} while (-- tryLimit);
	if(tryLimit == 0) {
		TRACE_DEBUG("call to GO_IDLE_STATE");
		return -1;
	}
	
	/* try to enter sd SPI mode */ 
    do{
		/* APPCMD */   
	    sd_cmd[0] = APP_CMD;
	    stat = sd_write_cmd(GPIOR_SD_CHANNEL, sd_cmd);
		/* SEND_OP_COND */
	    sd_cmd[0] = SEND_OP_COND;
	    stat = sd_write_cmd(GPIOR_SD_CHANNEL, sd_cmd);   
	    tryLimit ++;
    } while((stat != 0x00) && (tryLimit < SD_SPI_MODE_TIMEOUT)); 
	
	if(tryLimit < SD_SPI_MODE_TIMEOUT){
		/* faster clock */
		SPI_open(GPIOR_SD_CHANNEL, 10000000);
		//dbg("SD-CARD is in SPI mode\r\n");
		return 0;
	} else {
		//dbg("SD-CARD error\r\n");
	    return -1;
	}
}

/*
 * @ sd card close
 */
void SD_close(void) {
	uint8 sd_cmd[5];
	int i;
	
	memset(sd_cmd, 0 ,sizeof(sd_cmd));
	sd_cmd[0] = GO_IDLE_STATE;
	
	/* write some dummy clocks */
	SPI_enable(GPIOR_SD_CHANNEL);
	for(i = 0; i < 32; i++) {
   		SPI_write(GPIOR_SD_CHANNEL, 0xff);
   	}
	SPI_disable(GPIOR_SD_CHANNEL);
	/* GO_IDLE_STATE, should return 0x01 */
	sd_write_cmd(GPIOR_SD_CHANNEL, sd_cmd);
	
	/* write some dummy clocks */
	SPI_enable(GPIOR_SD_CHANNEL);
	for(i = 0; i < 32; i++) {
   		SPI_write(GPIOR_SD_CHANNEL, 0xff);
   	}
	SPI_disable(GPIOR_SD_CHANNEL);
	
	SPI_close(GPIOR_SD_CHANNEL);
}

/*
 * @ read single block by block address
 */
int SD_read_block(int sector, unsigned char *buffer) {
    volatile uint16 timeout = 0;
	uint16 i;
	uint8 sd_cmd[5];
	uint8 cache;
	uint16 block_crc16 = 0;
	uint16 crc2;
	int err = 0;
   
	if(buffer == NULL) 
		return 0;
		
	/* block address to liner address */
    sector *= 512;
    sd_cmd[0] = READ_SINGLE_BLOCK;
    sd_cmd[1] = (uint8)((sector & 0xff000000) >> 24);
    sd_cmd[2] = (uint8)((sector & 0x00ff0000) >> 16);
    sd_cmd[3] = (uint8)((sector & 0x0000ff00) >> 8);
    sd_cmd[4] = (uint8)((sector & 0x000000ff) >> 0);
    
    /* READ_SINGLE_BLOCK command */
    sd_write_cmd(GPIOR_SD_CHANNEL, sd_cmd);

    /* try to get sync packet 0xfe */
	SPI_enable(GPIOR_SD_CHANNEL);
	do {
		cache = SPI_read(GPIOR_SD_CHANNEL);
       	timeout ++;
    } while((cache != 0xfe) && (timeout < SD_READ_TIMEOUT));
	
    if(timeout < SD_READ_TIMEOUT) {
	    for(i = 0; i < 512; i++) {
        	*(buffer ++) = SPI_read(GPIOR_SD_CHANNEL);
       	}
		block_crc16 = (SPI_read(GPIOR_SD_CHANNEL) << 8);
		block_crc16 |= (SPI_read(GPIOR_SD_CHANNEL) & 0xff);
#if (SDSPI_READ_CRC16)
		crc2 = crc16(0, buffer - 512, 512);
		if(crc2 != block_crc16) {
			TRACE_DEBUG("CRC ERROR E[0x%x]G[0x%x]", crc2 & 0xFFFF, block_crc16 & 0xFFFF);
			err = -1;			
		}
#endif
	} else {
		err = -1;
    }
	SPI_disable(GPIOR_SD_CHANNEL);
	return err;
}

/*
** @ read many blocks
**/
int SD_read_many_blocks(int sector, unsigned char *buffer, int nblock) {
	volatile uint16 timeout = 0;
	uint16 i;
	uint8 sd_cmd[5];
	uint8 cache;
	uint16 block_crc16 = 0;
	uint16 crc2;
	int err = 0;
   
	if(buffer == NULL) 
		return 0;
		
	/* block address to liner address */
    sector *= 512;
    sd_cmd[0] = READ_MULTIPLE_BLOCK;
    sd_cmd[1] = (uint8)((sector & 0xff000000) >> 24);
    sd_cmd[2] = (uint8)((sector & 0x00ff0000) >> 16);
    sd_cmd[3] = (uint8)((sector & 0x0000ff00) >> 8);
    sd_cmd[4] = (uint8)((sector & 0x000000ff) >> 0);
    
    /* READ_MULTIPLE_BLOCK command */
    sd_write_cmd(GPIOR_SD_CHANNEL, sd_cmd);

    /* try to get sync packet 0xfe */
	SPI_enable(GPIOR_SD_CHANNEL);
	
	while(nblock --) {
		timeout = 0;
		do {
			cache = SPI_read(GPIOR_SD_CHANNEL);
			timeout ++;
		} while((cache != 0xfe) && (timeout < SD_READ_TIMEOUT));
		
		if(timeout < SD_READ_TIMEOUT) {
			for(i = 0; i < 512; i++) {
				*(buffer ++) = SPI_read(GPIOR_SD_CHANNEL);
			}
			block_crc16 = (SPI_read(GPIOR_SD_CHANNEL) << 8);
			block_crc16 |= (SPI_read(GPIOR_SD_CHANNEL) & 0xff);
#if (SDSPI_READ_CRC16)
			crc2 = crc16(0, buffer - 512, 512);
			if(crc2 != block_crc16) {
				TRACE_DEBUG("CRC ERROR E[0x%x]G[0x%x]", crc2 & 0xFFFF, block_crc16 & 0xFFFF);
				err = -1;			
			}
#endif
		} else {
			TRACE_DEBUG("read timeout");
			err = -1;
		}
		if(err == -1) {
			break;
		}
	}
	memset(sd_cmd, 0, sizeof(sd_cmd));
	sd_cmd[0] = STOP_TRANSMISSION;
	/* STOP_TRANSMISSION */
	sd_write_cmd(GPIOR_SD_CHANNEL, sd_cmd);
	SPI_disable(GPIOR_SD_CHANNEL);
	return err;
}

/*
** @ write many blocks
**/
int SD_write_many_blocks(int sector, unsigned char *buffer, int nblock) {
	volatile uint16 timeout = 0;
	uint16 i;
	uint8 sd_cmd[5];
	uint8 cache;
	uint16 block_crc16 = 0;
	int err = 0;	
	
   	/* block address to liner address */
    sector *= 512;
    sd_cmd[0] = WRITE_MULTIPLE_BLOCK;
    sd_cmd[1] = (uint8)((sector & 0xff000000) >> 24);
    sd_cmd[2] = (uint8)((sector & 0x00ff0000) >> 16);
    sd_cmd[3] = (uint8)((sector & 0x0000ff00) >> 8);
    sd_cmd[4] = (uint8)((sector & 0x000000ff) >> 0);
    block_crc16 = crc16(0, buffer, 512);
    
    /* WRITE_MULTIPLE_BLOCK command */
    sd_write_cmd(GPIOR_SD_CHANNEL, sd_cmd);
	
	SPI_enable(GPIOR_SD_CHANNEL);
	
	while(nblock --) {
		/* write sync and data */
		SPI_write(GPIOR_SD_CHANNEL, 0xfc);
		for(i = 0; i < 512; i++) {
			SPI_write(GPIOR_SD_CHANNEL, *(buffer ++));
		}
		
		/* send crc16 code */
		SPI_write(GPIOR_SD_CHANNEL, (block_crc16 & 0xff00)>>8);
		SPI_write(GPIOR_SD_CHANNEL, block_crc16 & 0x00ff);   
		   
		/* read send back */
		cache = SPI_read(GPIOR_SD_CHANNEL);
		switch(cache & 0x1F) {
		case 0x05:	/* Data accepted */
			timeout = SD_WRITE_TIMEOUT;
			while((SPI_read(GPIOR_SD_CHANNEL) == 0x00) && (-- timeout));
			if(timeout == 0) {
				err = -1;
				TRACE_DEBUG("multi write timeout");
			} else {
				TRACE_DEBUG("multi write data accepted");
			}
			break;
		case 0x0b:	/* Data rejected due to a CRC error */
			TRACE_DEBUG("multi write CRC error");
			err = -1;
			break;	
		case 0x0d:	/* Data Rejected due to a Write Error */
			TRACE_DEBUG("multi write error");
			err = -1;
			break;
		default:
			TRACE_DEBUG("multi write unknown error");
			err = -1;
		}
		if(err == -1) {
			break;
		}		
	}
#if (1)
	/* stop transfer token */
	SPI_write(GPIOR_SD_CHANNEL, 0xfd);
	for(i = 0; i < 8; i ++) {
		SPI_write(GPIOR_SD_CHANNEL, 0xff);
	}
	timeout = SD_WRITE_TIMEOUT;
	while((SPI_read(GPIOR_SD_CHANNEL) == 0x00) && (-- timeout));
	if(timeout == 0) {
		err = -1;
	}
#endif
	
#if (0)	
	memset(sd_cmd, 0, sizeof(sd_cmd));
	sd_cmd[0] = STOP_TRANSMISSION;
	/* STOP_TRANSMISSION */
	sd_write_cmd(GPIOR_SD_CHANNEL, sd_cmd);
	timeout = SD_WRITE_TIMEOUT;
	while((SPI_read(GPIOR_SD_CHANNEL) == 0x00) && (-- timeout));
	if(timeout == 0) {
		err = -1;
	}
#endif	
	SPI_disable(GPIOR_SD_CHANNEL);
	return err;
}


/*
 * @ write single block by block address
 */
int SD_write_block(int sector, unsigned char *buffer) {
	volatile uint16 timeout = 0;
	uint16 i;
	uint8 sd_cmd[5];
	uint8 cache;
	uint16 block_crc16 = 0;
	int err = 0;	
	
   	/* block address to liner address */
    sector *= 512;
    sd_cmd[0] = WRITE_SINGLE_BLOCK;
    sd_cmd[1] = (uint8)((sector & 0xff000000) >> 24);
    sd_cmd[2] = (uint8)((sector & 0x00ff0000) >> 16);
    sd_cmd[3] = (uint8)((sector & 0x0000ff00) >> 8);
    sd_cmd[4] = (uint8)((sector & 0x000000ff) >> 0);
    block_crc16 = crc16(0, buffer, 512);
	//dbg("write block crc16 = 0x");//dbg_hex(block_crc16);//dbg("\r\n");
    
    /* WRITE_SINGLE_BLOCK command */
    sd_write_cmd(GPIOR_SD_CHANNEL, sd_cmd);
	
	SPI_enable(GPIOR_SD_CHANNEL);
	/* write sync and data */
    SPI_write(GPIOR_SD_CHANNEL, 0xfe);
    for(i = 0; i < 512; i++) {
       	SPI_write(GPIOR_SD_CHANNEL, *(buffer ++));
    }
    
	/* send crc16 code */
    SPI_write(GPIOR_SD_CHANNEL, (block_crc16 & 0xff00)>>8);
    SPI_write(GPIOR_SD_CHANNEL, block_crc16 & 0x00ff);   
   	   
	/* read send back */
	cache = SPI_read(GPIOR_SD_CHANNEL);
    if((cache & 0x0f) == 0x05){
		while(SPI_read(GPIOR_SD_CHANNEL) == 0x00);
		err = 0;
	} else {
		//dbg("call to write block:");//dbg_hex(cache);//dbg("\r\n");
		err = -1;
	}
	SPI_disable(GPIOR_SD_CHANNEL);
	return err;
}

#endif // if (GPIOR_SDSPI)
