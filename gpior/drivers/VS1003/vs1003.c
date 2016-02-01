/***************************************************
*    文件名称: VS1003.c                               
*        用途: vs1003驱动程序               
*        作者: jk36125                             
*    创建日期: 2009.4.1                           
*    最后修改: 2009.4.1                                   
*        备注: 无    
****************************************************/


#include "iocfg.h"

#if (GPIOR_VS1003)

#undef TRACE_LEVEL
#undef TRACE_TITLE
#undef TRACE_TITLE
#define TRACE_LEVEL	TRACE_LEVEL_DEBUG
#define TRACE_NAME  TRACE_NAME_ENABLE
#define TRACE_TITLE "VS"
#include "trace.h"


/* wait device busy */ 
#define VS_WAIT_DREQ() 		while(GPIOR_GETIO(VS_PORT, VS_DREQ_PIN) == 0)

/*
 * @ VS1003 IO config
 */
__inline static void VS_IO_INIT() {
	VS_PLAT_INIT();
	GPIOR_CFGIO_INPUT(VS_PORT, VS_DREQ_PIN);
	GPIOR_CFGIO_OUTPUT(VS_PORT, VS_RST_PIN);
}

/*
 * @ write SPI 16BITS
 */
__inline static uint16 VSPI_WRITE(int ch, uint16 dat) {
	uint16 cache = 0;

	cache = SPI_write(ch, dat >> 8) << 8;
	cache |= SPI_write(ch, dat & 0xff);
	return cache;
}

/*
 * @ reset VS1003
 */
__inline static void VS_HW_RESET() {
	GPIOR_DELAYUS(2000); // min 16600 * T(XTAL-IN)
    GPIOR_CLRIO(VS_PORT, VS_RST_PIN);
    GPIOR_DELAYUS(10);	// min 2 * T(XTAL-IN)
    GPIOR_SETIO(VS_PORT, VS_RST_PIN);
	GPIOR_DELAYUS(10000); // min 50000 * T(XTAL-IN)
	VS_WAIT_DREQ();
}


/*
 * @ VS1003 read register 
 */
unsigned short VS1003_read_reg(uint16 address){
	uint16 cache;
	
	VS_WAIT_DREQ();
	SPI_enable(GPIOR_VS1003_CMDCH);
	VSPI_WRITE(GPIOR_VS1003_CMDCH, address & 0x00ff | 0x0300);
	cache = VSPI_WRITE(GPIOR_VS1003_CMDCH, 0xffff);
	SPI_disable(GPIOR_VS1003_CMDCH);
	return cache;
}


/*
 * @ VS1003 write register 
 */
void VS1003_write_reg(uint16 address, uint16 value){
	VS_WAIT_DREQ();
	SPI_enable(GPIOR_VS1003_CMDCH);
    VSPI_WRITE(GPIOR_VS1003_CMDCH, address & 0x00ff | 0x0200);
	VSPI_WRITE(GPIOR_VS1003_CMDCH, value);
	SPI_disable(GPIOR_VS1003_CMDCH);
}


/*
 * @ VS1003 write data
 */
void VS1003_write_data(char *buf, int len) {
   VS_WAIT_DREQ();
   SPI_enable(GPIOR_VS1003_DATCH);
   while(len --) {
       SPI_write(GPIOR_VS1003_DATCH, *(buf ++));
   }
   SPI_disable(GPIOR_VS1003_DATCH);
}

/*
 * @ VS1003 SIN test 
 */
void VS1003_sin_test(uint32 time) {
     char vsdata[8];
	 /*设置为测试模式*/
	 VS1003_write_reg(0x00,0x0820); //正弦测试
     GPIOR_DELAYUS(100000);
	 	 
	 vsdata[0] = 0x53;
     vsdata[1] = 0xef;
     vsdata[2] = 0x6e;
     vsdata[3] = 0x30;
     vsdata[4] = 0x00;
     vsdata[5] = 0x00;
     vsdata[6] = 0x00;
     vsdata[7] = 0x00;

     VS1003_write_data(&vsdata[0],8);
	 GPIOR_DELAYUS(time);

	 vsdata[0] = 0x45;
     vsdata[1] = 0x78;
     vsdata[2] = 0x69;
     vsdata[3] = 0x74;

	 VS1003_write_data(&vsdata[0],8);
}

/*
 * @ VS1003 RAM test 
 */
uint16 VS1003_ram_test(uint32 time) {
     char vsdata[8];
	 /*设置为测试模式*/
	 VS1003_write_reg(0x00,0x0820); //正弦测试
     GPIOR_DELAYUS(100000);
	 	 
	 vsdata[0] = 0x4d;
     vsdata[1] = 0xea;
     vsdata[2] = 0x6d;
     vsdata[3] = 0x54;
     vsdata[4] = 0x00;
     vsdata[5] = 0x00;
     vsdata[6] = 0x00;
     vsdata[7] = 0x00;

     VS1003_write_data(&vsdata[0],8);
	 GPIOR_DELAYUS(time);   
	 return VS1003_read_reg(0x08);
}


/*
 * @ open VS1003 with volume and bass
 */
int VS1003_open() {
	uint16 buf;
	uint16 retry = 32;

	VS_IO_INIT();
	SPI_open(GPIOR_VS1003_CMDCH, 500000);
	SPI_open(GPIOR_VS1003_DATCH, 1000000);
    
	/* register[0x00] should be 0x800 after reset */
	do {
		VS_HW_RESET();
		buf = VS1003_read_reg(0x00);
	} while((buf != 0x800) && (-- retry));
    if(retry == 0){
   	    return -1;  
    } else {  
		/* mode */
        VS1003_write_reg(0x00, 0x0800);
        GPIOR_DELAYUS(50000);
		/* clock */
        VS1003_write_reg(0x03, 0x9800);
		GPIOR_DELAYUS(50000);
        //GPIOR_DELAYUS(100);
		/* bass */
        //VS1003_write_reg(0x02, bass);	    
        //GPIOR_DELAYUS(100);
		/* volume */
        //VS1003_write_reg(0x0b, volume);
        //GPIOR_DELAYUS(2000000);
		return 0;
	}
}


/*
** @ close device
**/
int VS1003_close() {
	VS_HW_RESET();
	SPI_close(GPIOR_VS1003_CMDCH);
	SPI_close(GPIOR_VS1003_DATCH);
	VS_PLAT_DEINIT();
	return 0;
}

#else
#error SET GPIOR_VS1003 TO 1 TO ENALE VS1003 DRIVER
#endif // if (GPIOR_VS1003)











