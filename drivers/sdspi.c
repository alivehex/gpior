#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "crc7.h"
#include "crc16.h"
#include "spi.h"
#include "sdspi.h"
#include "dbgu/dbgu.h"

#undef DEBUG_ME
#define DEBUG_ME	1

#if (DEBUG_ME)
# define spi_dbg	printf
# define spi_dbgln	printf
#else
# define spi_dbg	while (0)
# define spi_dbgln	while (0)
#endif

#define SDSPI_SS_INDEX	SPI_SS_0

/* what we need in sd-spi driver from spi */
typedef struct {
	void (*spi_open)(SpiSlaveSelect ss, uint32_t freqhz);
	void (*spi_close)(SpiSlaveSelect ss);
	void (*spi_ss_low)(SpiSlaveSelect ss);
	void (*spi_ss_high)(SpiSlaveSelect ss);
	uint8_t (*spi_write_byte)(SpiSlaveSelect ss, uint8_t data);
	void (*spi_write)(SpiSlaveSelect ss, uint8_t *output, uint8_t* input, int length);
	void (*spi_write_async)(SpiSlaveSelect ss, uint8_t *output, uint8_t* input, int length);
} SpiDriver;

static SpiDriver drv = {
	.spi_open = spi_open,
	.spi_close = spi_close,
	.spi_ss_low = spi_ss_low,
	.spi_ss_high = spi_ss_high,
	.spi_write_byte = spi_write_byte,
	.spi_write = spi_write,
	.spi_write_async = NULL,
};


/* SD card command */
#define GO_IDLE_STATE			(0x00 | 0x40)
#define SEND_CSD				(0x09 | 0x40)
#define SEND_CID				(0x0a | 0x40)
#define STOP_TRANSFER			(0x0c | 0x40)
#define READ_SINGLE_BLOCK   	(0x11 | 0x40)
#define READ_MULTIPLE_BLOCKS	(0x12 | 0x40)
#define WRITE_SINGLE_BLOCK		(0x18 | 0x40)
#define WRITE_MULTIPLE_BLOCKS	(0x19 | 0x40)
#define APP_CMD	   				(0x37 | 0x40)
#define CRC_ON_OFF   			(0x3b | 0x40)
#define SEND_OP_COND   			(0x29 | 0x40)
#define PRE_ERASED				(0x17 | 0x40)
#define CMD8					(8 	  | 0x40)
#define CMD58					(58   | 0x40)
#define CMD9					(9    | 0x40)
#define CMD10					(10   | 0x40)

/* timeout when enter SD card */
#define SD_SPI_MODE_TIMEOUT	(unsigned long)5000
/* read timeout */
#define SD_READ_TIMEOUT		(unsigned long)1000000
/* write timeout */
#define SD_WRITE_TIMEOUT	(unsigned long)0xFFFFF

#define SPI_WCACHE_BLOCKS		8
// 4 bytes pre-writting clock + 512 bytes data + 
// 1 byte dummy colock + 2 bytes crc16 + 1 byte command response
#define SPI_WCACHE_BLOCK_SIZE	520 
#define SD_BLOCK_SIZE			512

typedef enum {
	SD1_1,
	SD2_0,
} SDVersion;

typedef enum {
	IDLE = 0,
	WRITE_CMD23,
	WRITE_CMD25,
	WRITE_BLOCK,
	WAIT_BUSY,
	STOP_TRAN,
} MutipleWriteStage;

typedef union {
	uint32_t u32value;
	uint8_t uint8value[4];
} Msb2Lsb;

//static uint8_t SpiTxCache[SPI_WCACHE_BLOCK_SIZE * SPI_WCACHE_BLOCKS];
//static uint8_t SpiRxCache[1];
//static uint8_t SpiCmdCache[9];
//static int SpiTxCacheIndex = 0;
//static int CacheBlockCount = 0;
//static int DMABlockCount = 0;
//static int LastWriteError = 0;
//static uint32_t CacheBlockAddress = 0;
//static uint8_t *CachePtr = NULL;
//static volatile MutipleWriteStage DMAStage = IDLE;
//static volatile int DMABusy = 0;
//static int RequestStopTransferCmd = 0;
//static int ReadMulCmdNotFinish = 0;
//static uint32_t PrevReadBlockIndex = (uint32_t)(-1);

static SDVersion Version = SD1_1;
// From CSD register
static float CapacityKByte = 0.0;
static uint32_t MaxBusFrequency = 0;
//static uint32_t BlockLength = 0x200;
// From CID register
//static uint8_t ManufacturerId = 0;
//static uint8_t OEMId[3] = "";
static uint8_t ProductName[6] = "";
//static uint8_t ProductRevision = 0;
static uint32_t ProductSerialNumber = 0;
//static uint16_t ManufacturingDate = 0;

static void read_cid_reg(void);
static void read_csd_reg(void);
//#define spi_read_byte()		spi_write_byte(0xFF)

/* Functions to serial put data input DMA cache */
//static void spiTxCacheReset(uint8_t *ptr) {
//	CachePtr = ptr;
//	SpiTxCacheIndex = 0;
//}

//static void spiTxCachePushByte(uint8_t b) {
//	CachePtr[SpiTxCacheIndex ++] = b;
//}

//static void spiTxCachePushBuffer(uint8_t *buffer, int length) {
//	memcpy(&CachePtr[SpiTxCacheIndex], buffer, length);
//	SpiTxCacheIndex += length;
//}

//static void spiTxCachePushRepeat(uint8_t b, int length) {
//	memset(&CachePtr[SpiTxCacheIndex], b, length);
//	SpiTxCacheIndex += length;
//}

//static int spiTxCacheCount(void) {
//	return SpiTxCacheIndex;
//}
	
/*
 * @ write SD-SPI command
 */
static void sd_write_cmd(uint8_t *cmd, uint8_t *res, uint16_t reslen) {
	int i;
	
	/* pre-writting clock */
	drv.spi_write_byte(SDSPI_SS_INDEX, 0xff);
	
	for(i = 0; i < 5; i++) { 
		drv.spi_write_byte(SDSPI_SS_INDEX, *(cmd+i));
	}
	drv.spi_write_byte(SDSPI_SS_INDEX, crc7(0, cmd, 5));
	
	/* pre-writting clock */
	drv.spi_write_byte(SDSPI_SS_INDEX, 0xff);
	
	if (res) {
		for(i = 0; i < reslen; i ++) {
			res[i] = drv.spi_read_byte(SDSPI_SS_INDEX);
		}
	}
}

// Easier way to write data transmit commands
static uint8_t sd_write_cmd_argu32(uint8_t cmd, uint32_t argument) {
	uint8_t sdCmd[5];
	uint8_t res;
	Msb2Lsb buffer;
	
	buffer.u32value = argument;
	sdCmd[0] = cmd;
	sdCmd[1] = buffer.uint8value[3]; 
    sdCmd[2] = buffer.uint8value[2]; 
    sdCmd[3] = buffer.uint8value[1]; 
    sdCmd[4] = buffer.uint8value[0];
	
	sd_write_cmd(sdCmd, &res, 1);
	return res;
}

// Sync wait for busy state with timeout
static int wait_busy_end(volatile unsigned long timeout) {
	uint8_t ret;
	
	do {
		ret = drv.spi_write_byte(SDSPI_SS_INDEX, 0xff);
	} while ( (ret != 0xff) && (-- timeout) );
	
	if (timeout == 0) {
		return -1;
	} else {
		return 0;
	}
}


// Read bits from MSB format stream buffer
static uint32_t bitsFromMsbBuffer(uint8_t *buffer, int buflen, int start, int stop) {
	int index, bits;
	uint32_t ret = 0;
	
	start = 127 - start;
	stop = 127 - stop;
	
	// the data format is MSB output first, so reverse bit setting	
	for(int i = stop; i <= start; i ++) {
		ret <<= 1;
		
		index = i / 8;
		bits = 7 - (i % 8);
		if (buffer[index] & (1 << bits)) {
			ret |= 1;
		}
	}
	return ret;
}

static void read_cid_reg(void) {
	uint32_t timeout;
	uint8_t cid[16];
	uint8_t token;
	uint8_t crc7Token;
	
	sd_write_cmd_argu32(CMD10, 0);
	
	// Data is NOT in command response
	timeout = 20000;
	do {
		token = drv.spi_write_byte(SDSPI_SS_INDEX, 0xff);
	} while((token != 0xfe) && (-- timeout));
	
	if (timeout == 0) {
		spi_dbgln("read CID register timeout");
		return;
	}
	
	// read 16bytes CID register
	drv.spi_write(SDSPI_SS_INDEX, NULL, cid, 16);
	
	// Check the CRC7 value
	crc7Token = crc7(0, cid, 15);
	
	if (cid[15] != crc7Token) {
		spi_dbgln("read CID crc error");
	}
	
	wait_busy_end(100);
	
	//ManufacturerId = cid[0]; 
	//memcpy(OEMId, &cid[1], 2);
	memcpy(ProductName, &cid[3], 5);
	//ProductRevision = cid[8];
	ProductSerialNumber = cid[12] | (cid[11] << 8) | (cid[10] << 16) | (cid[9] << 24);
	//ManufacturingDate = ((cid[13] & 0x0f) << 8) | cid[14];
}

static void read_csd_reg(void) {
	uint32_t blockLength = 0;
	uint32_t cVersion = 0;
	uint32_t tranSpeed = 0;
	uint32_t timeout;
	uint8_t csd[16];
	uint8_t token;
	uint8_t crc7Token;
	float capacity;
	
	sd_write_cmd_argu32(CMD9, 0);
	
	// Data is NOT in command response
	timeout = 20000;
	do {
		token = drv.spi_write_byte(SDSPI_SS_INDEX, 0xff);
	} while((token != 0xfe) && (-- timeout));
	
	if (timeout == 0) {
		spi_dbgln("read CSD register timeout");
		return;
	}
	
	// DMA read 16bytes CSD register
	drv.spi_write(SDSPI_SS_INDEX, NULL, csd, 16);
	
	// Check the CRC7 value
	crc7Token = crc7(0, csd, 15);
	if (csd[15] != crc7Token) {
		spi_dbgln("read CSD crc error");
	}
	
	wait_busy_end(100);
	
	cVersion = bitsFromMsbBuffer(csd, sizeof(csd), 126, 127);
	
	// Transmit clock frequency
	tranSpeed = bitsFromMsbBuffer(csd, sizeof(csd), 96, 103);
	if (tranSpeed == 0x32) {
		tranSpeed = 25000000;
	} else if (tranSpeed == 0x5a) {
		tranSpeed = 50000000;
	} 
	
	blockLength = (1 << bitsFromMsbBuffer(csd, sizeof(csd), 80, 83));
	
	uint32_t cSize;
	if (cVersion == 0) {
		uint32_t cSizeMul;
		
		cSize =  bitsFromMsbBuffer(csd, sizeof(csd), 62, 73);
		cSizeMul =  bitsFromMsbBuffer(csd, sizeof(csd), 47, 49);

		capacity = (cSize + 1) * (1 << (cSizeMul + 2)) * blockLength / 1024;		
	} else if (cVersion == 1) {
		cSize =  bitsFromMsbBuffer(csd, sizeof(csd), 48, 69);
		
		capacity = (cSize + 1) * blockLength;
	} else {
		spi_dbgln("unknown CSD version");
	}
	
	//BlockLength = blockLength;
	CapacityKByte = capacity;
	MaxBusFrequency = tranSpeed;
}

/*
 * @ SD-card init into SPI mode
 */
int sd_open(void) {
    uint8_t stat, i;
	uint8_t sd_cmd[5];
	uint8_t cmdRes[5];
	uint16_t tryLimit;
	
	/* slower clock frequency at beginning */
	drv.spi_open(SDSPI_SS_INDEX, 200000);
	
	/* atleast 74 clocks */
	drv.spi_ss_low(SDSPI_SS_INDEX);
	for(i = 0; i < 100; i++) {
   		drv.spi_write_byte(SDSPI_SS_INDEX, 0xff);
   	}
	
    /* GO_IDLE_STATE, should return 0x01 */
	stat = sd_write_cmd_argu32(GO_IDLE_STATE, 0);
       
	if(stat != 0x01) {
		spi_dbgln("Init SD error");
		return -1;
	}

	/* SD2.0 or SD1.0? */
	memset(sd_cmd, 0, sizeof(sd_cmd));
	sd_cmd[0] = CMD8;
	// 2.7~3.3V
	sd_cmd[3] = 0x01;
	sd_cmd[4] = 0xaa;
	
	sd_write_cmd(sd_cmd, cmdRes, 5);
	if ( (cmdRes[3] == 0x01) && (cmdRes[4] == 0xaa) ) {
		Version = SD2_0;
	} else {
		Version = SD1_1;
	}
	
	/* try to enter sd SPI mode */ 
	tryLimit = SD_SPI_MODE_TIMEOUT;
    do{
		/* APPCMD */   
		sd_write_cmd_argu32(APP_CMD, 0);
		
		/* SEND_OP_COND */
		stat = sd_write_cmd_argu32(SEND_OP_COND, 0x40000000);
    } while((stat != 0x00) && ( -- tryLimit));

	if (tryLimit == 0) {
		spi_dbgln("Init SD timeout");
		return -1;
	}
	
	// Read CSD and CID register
	// can NOT read after CMD58
	// can NOT read in IDLE mode
	read_cid_reg();
	read_csd_reg();
	
	// Read OCR register, after this command, card is data-transfer mode
	// can not read cid and csd register in data-transfer mode
	memset(sd_cmd, 0, sizeof(sd_cmd));
	sd_cmd[0] = CMD58;
	sd_write_cmd(sd_cmd, cmdRes, 5);
	
	spi_dbg("%s ", ProductName);
	
	if (MaxBusFrequency == 50000000) {
		spi_dbg("High Speed ");
	}
	
	if (Version == SD2_0) {
		if (cmdRes[1] & 0x40) {
			Version = SD2_0;
			spi_dbg("SDHC ");
		} else {
			Version = SD1_1;
			spi_dbg("SD ");
		}
	}
	 
	if ((CapacityKByte / 1024.0) > 1024.0) {
		spi_dbg("%0.1fGB ", CapacityKByte / 1024.0 / 1024.0);
	} else {
		spi_dbg("%0.1fMB ", CapacityKByte / 1024.0);
	}
	
	spi_dbg("SN: %08x\r\n", ProductSerialNumber);
	
	// Over clock to 36Mhz for the slow speed card
	drv.spi_open(SDSPI_SS_INDEX, 25000000);

	drv.spi_ss_high(SDSPI_SS_INDEX);
	return 0;
}

/*
 * @ sd card close
 */
void sd_close(void) {
	drv.spi_close(SDSPI_SS_INDEX);
}

/*
 * @ read single block by block address
 */
int sd_read_block(unsigned int sector, unsigned char * buffer) {
    volatile uint32_t timeout = 0;
	unsigned short i;
	unsigned char cache;
	unsigned short block_crc16 = 0;
	int err = 0;
   
	if (Version == SD1_1) {
		sector *= 512;
	}
    
	drv.spi_ss_low(SDSPI_SS_INDEX);
	
    /* READ_SINGLE_BLOCK command */
	sd_write_cmd_argu32(READ_SINGLE_BLOCK, sector);

    /* try to get sync packet 0xfe */
	do {
		cache = drv.spi_write_byte(SDSPI_SS_INDEX, 0xff);
       	timeout ++;
    } while((cache != 0xfe) && (timeout < SD_READ_TIMEOUT));
	
    if(timeout < SD_READ_TIMEOUT) {
	    for(i = 0; i < 512; i++) {
        	*(buffer ++) = drv.spi_write_byte(SDSPI_SS_INDEX, 0xff);
       	}
		block_crc16 = (drv.spi_write_byte(SDSPI_SS_INDEX, 0xff) << 8);
		block_crc16 |= (drv.spi_write_byte(SDSPI_SS_INDEX, 0xff) & 0xff);
		err = 0;
	} else {
		spi_dbgln("read block sync timeout");
		err = -1;
    }
	
	drv.spi_ss_high(SDSPI_SS_INDEX);
	return err;
}

/*
 * @ write single block by block address
 */
int sd_write_block(unsigned int sector, unsigned char * buffer) {
	volatile unsigned short timeout = 0;
	unsigned short i;
	unsigned char cache;
	unsigned short block_crc16 = 0;
	int err = 0;	
	
	if (Version == SD1_1) {
		sector *= 512;
	}
	
    block_crc16 = crc16(0, buffer, 512);
    
	drv.spi_ss_low(SDSPI_SS_INDEX);
	
    /* WRITE_SINGLE_BLOCK command */
	sd_write_cmd_argu32(WRITE_SINGLE_BLOCK, sector);
	
	/* write sync and data */
    drv.spi_write_byte(SDSPI_SS_INDEX, 0xfe);
    for(i = 0; i < 512; i++) {
       	drv.spi_write_byte(SDSPI_SS_INDEX, *(buffer ++));
    }
    
	/* send crc16 code */
    drv.spi_write_byte(SDSPI_SS_INDEX, (block_crc16 & 0xff00) >> 8);
    drv.spi_write_byte(SDSPI_SS_INDEX, (block_crc16 & 0x00ff));   
   	   
	/* read send back */
	cache = drv.spi_write_byte(SDSPI_SS_INDEX, 0xff);
    if((cache & 0x0f) == 0x05){
		while(drv.spi_write_byte(SDSPI_SS_INDEX, 0xff) == 0x00);
		err = 0;
	} else {
		spi_dbgln("write block crc error");
		err = -1;
	}
	
	drv.spi_ss_high(SDSPI_SS_INDEX);
	return err;
}

int sd_read_multiple_blocks(int blockIndex, char *buffer, int numOfBlocks) {
	volatile unsigned long timeout = 0;
	unsigned short i, j;
	unsigned char sd_cmd[5];
	unsigned char cache;
	unsigned short block_crc16;
	unsigned short softwareCRC16;
	int err = 0;
	uint32_t address;
	uint8_t dataRes;
	
	if (Version == SD1_1) {
		address = blockIndex * 512;
	} else if (Version == SD2_0) {
		address = blockIndex;
	} else {
		return 0;
	}
	
	/* block address to liner address */
    sd_cmd[0] = READ_MULTIPLE_BLOCKS;
    sd_cmd[1] = (unsigned char)((address & 0xff000000) >> 24);
    sd_cmd[2] = (unsigned char)((address & 0x00ff0000) >> 16);
    sd_cmd[3] = (unsigned char)((address & 0x0000ff00) >> 8);
    sd_cmd[4] = (unsigned char)((address & 0x000000ff) >> 0);
    
	drv.spi_ss_low(SDSPI_SS_INDEX);
	
    /* READ_MULTIPLE_BLOCKS command */
    sd_write_cmd(sd_cmd, &dataRes, 1);

	/* try to get sync packet 0xfe */
	for(j = 0; j < numOfBlocks; j ++) {
		err = 0;
		timeout = 0;
		
		do {
			cache = drv.spi_write_byte(SDSPI_SS_INDEX, 0xff);
			timeout ++;
		} while((cache != 0xfe) && (timeout < SD_READ_TIMEOUT));
		
		if(timeout < SD_READ_TIMEOUT) {
			for(i = 0; i < 512; i++) {
				buffer[i] = drv.spi_write_byte(SDSPI_SS_INDEX, 0xff);
			}
			block_crc16 = (drv.spi_write_byte(SDSPI_SS_INDEX, 0xff) << 8);
			block_crc16 |= (drv.spi_write_byte(SDSPI_SS_INDEX, 0xff) & 0xff);
			
			softwareCRC16 = crc16(0, (unsigned char *)buffer, 512);
			
			//spiRxDMATransfer(buffer, 514);
			//block_crc16 = (buffer[512] << 8);
			//block_crc16 |= (buffer[513] & 0xff);
		} else {
			debug("call to get sync timeout");
			err = 1;
		}
		
		if (err)
			break;
		
		buffer += 512;
	}
	
	/* write stop command */
	memset(sd_cmd, 0 ,sizeof(sd_cmd));
	sd_cmd[0] = STOP_TRANSFER;
	sd_write_cmd(sd_cmd, &dataRes, 1);
	
	//drv.spi_ss_low(SDSPI_SS_INDEX);

	if (wait_busy_end(SD_WRITE_TIMEOUT) != 0) {
		spi_dbgln("Read busy timeout");
		err = 1;
	}
	
	drv.spi_ss_high(SDSPI_SS_INDEX);
	return err;
}

int sd_write_multiple_blocks(int blockIndex, char *buffer, int numOfBlocks) {
	volatile unsigned long timeout = 0;
	unsigned short i, j;
	unsigned char sd_cmd[5];
	unsigned char cache;
	unsigned short block_crc16 = 0;
	int err = 0;
	uint8_t ret;
	uint32_t address;
	
	if (Version == SD1_1) {
		address = blockIndex * 512;
	} else if (Version == SD2_0) {
		address = blockIndex;
	} else {
		return 0;
	}
    
	drv.spi_ss_low(SDSPI_SS_INDEX);
	
	/* pre-defined write blocks command ACMD23 */
	if (numOfBlocks > 1) {
		memset(sd_cmd, 0, 5);
		sd_cmd[0] = APP_CMD;
		sd_write_cmd(sd_cmd, &ret, 1);
		
		/* pre-defined erase blocks */
		sd_cmd[0] = PRE_ERASED;
		sd_cmd[1] = 0;
		sd_cmd[2] = 0;
		sd_cmd[3] = 0;
		sd_cmd[4] = numOfBlocks;
		sd_write_cmd(sd_cmd, &ret, 1);	
	}
	
	/* block address to liner address */
    sd_cmd[0] = WRITE_MULTIPLE_BLOCKS;
    sd_cmd[1] = (unsigned char)((address & 0xff000000) >> 24);
    sd_cmd[2] = (unsigned char)((address & 0x00ff0000) >> 16);
    sd_cmd[3] = (unsigned char)((address & 0x0000ff00) >> 8);
    sd_cmd[4] = (unsigned char)((address & 0x000000ff) >> 0);
	
    /* WRITE_MULTIPLE_BLOCKS command */
    sd_write_cmd(sd_cmd, &ret, 1);
	
	// spi_cs_low(ch);
	
	for(j = 0; j < numOfBlocks; j ++) {
		//block_crc16 = crc16(0, (unsigned char *)buffer, 512);
		
		/* dummy clocks to make the card ready */
		//memset(SPI_DMA_TX_CACHE, 0xff, 4);
		//spiTxDMATransfer(4);
		drv.spi_write_byte(SDSPI_SS_INDEX, 0xff);
		drv.spi_write_byte(SDSPI_SS_INDEX, 0xff);
		
		/* write sync and data */
		drv.spi_write_byte(SDSPI_SS_INDEX, 0xfc);
		//SPI_DMA_TX_CACHE[4] = 0xfc;
		
		for(i = 0; i < 512; i++) {
			drv.spi_write_byte(SDSPI_SS_INDEX, buffer[i]);
		}
		//SPI_DMA_TX_CACHE[0] = 0xfc;
		//memcpy(&SPI_DMA_TX_CACHE[0 + 1], buffer, 512);
	
		/* send crc16 code */
		//spi_write_byte(ch, (block_crc16 & 0xff00)>>8);
		//spi_write_byte(ch, block_crc16 & 0x00ff);
		drv.spi_write_byte(SDSPI_SS_INDEX, 0xff);
		drv.spi_write_byte(SDSPI_SS_INDEX, 0xff);
		
		//SPI_DMA_TX_CACHE[512 + 1] = 0xff;//(block_crc16 & 0xff00) >> 8;   
		//SPI_DMA_TX_CACHE[513 + 1] = 0xff;//block_crc16 & 0x00ff;
		//SPI_DMA_TX_CACHE[514 + 1] = 0xff;//Send back
		//spiTxDMATransfer(515 + 1);
		
		/* read send back */
		//cache = SPI_DMA_RX_CACHE[515];
		cache = drv.spi_write_byte(SDSPI_SS_INDEX, 0xff);
		
		if((cache & 0x0f) == 0x05) {			
			if (wait_busy_end(SD_WRITE_TIMEOUT) != 0) {
				spi_dbgln("Busy timeout");
				err = 1;
			}
		} else {
			spi_dbgln("multiple write error: 0x%x", cache & 0x0f);
			err = 1;
		}
		
		if (err)
			break;
		
		buffer += 512;
	}
	
	/* stop token for CMD25 */
	drv.spi_write_byte(SDSPI_SS_INDEX, 0xfd);
	
	if (wait_busy_end(SD_WRITE_TIMEOUT) != 0) {
		spi_dbgln("Busy timeout");
		err = 1;
	}
	
	drv.spi_ss_high(SDSPI_SS_INDEX);
	return err;
}

#if (0)

// Async wait for busy state, work with DMA interrupt
//static void waitForBusyAsync(void) {
//	// At this moment, SpiTxCache[0] data should be send
//	SpiTxCache[0] = 0xff;
//	spiDMATransfer(NULL, NULL, 32, 0);
//}

// Check if any STOP_TRAN or STOP_TOKEN is need when file is close
int closePrevSdReadMultipleBlocksTransfer(void) {
	int err = 0;
	
	// if (RequestStopTransferCmd) {
		//RequestStopTransferCmd = 0;
	
	if (ReadMulCmdNotFinish) {
		spi_cs_low();
		
		sdWriteCmdSyncArgUint32(STOP_TRANSFER, 0);

		if (waitForBusy(SD_WRITE_TIMEOUT) != 0) {
			spi_dbgln("Stop busy timeout");
			err = 1;
		}
		ReadMulCmdNotFinish = 0;
		
		spi_cs_high();
	}
	return err;
}

/* Read Multiple blocks with DMA */
int sdReadMutipleBlocks(uint32_t blockIndex, uint8_t *buffer, unsigned int numOfBlocks) {
	volatile unsigned long timeout = 0;
	unsigned short j;
	unsigned char cache;
	int err = 0;
	//uint32_t address;
	//uint8_t readCRC16[2];
	
#if (READ_CRC16_CHECK)
	uint16_t softwareCRC16;
	uint16_t block_crc16;
#endif
	
	while (DMAStage != IDLE);
	
	if (Version == SD1_1) {
		blockIndex <<= 9; // x 512
	}
	
	// Just continue readding if the address is matched
	// Save a little bit time from STOP_TRANSFER command
	if (PrevReadBlockIndex != blockIndex) {
		if (closePrevSdReadMultipleBlocksTransfer() != 0) {
			return 1;
		}
		
		spi_cs_low();
		
		/* READ_MULTIPLE_BLOCKS command */
		sdWriteCmdSyncArgUint32(READ_MULTIPLE_BLOCKS, blockIndex);
		// Mark for write multiple opration
		ReadMulCmdNotFinish = 1;
	} else {
		spi_cs_low();
	}
	
	if (Version == SD1_1) {
		PrevReadBlockIndex = blockIndex + (numOfBlocks << 9);
	} else {
		PrevReadBlockIndex = blockIndex + numOfBlocks;
	}
	
	/* try to get sync packet 0xfe */
	for(j = 0; j < numOfBlocks; j ++) {
		err = 0;
		timeout = 0;
		
		do {
			cache = spi_read_byte();
			// DMA is not faster in 1 byte transfer
			// spiDMATransfer(&cache, NULL, 1, 1);
			timeout ++;
		} while((cache != 0xfe) && (timeout < SD_READ_TIMEOUT));
		
		if(timeout < SD_READ_TIMEOUT) {
			// 1 byte dummy clock
			SpiTxCache[0] = 0xff;
			
			// 512 byte block data
			spiDMATransfer(buffer, NULL, 512, 1);
			
#if (READ_CRC16_CHECK)
			readCRC16[0] = spi_read_byte();
			readCRC16[1] = spi_read_byte();
			
			softwareCRC16 = crc16(0, (unsigned char *)buffer, 512);
			
			block_crc16 = (readCRC16[0] << 8);
			block_crc16 |= (readCRC16[1] & 0xff);
			
			if (softwareCRC16 != block_crc16) {
				err = 1;
				spi_dbgln("Read CRC error");
			}
#else
			// only two bytes without DMA is faster about 30KB/S
			spi_read_byte();
			spi_read_byte();
			// spiDMATransfer(readCRC16, NULL, 2, 1);
#endif
		} else {
			spi_dbgln("call to get sync timeout");
			err = 1;
		}
		
		if (err)
			break;
		
		buffer += 512;
	}	
#if (0)
	/* write stop command */
	sdWriteCmdSyncArgUint32(STOP_TRANSFER, 0);

	if (waitForBusy(SD_WRITE_TIMEOUT) != 0) {
		spi_dbgln("Read busy timeout");
		err = 1;
	}
#endif
	
	spi_cs_high();
	return err;
}

int isAsyncWriteDone(void) {
	return (DMAStage == IDLE) ? 1 : 0;
}

int getLastWriteError(void) {
	return LastWriteError;
}

/* With DMA and interrupt */
int sdWriteMutipleBlocksAsync(int blockIndex, char *buffer, int numOfBlocks) {	
	// Wait the last transmit done
	while (DMAStage != IDLE);
	
	// Close the read opration
	if (closePrevSdReadMultipleBlocksTransfer() != 0) {
		return 1;
	}
	// So the read command will be sent when switch to read mode
	PrevReadBlockIndex = (uint32_t)(-1);
	
	// clear the last write error mark
	LastWriteError = 0;
	
	if (Version == SD1_1) {
		CacheBlockAddress = blockIndex << 9; // x 512
	} else {
		CacheBlockAddress = blockIndex;
	}
	
	CacheBlockCount = numOfBlocks;
	DMABlockCount = 0;
	
	// Save the Tx data to DMA cache
	spiTxCacheReset(SpiTxCache);
	for(int i = 0; i < numOfBlocks; i ++) {
		// 4 bytes dummy clocks
		spiTxCachePushRepeat(0xff, 4);
		// 1 byte data packet token
		spiTxCachePushByte(0xfc);
		// 512 bytes block data
		spiTxCachePushBuffer((uint8_t *)buffer, 512);
		// 1 byte dummy clock and 2 bytes crc16 value
		spiTxCachePushRepeat(0xff, 3);
		
		buffer += 512;
	}
	
	spi_cs_low();
	
	if (numOfBlocks > 1) {
		// Write pre-write command if block > 1
		// May it will be faster
		DMAStage = WRITE_CMD23;
		sdWriteCmdAsyncArgUint32(APP_CMD, 0);
	} else {
		// Without ACMD23, start with write command
		DMAStage = WRITE_BLOCK;
		sdWriteCmdAsyncArgUint32(WRITE_MULTIPLE_BLOCKS, CacheBlockAddress);
	}
	
	return 0;
}

int sdWriteMutipleBlocks(int blockIndex, char *buffer, int numOfBlocks) {
	volatile unsigned long timeout = 0;
	unsigned short j;
	//unsigned char sd_cmd[5];
	unsigned char cache;
	int err = 0;
	uint32_t address;
	
	// Close the read opration
	if (closePrevSdReadMultipleBlocksTransfer() != 0) {
		return 1;
	}
	// So the read command will be sent when switch to read mode
	PrevReadBlockIndex = (uint32_t)(-1);
	
	if (Version == SD1_1) {
		address = blockIndex * 512;
	} else if (Version == SD2_0) {
		address = blockIndex;
	} else {
		return 0;
	}
    
	spi_cs_low();
	
	/* pre-defined write blocks command ACMD23 */
	if (numOfBlocks > 1) {
		sdWriteCmdSyncArgUint32(APP_CMD, 0);
		
		/* pre-defined erase blocks */
		/*sd_cmd[0] = PRE_ERASED;
		sd_cmd[1] = 0;
		sd_cmd[2] = 0;
		sd_cmd[3] = 0;
		sd_cmd[4] = numOfBlocks;*/
		sdWriteCmdSyncArgUint32(PRE_ERASED, numOfBlocks);
	}
	
    /* WRITE_MULTIPLE_BLOCKS command */
	sdWriteCmdSyncArgUint32(WRITE_MULTIPLE_BLOCKS, address);
	
	for(j = 0; j < numOfBlocks; j ++) {		
		// dummy clocks to make the card ready
		spiTxCacheReset(SpiTxCache);
		spiTxCachePushRepeat(0xff, 4);
		// 1 byte data packet
		spiTxCachePushByte(0xfc);
		// 512 bytes data
		spiTxCachePushBuffer((uint8_t *)buffer, 512);
		// 2 bytes crc16 and 1byte dummy clock
		spiTxCachePushRepeat(0xff, 3);
		// start transfer
		spiDMATransfer(NULL, SpiTxCache, spiTxCacheCount(), 1);
		
		// read response
		cache = SpiRxCache[0];
		
		if((cache & 0x0f) == 0x05) {			
			if (waitForBusy(SD_WRITE_TIMEOUT) != 0) {
				spi_dbgln("Busy timeout");
				err = 1;
			}
		} else {
			spi_dbgln("multiple write error: 0x%x", cache & 0x0f);
			err = 1;
		}
		
		if (err)
			break;
		
		buffer += 512;
	}
	
	/* stop token for CMD25 */
	spi_write_byte(0xfd);
	
	if (waitForBusy(SD_WRITE_TIMEOUT) != 0) {
		spi_dbgln("Busy timeout");
		err = 1;
	}
	
	spi_cs_high();
	return err;
}

int MTD_read_block(int block_num,  char *buffer, int blocks) {	
	if (sdReadMutipleBlocks(block_num, (uint8_t *)buffer, blocks) != 0) {
		spi_dbgln("read error");
		return -1;
	} else {
		return 0;
	}
}

int MTD_write_block(int block_num, char *buffer, int blocks) {	
	while (blocks > SPI_WCACHE_BLOCKS) {
		sdWriteMutipleBlocksAsync(block_num, buffer, SPI_WCACHE_BLOCKS);
		block_num += SPI_WCACHE_BLOCKS;
		buffer += (SPI_WCACHE_BLOCKS * SD_BLOCK_SIZE);
		blocks -= SPI_WCACHE_BLOCKS;
	}
	return sdWriteMutipleBlocksAsync(block_num, buffer, blocks);
		
	/*if (sdWriteMutipleBlocks(block_num, buffer, blocks) != 0) {
		return -1;
	} else {
		return 0;
	}*/
}

int MTD_device_init(void) {
	return sd_open();
}
	
int MTD_device_deinit(void) {
	return sd_close();
}

/* DMA Transfer Compleate Interrupt Handler */
DMA_SPIRx_IRQHandler {
	
	static int waitForRes = 0;
	int ret;
	
	DMA_ClearFlag(SPI_Rx_DMA_Flag);
	DMABusy = 0;
	
	switch(DMAStage) {
		case WRITE_CMD23:
			// CMD55 response
			ret = SpiRxCache[0];
		
			/*sdCmd[0] = PRE_ERASED;
			sdCmd[1] = 0; sdCmd[2] = 0; sdCmd[3] = 0;
			sdCmd[4] = CacheBlockCount & 0xff;*/
			DMAStage = WRITE_CMD25;
			
			sdWriteCmdAsyncArgUint32(PRE_ERASED, CacheBlockCount & 0xff);
			break;
		case WRITE_CMD25:
			// CMD23 response
			ret = SpiRxCache[0];
			DMAStage = WRITE_BLOCK;
			DMABlockCount = 0;
		
			sdWriteCmdAsyncArgUint32(WRITE_MULTIPLE_BLOCKS, CacheBlockAddress);
			break;
		case WRITE_BLOCK:
			// response from command "WRITE_MULTIPLE_BLOCKS" 
			ret = SpiRxCache[0];
			
			if (DMABlockCount < CacheBlockCount) { 
				spiDMATransfer(NULL, SpiTxCache + DMABlockCount * SPI_WCACHE_BLOCK_SIZE, SPI_WCACHE_BLOCK_SIZE, 0);
				DMABlockCount ++;
				
				DMAStage = WAIT_BUSY;
				waitForRes = 1;
			}
			break;
		case STOP_TRAN:
			// busy result
			ret = SpiRxCache[0];
		
			if (ret != 0xff) {
				// busy not done
				waitForBusyAsync();
			} else {
				DMAStage = IDLE;
				spi_cs_high();
			}
			break;
		case WAIT_BUSY:
			ret = SpiRxCache[0];
			
			// From data transfer status
			if (waitForRes == 1) {
				if((ret & 0x0f) != 0x05) {
					spi_dbgln("write error");
					LastWriteError = 1;
				}
				waitForRes = 0;
				waitForBusyAsync();					
				break;
			}
			
			if (ret != 0xff) {
				// busy not done
				waitForBusyAsync();					
				break;
			}
			
			// there still be data
			if (DMABlockCount < CacheBlockCount) {
				spiDMATransfer(NULL, SpiTxCache + DMABlockCount * SPI_WCACHE_BLOCK_SIZE, SPI_WCACHE_BLOCK_SIZE, 0);
				waitForRes = 1;
				DMABlockCount ++;
			} else {
				// no data left, sent stop token
				spi_write_byte(0xfd);
				DMAStage = STOP_TRAN;
				
				waitForBusyAsync();
			}
			break;
	}
}

#endif // if (0)

