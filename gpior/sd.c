#include "sd.h"
#include "iocfg.h"
#include "crc7.h"
#include "crc16.h"
#include <string.h>

#if (GPIOR_SD)

#undef TRACE_LEVEL
#undef TRACE_TITLE
#undef TRACE_TITLE
#define TRACE_LEVEL	TRACE_LEVEL_NO_TRACE
#define TRACE_NAME  TRACE_NAME_DISABLE
#define TRACE_TITLE "SD"
#include "trace.h"

__packed typedef struct _SDCMD {
	uint8	index;
	__packed union {
		uint32 value32;
		uint8  value8[4];
	} argu;
} SD_CMD_T;


static void sd_block_crc16(unsigned char *buffer, unsigned char *crc_buf);


#if !(SD_MAX_FREQ)
__inline void sd_dummy_delay(volatile int us) {
	while (us --);
}

/* clock frequency control */
static uint8  SDIO_CLK_DELAY = SDIO_IDEN_CLKDEL_VALUE;
#endif

/* HOST VDD range */ 
#define VDD33_32		0x00100000
#define VDD32_31		0x00080000
#define VDD31_30		0x00040000
#define VDD30_29		0x00020000
#define VDD29_28		0x00010000
#define HOST_VDD_RANGE	( VDD33_32 | VDD32_31 | VDD31_30 | VDD30_29 | VDD29_28 )

/* BUS wide */
#define SD_1BIT_BUS		0
#define SD_4BIT_BUS		2

/* clock control value for iden mode, with _dummy_delay() */
// #define SDIO_IDEN_CLKDEL_VALUE		5
/* clock control value for tran mode, with _dummy_delay() */
// #define SDIO_TRAN_CLKDEL_VALUE		0

/* timeout define */
#define READ_TIMEOUT		100000
#define WRITE_TIMEOUT		2000000

 
/* config 4 D-LINES input */
#define SDIO_CFG_DIN()		GPIOR_CFGIO_INPUT(SDIO_PORT, (SDIO_D0_PIN | SDIO_D1_PIN | SDIO_D2_PIN | SDIO_D3_PIN))
/* config 4 D-LINES output */
#define SDIO_CFG_DOUT()		GPIOR_CFGIO_OUTPUT(SDIO_PORT, (SDIO_D0_PIN | SDIO_D1_PIN | SDIO_D2_PIN | SDIO_D3_PIN))

#if !(SD_MAX_FREQ)
/* clear and set clock pin */
#	define SDIO_CLR_CLK()	GPIOR_CLRIO(SDIO_PORT, SDIO_CLK_PIN);sd_dummy_delay(SDIO_CLK_DELAY)
#	define SDIO_SET_CLK()	GPIOR_SETIO(SDIO_PORT, SDIO_CLK_PIN);sd_dummy_delay(SDIO_CLK_DELAY)	
#else 
#	define SDIO_CLR_CLK()	GPIOR_CLRIO(SDIO_PORT, SDIO_CLK_PIN)
#	define SDIO_SET_CLK()	GPIOR_SETIO(SDIO_PORT, SDIO_CLK_PIN)
#endif
/* 
 * @ wait D0 line to be HIGH 
 */
static __inline int SDIO_WAIT_BUSY(volatile unsigned int timeout) {
	int count = 0;

	while(-- timeout) {
		SDIO_CLR_CLK();	
		SDIO_SET_CLK();
		if(GPIOR_GETIO(SDIO_PORT, SDIO_D0_PIN)) {
		 	count ++;
		}
		if(count > 16)
			break;
	}
	if(timeout == 0)
		return -1;
	return 0;
}

/*
 * @ response type 
 */
typedef enum _SD_RES {
	SD_NORES,
 	SD_48BITS_RES,
	SD_136BITS_RES,
	SD_48BITS_RES_BUSY,
} SD_RES_T;

/*
 * @ GPIO init
 */
static int sd_gpio_init() {
	SDIO_PLAT_INIT();
	GPIOR_CFGIO_OUTPUT(SDIO_PORT, (SDIO_CMD_PIN | SDIO_CLK_PIN | SDIO_D0_PIN | SDIO_D1_PIN | SDIO_D2_PIN | SDIO_D3_PIN));
	GPIOR_SETIO(SDIO_PORT, (SDIO_CMD_PIN | SDIO_CLK_PIN | SDIO_D0_PIN | SDIO_D1_PIN | SDIO_D2_PIN | SDIO_D3_PIN));
	return 0;
}

/*
 * @ write sd some dummy clocks
 */
static void sd_write_init_clocks(int cycles) {
 	int i;

	for(i = 0; i < cycles; i ++) {
	 	SDIO_CLR_CLK();
		SDIO_SET_CLK();
	}
}

/*
 * @ write bytes to cmd line
 */
static void sd_write_cmd_byte(uint8 dat) {
 	int i;

	for(i = 0; i < 8; i ++) {
	 	SDIO_CLR_CLK();
		if(dat & 0x80) {
			GPIOR_SETIO(SDIO_PORT, SDIO_CMD_PIN);
		} else {
		 	GPIOR_CLRIO(SDIO_PORT, SDIO_CMD_PIN);
		}
		SDIO_SET_CLK();
		dat <<= 1;	
	}
}

/*
 * @ read bytes from cmd line
 */
static int sd_read_cmd_byte() {
 	int i;
	uint8 cache = 0;
	
	for(i = 0; i < 8; i ++) {
		cache <<= 1;
	 	SDIO_CLR_CLK();
		SDIO_SET_CLK();
		if(GPIOR_GETIO(SDIO_PORT, SDIO_CMD_PIN)) {
		 	cache |= 1;
		}	
	}
	return cache;
}

/*
 * @ read response index
 */
static int sd_read_res_index() {
 	int i;
	uint8 cache = 0;
	volatile int timeout = READ_TIMEOUT;

	/* sync the response start bit */
	while(-- timeout) {
	 	SDIO_CLR_CLK();
		SDIO_SET_CLK();
		if(GPIOR_GETIO(SDIO_PORT, SDIO_CMD_PIN) == 0)
		 	break;
	}
	if(timeout == 0) {
		TRACE_DEBUG("RES TIMEOUT!");
		return -1;
	}
	/* left seven bits */
	for(i = 0; i < 7; i ++) {
		cache <<= 1;
	 	SDIO_CLR_CLK();
		SDIO_SET_CLK();
		if(GPIOR_GETIO(SDIO_PORT, SDIO_CMD_PIN)) {
		 	cache |= 1;
		}	
	}
	TRACE_DEBUG("RES INDEX=%d", cache);
	return cache;
}

/*
 * @ read crc ok response after write opt
 * @ start bit 	err bits		stop bit
 * 		0		(010,101,110)	1
 */
static int sd_read_crc_status() {
 	uint8 cache = 0;
	volatile int timeout = 1000;
	int i;

	/* sync start bit */
    while(-- timeout) {
	 	SDIO_CLR_CLK();
		SDIO_SET_CLK();
		if(GPIOR_GETIO(SDIO_PORT, SDIO_D0_PIN) == 0)
		 	break;
	}
	/* 3bits data */
	for(i = 0; i < 3; i ++) {
	 	cache <<= 1;
		SDIO_CLR_CLK();
		SDIO_SET_CLK();
		if(GPIOR_GETIO(SDIO_PORT, SDIO_D0_PIN))
			cache |= 1;	
	}
	/* stop bit */
	SDIO_CLR_CLK();
	SDIO_SET_CLK();
	switch(cache) {
	case 2:
		TRACE_DEBUG("CRC ACCEPTED");
		return 0;
	case 5:
		TRACE_DEBUG("CRC ERROR");
		return -1;
	case 6:
		TRACE_DEBUG("WRITE ERROR");
		return -1;
	default:
		TRACE_DEBUG("UKNOWN CRC STATUS[%d]", cache);
		return -1;
	}
}

/*
 * @ read 48bits response
 * @ return card status 32bits
 * @ NOT check the CRC value
 */
static int sd_read_48bits_rsp(uint8 cmd_index, uint32 *buffer) {
	int i;
	
	*buffer = 0;
	/* commad index */
	if(sd_read_res_index() == -1)
		return -1;
	/* argument */
	for(i = 0; i < 4; i ++) {
		(*buffer) <<= 8;
		(*buffer) |= sd_read_cmd_byte();
	}
	/* crc value, leave it along */
	sd_read_cmd_byte();
	TRACE_DEBUG("48BIT RSP CONTENT=0x%x", *buffer);
	return 0;	
}

/*
 * @ read 136bits response
 * @ return CID or CSD 128bits
 * @ NOT check the CRC value
 */
static int sd_read_136bits_rsp(uint8 cmd_index, uint32 buffer[4]) {
	int i, j;
	
	/* commad index */
	if(sd_read_res_index() == -1)
		return -1;
	for(j = 0; j < 4; j ++) {
		buffer[j] = 0;
		for(i = 0; i < 4; i ++) {
			buffer[j] <<= 8;
			buffer[j] |= sd_read_cmd_byte();
		}
	}
	/* crc value */
	sd_read_cmd_byte();
	TRACE_DEBUG("136BIT RSP CONTENT=0x%x 0x%x 0x%x 0x%x", 
		buffer[0], buffer[1], buffer[2], buffer[3]);
	return 0;	
}

 

/*
 * @ read one block(512B), 4BIT bus
 */
static int sd_read_block_data(uint8 *buffer) {
	int i;
	volatile int try_limit;
	uint8 crc[8];
#if (SD_READ_CRC16) 
	uint8 crc2[8];
#endif
	
	try_limit = READ_TIMEOUT;
	/* wait for the start bit */
	while(-- try_limit) {
	 	SDIO_CLR_CLK();
		SDIO_SET_CLK();
		if(SDIO_GET_D4BIT() == 0)
		 	break;
	}
	if(try_limit == 0) {
	 	TRACE_DEBUG("READ BLOCK TIMEOUT")
		return -1;
	}
	/* datas */
	for(i = 0; i < 512; i ++) {
		SDIO_CLR_CLK();
		SDIO_SET_CLK();
		buffer[i] = SDIO_GET_D4BIT() << 4; 	
		SDIO_CLR_CLK();
		SDIO_SET_CLK();
		buffer[i] |= SDIO_GET_D4BIT();
	}
	memset(crc, 0, sizeof(crc));
	/* crc16 */
	for(i = 0; i < 8; i ++) {
		SDIO_CLR_CLK();
		SDIO_SET_CLK();
		crc[i] = (SDIO_GET_D4BIT() << 4) & 0xF0;
		SDIO_CLR_CLK();
		SDIO_SET_CLK();
		crc[i] |= SDIO_GET_D4BIT();
	}
	TRACE_DEBUG("CRC16 line mode[HEX]: ");
#if (TRACE_LEVEL > TRACE_LEVEL_NO_TRACE)
	for(i = 0; i < 16; i ++) {
	 	TRACE_DEBUG_WP("%x", crc[i]);
	}
#endif
	
#if (SD_READ_CRC16) 
	sd_block_crc16(buffer, crc2);
	for(i = 0; i < 8; i ++) {
		if(crc[i] != crc2[i]) {
			TRACE_DEBUG("READ CRC ERROR!");
			return -1;
		}
	}
#endif
 	return 0;
}

/*
 * @ generate crc16 on each 4 SD lines
 * @ buffer should be 512 size of byte
 */
static unsigned short sd_datline_crc16(unsigned char *buffer, char line_id) {
	unsigned char cache[128];
	unsigned char mask_high;
	unsigned char mask_low;
	int x, y;
	
	mask_high = (0x10 << line_id);
	mask_low  = (0x01 << line_id);
	for(x = 0; x < 128; x ++) {
		cache[x] = 0;
		for(y = 0; y < 4; y ++) {
			/* 2bits per time */
			cache[x] <<= 2;
			if((*buffer) & mask_high) {
				cache[x] |= 0x2;
			}
			if((*buffer) & mask_low) { 
				cache[x] |= 0x1;
			}
			buffer ++;
		}
	}
	return crc16(0, cache, 128);
}

/*
 * @ write crc16 value on each 4 SD lines
 */
static void sd_block_crc16(unsigned char *buffer, unsigned char *crc_buf) {
	unsigned short crc[4];
	unsigned char cache;
	int i, j, k = 0;

	for(i = 0; i < 4; i ++) {
		crc[i] = sd_datline_crc16(buffer, i);
		TRACE_DEBUG("CRC16[D%d LINE]=0x%x\n", i, crc[i]);
	}
	TRACE_DEBUG("CRC16 line mode[HEX]: ");
	/* 4short -> 8bytes */
	for(i = 0; i < 16; i ++) {
		cache = 0;
		*crc_buf &= ~0x0f;
		for(j = 0; j < 4; j ++) {
			cache >>= 1;
			if(crc[j] & 0x8000) {
				cache |= 0x08;
			}
			crc[j] <<= 1;
		}
		TRACE_DEBUG_WP("%x", cache);
		*crc_buf |= cache; 
		k ++;
		if(k == 2) {
			k = 0;
			crc_buf ++;
		} else {
		 	*crc_buf <<= 4;
		}
	}
}

/*
 * @ write one block(512B), 4BIT bus
 */
static void sd_write_block_data(uint8 *buffer) {
	int i;
	unsigned char crc_value[8];

	sd_block_crc16(buffer, crc_value);
	/* write some dummy clocks */
	sd_write_init_clocks(16);
	/* start bit */
	SDIO_CLR_CLK();
	SDIO_SET_D4BIT(0x00);
	SDIO_SET_CLK();
	/* datas */		
	for(i = 0; i < 512; i ++) {
		SDIO_CLR_CLK();
		SDIO_SET_D4BIT(buffer[i] >> 4);
		SDIO_SET_CLK();		   			
		SDIO_CLR_CLK();
		SDIO_SET_D4BIT(buffer[i]);
		SDIO_SET_CLK();	  		
	}
	/* crc16 is on each line separately */
	for(i = 0; i < 8; i ++) {
	 	SDIO_CLR_CLK();
		SDIO_SET_D4BIT(crc_value[i] >> 4);
		SDIO_SET_CLK();		   			
		SDIO_CLR_CLK();
		SDIO_SET_D4BIT(crc_value[i]);
		SDIO_SET_CLK();
	}		
	/* stop bit */
	SDIO_CLR_CLK();
	SDIO_SET_D4BIT(0x0f);
	SDIO_SET_CLK();
}

/*
 * @ sd CRC7 culculate
 */
static uint8 sd_cmd_crc(SD_CMD_T *cmd) {
 	uint8 buffer[5];

	buffer[0] = cmd->index | 0x40;
	buffer[1] = cmd->argu.value8[3];
	buffer[2] = cmd->argu.value8[2];
	buffer[3] = cmd->argu.value8[1];
	buffer[4] = cmd->argu.value8[0];
	return crc7(0, (const unsigned char *)buffer, sizeof(buffer)) | 1;
}

/*
 * @ sd write commad
 */
static void sd_write_command(SD_CMD_T *cmd) {
	uint8 crc;

	crc = sd_cmd_crc(cmd);
	TRACE_DEBUG("WRITE CMD=%d, ARGU=0x%x, CRC=0x%x", 
		cmd->index, cmd->argu.value32, crc);
	sd_write_cmd_byte(cmd->index | 0x40);
	sd_write_cmd_byte(cmd->argu.value8[3]);
	sd_write_cmd_byte(cmd->argu.value8[2]);
	sd_write_cmd_byte(cmd->argu.value8[1]);
	sd_write_cmd_byte(cmd->argu.value8[0]);
	sd_write_cmd_byte(crc);
}

/* 
 * @ short-cut for sd_write_command 
 */
static int SD_WRITE_CMD(SD_CMD_T *cmd, uint8 index, uint32 argu, uint32 *res_buf, SD_RES_T res_t) {
	cmd->index = index;
	cmd->argu.value32 = argu;
	/* write some dummy clocks */
	sd_write_init_clocks(16);
	sd_write_command(cmd);
	/* we do not read response in READ commad */
	if((cmd->index == 17) || (cmd->index == 18))
		return 0;
	GPIOR_CFGIO_INPUT(SDIO_PORT, SDIO_CMD_PIN);
	switch(res_t) {
	case SD_48BITS_RES:
		if(sd_read_48bits_rsp(index, res_buf) == -1)
			return -1;
		break;
	case SD_136BITS_RES:
		if(sd_read_136bits_rsp(index, res_buf) == -1)
			return -1;
		break;
	case SD_48BITS_RES_BUSY:
		if(sd_read_48bits_rsp(index, res_buf) == -1)
			return -1;
		break; 		
	}
	GPIOR_CFGIO_OUTPUT(SDIO_PORT, SDIO_CMD_PIN);
	return 0;
}

/* 
 * @ short-cut for sd_write_app_command 
 */
static int SD_WRITE_ACMD(SD_CMD_T *cmd, uint8 index, uint32 argu, uint32 *res_buf, SD_RES_T res_t, uint32 RCA) {
 	if(SD_WRITE_CMD(cmd, 55, RCA, res_buf, res_t) != 0)
		return -1;
	return SD_WRITE_CMD(cmd, index, argu, res_buf, res_t);
}


/*
 * @ init sd card
 */
int SD_open(void) {
	SD_CMD_T cmd;
	uint32 try_limit;
	uint32 buffer[4];
	uint32 RCA;
	int state;	

	/* manu-config for clock frequency */
	sd_gpio_init();
#if !(SD_MAX_FREQ)
	SDIO_CLK_DELAY = SDIO_IDEN_CLKDEL_VALUE;
#endif	
	sd_write_init_clocks(128);
	/* reset card */
    SD_WRITE_CMD(&cmd, 0, 0, buffer, SD_NORES);
	/* read OCR register, buffer[0] = OCR value */
	if(SD_WRITE_ACMD(&cmd, 41, HOST_VDD_RANGE, buffer, SD_48BITS_RES, 0) == -1)
		return -1;
	/* reset card again */
	SD_WRITE_CMD(&cmd, 0, 0, buffer, SD_NORES);
	/* wait for card ready */
	try_limit = 3000;
	do {
	 	state = SD_WRITE_ACMD(&cmd, 41, buffer[0], buffer, SD_48BITS_RES, 0);
	} while(((buffer[0] & 0x80000000) == 0) && (state == 0) && (-- try_limit));
	if(try_limit == 0) {
	 	TRACE_DEBUG("OPEN CARD TIMEOUT");
		return -1;
	}
	TRACE_DEBUG("CARD IS READY");
	/* read CID register */
	if(SD_WRITE_CMD(&cmd, 2, 0, buffer, SD_136BITS_RES) == -1)
		return -1;
	/* publish RCA */
	if(SD_WRITE_CMD(&cmd, 3, 0, buffer, SD_48BITS_RES) == -1)
		return -1;
	RCA = buffer[0] & 0xffff0000;
	/* read CSD register */
	if(SD_WRITE_CMD(&cmd, 9, RCA, buffer, SD_136BITS_RES) == -1)
		return -1;
	/* select card */
	if(SD_WRITE_CMD(&cmd, 7, RCA, buffer,  SD_48BITS_RES) == -1)
		return -1;
#if !(SD_MAX_FREQ)
	/* make clock high frequency from here */
	SDIO_CLK_DELAY = SDIO_TRAN_CLKDEL_VALUE;
#endif	
	/* select 4bit bus */
	if(SD_WRITE_ACMD(&cmd, 6, SD_4BIT_BUS, buffer, SD_48BITS_RES, RCA) == -1)
		return -1;
	/* set block size */
	if(SD_WRITE_CMD(&cmd, 16, 512, buffer, SD_48BITS_RES) == -1)
		return -1;
	return 0;
}

/*
 * @ read single block from sd card
 */
int SD_read_block(int block_addr, uint8 *buffer) {
	uint32 status = 0;
	SD_CMD_T cmd;

	SDIO_CFG_DIN();
	block_addr *= 512;
	SD_WRITE_CMD(&cmd, 17, block_addr, &status, SD_48BITS_RES);
	if(sd_read_block_data(buffer) == -1) {
		status = 1;
	} else {
	 	status = 0;
	}
	/* wait busy status */
	SDIO_WAIT_BUSY(READ_TIMEOUT);
	SDIO_CFG_DOUT();
 	if(status)
		return -1;
	return 0;
}

/*
** @ read many many blocks
**/
int SD_read_many_blocks(int block_addr, uint8 *buffer, int nblock) {
	uint32 status = 0;
	int err = 0;
	SD_CMD_T cmd;

	SDIO_CFG_DIN();
	block_addr *= 512;
	/* start transfer */
	SD_WRITE_CMD(&cmd, 18, block_addr, &status, SD_48BITS_RES);
	while (nblock --) {		
		if(sd_read_block_data(buffer) == -1) {
			err = 1;
			break;
		}
		buffer += 512;
	}
	/* stop command */
	SD_WRITE_CMD(&cmd, 12, 0, &status, SD_48BITS_RES);
	/* wait busy status */
	SDIO_WAIT_BUSY(READ_TIMEOUT);
	SDIO_CFG_DOUT();
 	if(err)
		return -1;
	return 0;
}

/*
 * @ write single block to sd card
 */
int SD_write_block(int block_addr, uint8 *buffer) {
 	uint32 status = 0;
	SD_CMD_T cmd;

	block_addr *= 512;
	SD_WRITE_CMD(&cmd, 24, block_addr, &status, SD_48BITS_RES);
	SDIO_CFG_DOUT();
 	sd_write_block_data(buffer);
	GPIOR_CFGIO_INPUT(SDIO_PORT, SDIO_D0_PIN);
	if(sd_read_crc_status() != 0) {
	 	status = 1;
	} else {
	 	status = 0;
	}
	/* wait busy status */
	SDIO_WAIT_BUSY(WRITE_TIMEOUT);
	GPIOR_CFGIO_OUTPUT(SDIO_PORT, SDIO_D0_PIN);
 	if(status)
		return -1;
	return 0;
}

/*
 * @ write single block to sd card
 */
int SD_write_many_blocks(int block_addr, uint8 *buffer, int nblock) {
 	uint32 status = 0;
	int err = 0;
	SD_CMD_T cmd;
	int res;

	block_addr *= 512;
	SD_WRITE_CMD(&cmd, 25, block_addr, &status, SD_48BITS_RES);
	SDIO_CFG_DOUT();
	while (nblock --) {		
		sd_write_block_data(buffer);
		GPIOR_CFGIO_INPUT(SDIO_PORT, SDIO_D0_PIN);
		res = sd_read_crc_status();
		GPIOR_CFGIO_OUTPUT(SDIO_PORT, SDIO_D0_PIN);
		if(res != 0) {
			err = 1;
			break;
		}
		buffer += 512;
	}
	/* stop command */
	SD_WRITE_CMD(&cmd, 12, 0, &status, SD_48BITS_RES);
	GPIOR_CFGIO_INPUT(SDIO_PORT, SDIO_D0_PIN);
	/* wait busy status */
	SDIO_WAIT_BUSY(WRITE_TIMEOUT);
	SDIO_CFG_DOUT();
 	if(err)
		return -1;
	return 0;
}

/*
 * @ close sd card
 */
void SD_close(void) {
	SD_CMD_T cmd;
	uint32 buffer[4];
	
	/* reset SD-CARD */
	SD_WRITE_CMD(&cmd, 0, 0, buffer, SD_NORES);
	
	SDIO_CFG_DIN();
	GPIOR_CFGIO_INPUT(SDIO_PORT, (SDIO_CMD_PIN | SDIO_CLK_PIN));
	SDIO_PLAT_DEINIT();
}

#else
#error SET GPIOR_SD TO 1 TO ENABLE SD DRIVER(IN iocfg.h)
#endif /* if (GPIOR_SD) */
