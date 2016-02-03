#ifndef _IOCFG_H_
#define _IOCFG_H_

#if (0)

//*----------------------------------------------------------------
//*		Low-level IO functions for GPIOR drivers
//*----------------------------------------------------------------
/** @ config IO input **/
#define GPIOR_CFGIO_INPUT(PORT, PIN)	(0)/* add code here */

/** @ config IO output **/
#define GPIOR_CFGIO_OUTPUT(PORT, PIN)	(0)/* add code here */

/** @ set IO **/
#define GPIOR_SETIO(PORT, PIN)  		(0)/* add code here */

/** @ reset IO **/
#define GPIOR_CLRIO(PORT, PIN)			(0)/* add code here */

/** @ get IO, return ZERO or NOT **/
#define GPIOR_GETIO(PORT, PIN)			(0)/* add code here */

/** @ the GPIO port macro **/
#define GPIOR_PORT(PORT)				(0)/* add code here */

/** @ the GPIO pin macro **/
#define GPIOR_PIN(PIN)					(0)/* add code here */

/** @ delay micro-seconds **/
__inline void  GPIOR_DELAYUS(volatile int us) {
	while (us --);
}

//*----------------------------------------------------------------
//*		Config the GPIOR functions (1 = ENABLE, 0 = DISABLE)
//*----------------------------------------------------------------
//#define GPIOR_TEST		1	// Basic IO SET and RESET test
//#define GPIOR_SPI		1	// SPI Driver
//#define GPIOR_SD		1	// SD-card Driver(SD mode, NOT SPI mode)
//#define GPIOR_SDSPI		1	// SD-card Driver(SPI mode)
//#define GPIOR_I2C		1	// I2C Driver
//#define GPIOR_SCCB		1	// SCCB Driver
//#define GPIOR_NKLCD		0	// NokiaLCD5110 Driver
//#define GPIOR_VS1003	1	// VS1003 Driver
//#define GPIOR_RF24L		1	// NRF24L01 Driver
//#define GPIOR_E2PROM	1	// AT24CXX Driver
//#define GPIOR_DS18B20	1	// DS18B20 Driver


//*----------------------------------------------------------------
//*		Test the GPIOR_SETIO and GPIOR_CLRIO macros
//*----------------------------------------------------------------
#if (GPIOR_TEST)

#define GPIOR_FREQTEST_PORT		GPIOR_PORT(2)
#define GPIOR_FREQTEST_PIN		GPIOR_PIN(12)

/* MAX GPIOR IO FREQUENCY */
#define GPIOR_MAX_FREQHZ		2000000

__inline void GPIO_test(void) {
	/* config IO output */
	GPIOR_CFGIO_OUTPUT(GPIOR_FREQTEST_PORT, GPIOR_FREQTEST_PIN);
	while(1) {
	 	GPIOR_SETIO(GPIOR_FREQTEST_PORT, GPIOR_FREQTEST_PIN);
		GPIOR_DELAYUS(1);
		GPIOR_CLRIO(GPIOR_FREQTEST_PORT, GPIOR_FREQTEST_PIN);
		GPIOR_DELAYUS(1);
	}
}

#endif /* if(GPIOR_TEST_FUNC) */

//*----------------------------------------------------------------
//*		GPIOR SPI PINS CONFIG
//*----------------------------------------------------------------
#if (GPIOR_SPI)

/* DO NOT CHANGE THESE VALUES */
#define SPI_CLOCK_INACTIVE_HIGH			1
#define SPI_CLOCK_INACTIVE_LOW			0
#define SPI_CAPTURE_ON_FALLING_EDGE		0	
#define SPI_CAPTURE_ON_RISING_EDGE		1
#define SPI_CS_ACTIVE_LOW				0
#define SPI_CS_ACTIVE_HIGH				1

/* CPOL: Clock Polarity */
#define SPI_CPOL			SPI_CLOCK_INACTIVE_HIGH

/* NCPHA: Clock Phase */
#define SPI_CLPH			SPI_CAPTURE_ON_RISING_EDGE

/* Chip select line */
#define SPI_CS				SPI_CS_ACTIVE_LOW

/* Ignore the SPCK frequency setting, go as far as we could */
#define SPI_MAX_FREQ		0

/* SPI IO port */
#define SPI_PORT    	 	GPIOR_PORT(0) 	

/* SPI #CS line */
#define SPI_NPCS0_PIN		GPIOR_PIN(0)
#define SPI_NPCS1_PIN		GPIOR_PIN(0)
#define SPI_NPCS2_PIN		GPIOR_PIN(0)
#define SPI_NPCS3_PIN		GPIOR_PIN(0)

/* SPI MOSI line */
#define SPI_MOSI_PIN		GPIOR_PIN(0)
/* SPI MISO line */
#define SPI_MISO_PIN		GPIOR_PIN(0)
/* SPI SPCK line */
#define SPI_SPCK_PIN		GPIOR_PIN(0)

/** @ FIXED 4 channels **/
__inline unsigned int SPI_NPCS_PIN(int ch) {
	switch(ch) {
	case 0: 
		return SPI_NPCS0_PIN;
	case 1: 
		return SPI_NPCS1_PIN;
	case 2: 
		return SPI_NPCS2_PIN;
	case 3: 
		return SPI_NPCS3_PIN;
	default: 
		return SPI_NPCS0_PIN;
	}
}

/* 
** @ called at the beginning of SPI_open()
**/
__inline void SPI_PLAT_INIT(int ch) {
}

/* 
** @ called at the end of SPI_close()
**/
__inline void SPI_PLAT_DEINIT(int ch) {
}
#endif /* if(GPIOR_SPI) */

//*----------------------------------------------------------------
//*		SD-CARD PINS CONFIG (NOT SPI MODE)
//*----------------------------------------------------------------
#if (GPIOR_SD)

/* Ignore the frequency setting, go as far as we could */
/* If anything goes wrong, reset this value to ZERO */
#define SD_MAX_FREQ			0

/* Do CRC-16 check on SD_read(), this will cost time */
#define SD_READ_CRC16		1

/* SD-BUS: PORT */
#define SDIO_PORT    	 	GPIOR_PORT(0) 	

/* SD-BUS: CMD line */
#define SDIO_CMD_PIN		GPIOR_PIN(0)

/* SD-BUS: CLOCK line */
#define SDIO_CLK_PIN		GPIOR_PIN(0)

/* SD-BUS: D[0,3] lines */
#define SDIO_D0_PIN			GPIOR_PIN(0)
#define SDIO_D1_PIN			GPIOR_PIN(0)
#define SDIO_D2_PIN			GPIOR_PIN(0)
#define SDIO_D3_PIN			GPIOR_PIN(0)

#if !(SD_MAX_FREQ)
/* clock control value for iden mode, the bigger, the slower */
#	define SDIO_IDEN_CLKDEL_VALUE		0

/* clock control value for tran mode, the bigger, the slower */
#	define SDIO_TRAN_CLKDEL_VALUE		0
#endif

/* 
** @ called at the beginning of SD_open()
**/
__inline void SDIO_PLAT_INIT(void) {
}

/* 
** @ called at the end of SD_close()
**/
__inline void SDIO_PLAT_DEINIT(void) {
}

/*
** @ read 4bits from SD[0,3]
** @ return from 0 to 1111B 
** @ do optimized for different cpu 
**/
__inline unsigned char SDIO_GET_D4BIT() {
	unsigned char cache = 0;
	
	if(GPIOR_GETIO(SDIO_PORT, SDIO_D0_PIN))
		cache |= 0x01;
	if(GPIOR_GETIO(SDIO_PORT, SDIO_D1_PIN))
		cache |= 0x02;
	if(GPIOR_GETIO(SDIO_PORT, SDIO_D2_PIN))
		cache |= 0x04;
	if(GPIOR_GETIO(SDIO_PORT, SDIO_D3_PIN))
		cache |= 0x08;
	return cache; 
}

/* 
** @ write 4bits to SD[0,3]
** @ do optimized for different cpu
**/
__inline void SDIO_SET_D4BIT(unsigned char data) {
	unsigned int set_mask = 0;
	unsigned int clr_mask = 0;

	data &= 0x0f;
	((data & 0x01) ? (set_mask |= SDIO_D0_PIN) : (clr_mask |= SDIO_D0_PIN));
	((data & 0x02) ? (set_mask |= SDIO_D1_PIN) : (clr_mask |= SDIO_D1_PIN));
	((data & 0x04) ? (set_mask |= SDIO_D2_PIN) : (clr_mask |= SDIO_D2_PIN));
	((data & 0x08) ? (set_mask |= SDIO_D3_PIN) : (clr_mask |= SDIO_D3_PIN));
	GPIOR_CLRIO(SDIO_PORT, clr_mask);
	GPIOR_SETIO(SDIO_PORT, set_mask);
}
#endif /* if (GPIOR_SD) */

//*----------------------------------------------------------------
//*		I2C PINS CONFIG
//*----------------------------------------------------------------
#if (GPIOR_I2C)

/* ISC PORT */
#define I2C_PORT		GPIOR_PORT(0) 	

/* I2C clock line */
#define I2C_SCL_PIN		GPIOR_PIN(0)

/* I2C data line */
#define I2C_SDA_PIN		GPIOR_PIN(0)

/* 
** @ called at the beginning of I2C_open()
**/
__inline void I2C_PLAT_INIT(void) {
}

/* 
** @ called at the end of I2C_close()
**/
__inline void I2C_PLAT_DEINIT(void) {
}
#endif /* if (GPIOR_I2C) */

//*----------------------------------------------------------------
//*		SD-SPI PINS CONFIG (SPI MODE FOR SD-CARD)
//*----------------------------------------------------------------
#if (GPIOR_SDSPI)

/* SD-SPI channel, must be one of the 4 SPI channels */
#define GPIOR_SD_CHANNEL	0

/* Do CRC-16 check on SD_read(), this will cost time */
#define SDSPI_READ_CRC16	1

#endif /* if (GPIOR_SDSPI) */

//*----------------------------------------------------------------
//*		LCD5110 DRIVERS CONFIG (NOKIA LCD5110 84 * 48 LCD)
//*----------------------------------------------------------------
#if (GPIOR_NKLCD)

/* LCD5110-SPI channel, must be one of the 4 SPI channels */
#define GPIOR_LCD_CHANNEL	0

/* LCD 8 * 5 Bitmap Fonts */
#define GPIOR_LCD_BITMAP	1

/* LCD icons */
#define GPIOR_LCD_ICONS		1

/* LCD PORT */
#define LCD_PORT		GPIOR_PORT(0)

/* LCD D/C line */
#define LCD_DC_PIN		GPIOR_PIN(0)

/* LCD reset line */
#define LCD_RST_PIN		GPIOR_PIN(0)

/* 
** @ called at the beginning of LCD_open()
**/	
__inline void LCD_PLAT_INIT(void) {	
}

/* 
** @ called at the end of LCD_close()
**/	
__inline void LCD_PLAT_DEINIT(void) {	
}
#endif /* GPIOR_NKLCD */

//*----------------------------------------------------------------
//*		DS18B20 DRIVERS CONFIG
//*----------------------------------------------------------------
#if (GPIOR_SCCB)

/* DS18B20 PORT */
#define DS18B20_PORT	GPIOR_PORT(0)

/* DS18B20 1-line bus */
#define DS18B20_PIN		GPIOR_PIN(0)

/* 
** @ called at the beginning of DS_open()
**/	
__inline void DS18B20_PLAT_INIT(void) {
}

/* 
** @ called at the end of DS_close()
**/	
__inline void DS18B20_PLAT_DEINIT(void) {
}
#endif /* if (GPIOR_SCCB) */

//*----------------------------------------------------------------
//*		NRF24L01 DRIVERS CONFIG
//*----------------------------------------------------------------
#if (GPIOR_RF24L)

/* NRF24L01-SPI channel, must be one of the 4 SPI channels */
#define GPIOR_RF_CHANNEL	0

/* RF INTERRUPT MODE (nRF24L01 IRQ line IO interrupt enable) */
#define RF_INT_MODE			0

/* nRF24L01 PORT */
#define RF_PORT			GPIOR_PORT(0)

/* nRF24L01 CE line */
#define RF_CE_PIN		GPIOR_PIN(0)

/* nRF24L01 IRQ line */
#define RF_IRQ_PIN		GPIOR_PIN(0)

/* 
** @ called at the beginning of RF_open()
**/	
__inline void RF_PLAT_INIT(void) {
}

/* 
** @ called at the end of RF_close()
**/	
__inline void RF_PLAT_DEINIT(void) {
}
#endif /* if (GPIOR_RF24L) */

//*----------------------------------------------------------------
//*		VS1003(B) DRIVER CONFIG
//*----------------------------------------------------------------
#if (GPIOR_VS1003)

/* VS1003-COMMAND-SPI channel, must be one of the 4 SPI channels */
#define GPIOR_VS1003_CMDCH		0

/* VS1003-DATA-SPI channel, must be one of the 4 SPI channels */
#define GPIOR_VS1003_DATCH		1

/* VS1003 PORT */
#define VS_PORT			GPIOR_PORT(0)

/* VS1003 REST line */
#define VS_RST_PIN		GPIOR_PIN(0)

/* VS1003 DREQ line */
#define VS_DREQ_PIN		GPIOR_PIN(0)

/* 
** @ called at the beginning of VS_open()
**/	
__inline void VS_PLAT_INIT(void) {
}

/* 
** @ called at the end of VS_close()
**/	
__inline void VS_PLAT_DEINIT(void) {
}
#endif /* if (GPIOR_VS1003) */

//*----------------------------------------------------------------
//*		SCCB DRIVER CONFIG
//*----------------------------------------------------------------
/* CONFIG SCCB IOS IN I2C PINS CONFIG */

//*----------------------------------------------------------------
//*		E2PROM DRIVER CONFIG (AT24CXX)
//*----------------------------------------------------------------
#if (GPIOR_E2PROM)
/* CONFIG E2PROM CLK AND DAT IN I2C PINS CONFIG */

/* enable the E2PROM write protect function */
#define GPIOR_E2PROM_WP		0

#if (GPIOR_E2PROM_WP)
/* E2PROM WP port */
#define E2PROM_WP_PORT	GPIOR_PORT(0)

/* E2PROM WP line */
#define E2PROM_WP_PIN	GPIOR_PIN(0)
#endif // if (GPIOR_E2PROM_WP)

/* 
** @ called at the beginning of E2PROM_open()
**/	
__inline void E2PROMWP_PLAT_INIT(void) {
}

/* 
** @ called at the end of E2PROM_close()
**/	
__inline void E2PROMWP_PLAT_DEINIT(void) {
}
#endif /* if (GPIOR_E2PROM) */

#endif /* ifndef _IOCFG_H_ */

#endif // if 0
