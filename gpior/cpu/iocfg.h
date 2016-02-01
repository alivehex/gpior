#ifndef _IOCFG_H_
#define _IOCFG_H_


#include "hex_gpio.h"
#include "hex_sysdelay.h"


typedef struct _SYSCFG0 {
	vuint32 REVID;		// Revision Identification Register ！ Section 10.4.1
	vuint32 RESERVED0;
	vuint32 DIEIDR[4];	// Die Identification Register [0:4]
	vuint32 DEVIDR0;	// Device Identification Register 0 Privileged mode Section 10.4.2
	vuint32 RESERVED1;
	vuint32 BOOTCFG;	// Boot Configuration Register Privileged mode Section 10.4.3
	vuint32 RESERVED2[5];
	vuint32 KICK0R;		// Kick 0 Register Privileged mode Section 10.4.4.1
	vuint32 KICK1R;		// Kick 1 Register Privileged mode Section 10.4.4.2
	vuint32 HOST0CFG;	// Host 0 Configuration Register ！ Section 10.4.5
	vuint32 RESERVED3[39];
	vuint32 IRAWSTAT;	// Interrupt Raw Status/Set Register Privileged mode Section 10.4.6.1
	vuint32 IENSTAT;	// Interrupt Enable Status/Clear Register Privileged mode Section 10.4.6.2
	vuint32 IENSET;		// Interrupt Enable Register Privileged mode Section 10.4.6.3
	vuint32 IENCLR;		// Interrupt Enable Clear Register Privileged mode Section 10.4.6.4
	vuint32 EOI;		// End of Interrupt Register Privileged mode Section 10.4.6.5
	vuint32 FLTADDRR;	// Fault Address Register Privileged mode Section 10.4.7.1
	vuint32 FLTSTAT;	// Fault Status Register ！ Section 10.4.7.2
	vuint32 RESERVED4[5];
	vuint32 MSTPRI[3];	// Master Priority [0:2]
	vuint32 RESERVED5;
	vuint32 PINMUX[20];	// Pin Multiplexing Control [0:19]
	vuint32 SUSPSRC;	// Suspend Source Register Privileged mode Section 10.4.10
	vuint32 CHIPSIG;	// Chip Signal Register ！ Section 10.4.11
	vuint32 CHIPSIG_CLR;// Chip Signal Clear Register ！ Section 10.4.12
	vuint32 CFGCHIP[5];	// Chip Configuration [0:4]
} AM18S_SYSCFG0, *AM18PS_SYSCFG0;


#define AM18C_BASE_SYSCFG0		((AM18PS_SYSCFG0)0x01C14000)


typedef struct _GPIO_BANK {
	volatile unsigned int DIR;			// GPIO Bank Direction Register
	volatile unsigned int OUT_DATA;		// GPIO Bank Output Data Register
	volatile unsigned int SET_DATA;		// GPIO Bank Set Data Register
	volatile unsigned int CLR_DATA;		// GPIO Bank Clear Data Register
	volatile unsigned int IN_DATA;		// GPIO Bank Input Data Register
	volatile unsigned int SET_RIS_TRIG;	// GPIO Bank Set Rising Edge Interrupt Register
	volatile unsigned int CLR_RIS_TRIG;	// GPIO Bank Clear Rising Edge Interrupt Register
	volatile unsigned int SET_FAL_TRIG;	// GPIO Bank Set Falling Edge Interrupt Register
	volatile unsigned int CLR_FAL_TRIG;	// GPIO Bank Clear Falling Edge Interrupt Register
	volatile unsigned int INTSTAT;		// GPIO Bank Interrupt Status Register
} AM18S_GPIO_BANK, *AM18PS_GPIO_BANK;


typedef struct _GPIO {
	volatile unsigned int REVID;
	volatile unsigned int RESERVED0;
	volatile unsigned int BINTEN;
	volatile unsigned int RESERVED1;
	AM18S_GPIO_BANK BANKS[5];
} AM18S_GPIO, *AM18PS_GPIO;


#define AM18C_BASE_GPIO		((AM18PS_GPIO)0x01E26000)


//*----------------------------------------------------------------
//*		Low-level IO functions for GPIOR drivers
//*----------------------------------------------------------------
/** @ config IO input **/
#define GPIOR_CFGIO_INPUT(PORT, PIN)	AM18C_BASE_GPIO->BANKS[PORT].DIR |= PIN

/** @ config IO output **/
#define GPIOR_CFGIO_OUTPUT(PORT, PIN)	AM18C_BASE_GPIO->BANKS[PORT].DIR &= ~PIN

/** @ set IO **/
#define GPIOR_SETIO(PORT, PIN)  		AM18C_BASE_GPIO->BANKS[PORT].SET_DATA = PIN

/** @ reset IO **/
#define GPIOR_CLRIO(PORT, PIN)			AM18C_BASE_GPIO->BANKS[PORT].CLR_DATA = PIN	

/** @ get IO, return ZERO or NOT **/
#define GPIOR_GETIO(PORT, PIN)			(AM18C_BASE_GPIO->BANKS[PORT].IN_DATA & PIN)

/** @ the GPIO port macro **/
#define GPIOR_PORT(PORT)				(PORT / 2)

/** @ the GPIO pin macro **/
#define GPIOR_PIN(PIN)					(1 << (PIN))

/** @ delay micro-seconds **/
__inline void  GPIOR_DELAYUS(volatile int us) {
	while (us --);
}


//*----------------------------------------------------------------
//*		Config the GPIOR functions (1 = ENABLE, 0 = DISABLE)
//*----------------------------------------------------------------
#define GPIOR_TEST		1	// Basic IO SET and RESET test
#define GPIOR_SPI		1	// SPI Driver
#define GPIOR_SD		1	// SD-card Driver(SD mode, NOT SPI mode)
#define GPIOR_SDSPI		1	// SD-card Driver(SPI mode)
#define GPIOR_I2C		1	// I2C Driver
#define GPIOR_SCCB		1	// SCCB Driver
#define GPIOR_NKLCD		0	// NokiaLCD5110 Driver
#define GPIOR_VS1003	1	// VS1003 Driver
#define GPIOR_RF24L		1	// NRF24L01 Driver
#define GPIOR_E2PROM	1	// AT24CXX Driver
#define GPIOR_DS18B20	1	// DS18B20 Driver


//*----------------------------------------------------------------
//*		GPIOR headers
//*----------------------------------------------------------------
#include "spi.h"
#include "sd.h"
#include "i2c.h"
#include "sccb.h"
#include "e2prom.h"
#include "vs1003.h"
#include "ds18b20.h"
#include "nrf24l01.h"


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
		//GPIOR_DELAYUS(1);
		GPIOR_CLRIO(GPIOR_FREQTEST_PORT, GPIOR_FREQTEST_PIN);
		//GPIOR_DELAYUS(1);
	}
}

#endif /* if(GPIOR_TEST_FUNC) */

//*----------------------------------------------------------------
//*		GPIOR SPI PINS CONFIG
//*----------------------------------------------------------------
#if (GPIOR_SPI)

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
#define SPI_PORT    	 	GPIOR_PORT(4) 	

/* SPI #CS line */
#define SPI_NPCS0_PIN		GPIOR_PIN(2)
#define SPI_NPCS1_PIN		GPIOR_PIN(2)
#define SPI_NPCS2_PIN		GPIOR_PIN(2)
#define SPI_NPCS3_PIN		GPIOR_PIN(2)

/* SPI MOSI line */
#define SPI_MOSI_PIN		GPIOR_PIN(6)
/* SPI MISO line */
#define SPI_MISO_PIN		GPIOR_PIN(5)
/* SPI SPCK line */
#define SPI_SPCK_PIN		GPIOR_PIN(7)

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
	AM18C_BASE_SYSCFG0->PINMUX[10] &= ~(0x00FFFFFF);
	AM18C_BASE_SYSCFG0->PINMUX[10] |= (0x00888888);
#if (0)
	/* PINMUX5_15_12 GP2[12] SPI1_ENA J7 PIN1 V1.21 */
	AM18C_BASE_SYSCFG0->PINMUX[5] &= ~(0xF << 12);
	AM18C_BASE_SYSCFG0->PINMUX[5] |= (8 << 12);
	
	/* PINMUX5_7_4 GP2[14] SPI1_CS J7 PIN2 V1.21 */
	AM18C_BASE_SYSCFG0->PINMUX[5] &= ~(0xF << 4);
	AM18C_BASE_SYSCFG0->PINMUX[5] |= (8 << 4);

	/* PINMUX5_11_8 GP2[13] SPI1_CLK J7 PIN3 V1.21 */
	AM18C_BASE_SYSCFG0->PINMUX[5] &= ~(0xF << 8);
	AM18C_BASE_SYSCFG0->PINMUX[5] |= (8 << 8);
	
	/* PINMUX5_23_20 GP2[10] SPI1_MOSI J7 PIN4 V1.21 */
	AM18C_BASE_SYSCFG0->PINMUX[5] &= ~(0xF << 20);
	AM18C_BASE_SYSCFG0->PINMUX[5] |= (8 << 20);
	
	/* PINMUX5_19_16 GP2[11] SPI1_MISO J7 PIN5 V1.21 */
	AM18C_BASE_SYSCFG0->PINMUX[5] &= ~(0xF << 16);
	AM18C_BASE_SYSCFG0->PINMUX[5] |= (8 << 16);
#endif

}

/* 
** @ called at the end of SPI_close()
**/
__inline void SPI_PLAT_DEINIT(int ch) {
	AM18C_BASE_SYSCFG0->PINMUX[10] &= ~(0x00FFFFFF);
#if (0)
	AM18C_BASE_SYSCFG0->PINMUX[5] &= 
		~((0xF << 4) | (0xF << 8) | (0xF << 12) | (0xF << 16) | (0xF << 20));
#endif
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
#define SDIO_PORT    	 	GPIOR_PORT(4) 	

/* SD-BUS: CMD line */
#define SDIO_CMD_PIN		GPIOR_PIN(6)

/* SD-BUS: CLOCK line */
#define SDIO_CLK_PIN		GPIOR_PIN(7)

/* SD-BUS: D[0,3] lines */
#define SDIO_D0_PIN			GPIOR_PIN(5)
#define SDIO_D1_PIN			GPIOR_PIN(4)
#define SDIO_D2_PIN			GPIOR_PIN(3)
#define SDIO_D3_PIN			GPIOR_PIN(2)

#if !(SD_MAX_FREQ)
/* clock control value for iden mode, the bigger, the slower */
#	define SDIO_IDEN_CLKDEL_VALUE		0

/* clock control value for tran mode, the bigger, the slower */
#	define SDIO_TRAN_CLKDEL_VALUE		0
#endif

/* 
** @ called at the beginning of SD_open()
**/
__inline SDIO_PLAT_INIT(void) {
	//*	SD0_CLK	 SD0_CMD  SD0_D0  SD0_D1  SD0_D2  SD0_D3
	//*	GP4[7]   GP4[6]   GP4[5]  GP4[4]  GP4[3]  GP4[2]
	
	/* PINMUX10_3_0 SD_CLK GP4[7] */
	/* PINMUX10_7_4 SD_CMD GP4[6] */
	/* PINMUX10_11_8 SD_D0 GP4[5] */
	/* PINMUX10_15_12 SD_D1 GP4[4] */
	/* PINMUX10_19_16 SD_D2 GP4[3] */
	/* PINMUX10_23_20 SD_D3 GP4[2] */
	AM18C_BASE_SYSCFG0->PINMUX[10] &= ~(0x00FFFFFF);
	AM18C_BASE_SYSCFG0->PINMUX[10] |= (0x00888888);
}

/* 
** @ called at the end of SD_close()
**/
__inline SDIO_PLAT_DEINIT(void) {
	AM18C_BASE_SYSCFG0->PINMUX[10] &= ~(0x00FFFFFF);
}

/*
** @ read 4bits from SD[0,3]
** @ return from 0 to 1111B 
** @ optimized for different cpu 
**/
__inline unsigned char SDIO_GET_D4BIT() {
	uint8 cache = 0;
	
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
** @ optimized for different cpu
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
#define I2C_PORT		GPIOR_PORT(2) 	

/* I2C clock line */
#define I2C_SCL_PIN		GPIOR_PIN(14)

/* I2C data line */
#define I2C_SDA_PIN		GPIOR_PIN(13)

/* 
** @ called at the beginning of I2C_open()
**/
__inline I2C_PLAT_INIT(void) {
	/* PINMUX5_7_4 GP2[14] SPI1_CS J7 PIN2 V1.21 */
	AM18C_BASE_SYSCFG0->PINMUX[5] &= ~(0xF << 4);
	AM18C_BASE_SYSCFG0->PINMUX[5] |= (8 << 4);

	/* PINMUX5_11_8 GP2[13] SPI1_CLK J7 PIN3 V1.21 */
	AM18C_BASE_SYSCFG0->PINMUX[5] &= ~(0xF << 8);
	AM18C_BASE_SYSCFG0->PINMUX[5] |= (8 << 8);
}

/* 
** @ called at the beginning of close()
**/
__inline I2C_PLAT_DEINIT(void) {
	AM18C_BASE_SYSCFG0->PINMUX[5] &= ~((0xF << 4) | (0xF << 8));
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
#define LCD_PORT		GPIOR_PORT(A)

/* LCD D/C line */
#define LCD_DC_PIN		GPIOR_PIN(7)

/* LCD reset line */
#define LCD_RST_PIN		GPIOR_PIN(8)

/* 
** @ called at the beginning of LCD_open()
**/	
__inline LCD_PLAT_INIT(void) {	
}

/* 
** @ called at the beginning of LCD_open()
**/	
__inline LCD_PLAT_DEINIT(void) {	
}
#endif /* GPIOR_NKLCD */

//*----------------------------------------------------------------
//*		DS18B20 DRIVERS CONFIG
//*----------------------------------------------------------------
#if (GPIOR_SCCB)

/* DS18B20 PORT */
#define DS18B20_PORT	GPIOR_PORT(1)

/* DS18B20 1-line bus */
#define DS18B20_PIN		GPIOR_PIN(0)

/* 
** @ called at the beginning of DS_open()
**/	
__inline DS18B20_PLAT_INIT(void) {
}

/* 
** @ called at the beginning of DS_close()
**/	
__inline DS18B20_PLAT_DEINIT(void) {
}
#endif /* if (GPIOR_SCCB) */

//*----------------------------------------------------------------
//*		NRF24L01 DRIVERS CONFIG
//*----------------------------------------------------------------
#if (GPIOR_RF24L)

/* NRF24L01-SPI channel, must be one of the 4 SPI channels */
#define GPIOR_RF_CHANNEL	1

/* RF INTERRUPT MODE (nRF24L01 IRQ line IO interrupt enable) */
#define RF_INT_MODE			0

/* nRF24L01 PORT */
#define RF_PORT			GPIOR_PORT(1)

/* nRF24L01 CE line */
#define RF_CE_PIN		GPIOR_PIN(22)

/* nRF24L01 IRQ line */
#define RF_IRQ_PIN		GPIOR_PIN(23)

/* 
** @ called at the beginning of RF_open()
**/	
__inline RF_PLAT_INIT(void) {
}

/* 
** @ called at the beginning of RF_close()
**/	
__inline RF_PLAT_DEINIT(void) {
}
#endif /* if (GPIOR_RF24L) */

//*----------------------------------------------------------------
//*		VS1003(B) DRIVER CONFIG
//*----------------------------------------------------------------
#if (GPIOR_VS1003)

/* VS1003-COMMAND-SPI channel, must be one of the 4 SPI channels */
#define GPIOR_VS1003_CMDCH		1

/* VS1003-DATA-SPI channel, must be one of the 4 SPI channels */
#define GPIOR_VS1003_DATCH		2

/* VS1003 PORT */
#define VS_PORT			GPIOR_PORT(4)

/* VS1003 REST line */
#define VS_RST_PIN		GPIOR_PIN(10)

/* VS1003 DREQ line */
#define VS_DREQ_PIN		GPIOR_PIN(11)

/* 
** @ called at the beginning of VS_open()
**/	
__inline VS_PLAT_INIT(void) {
}

/* 
** @ called at the beginning of VS_close()
**/	
__inline VS_PLAT_DEINIT(void) {
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
#define GPIOR_E2PROM_WP		1

#if (GPIOR_E2PROM_WP)
/* E2PROM WP port */
#define E2PROM_WP_PORT	GPIOR_PORT(2)

/* E2PROM WP line */
#define E2PROM_WP_PIN	GPIOR_PIN(12)
#endif // if (GPIOR_E2PROM_WP)

/* 
** @ called at the beginning of E2PROM_open()
**/	
__inline E2PROMWP_PLAT_INIT(void) {
	/* PINMUX5_15_12 GP2[12] SPI1_ENA J7 PIN1 V1.21 */
	AM18C_BASE_SYSCFG0->PINMUX[5] &= ~(0xF << 12);
	AM18C_BASE_SYSCFG0->PINMUX[5] |= (8 << 12);
}

/* 
** @ called at the beginning of E2PROM_close()
**/	
__inline E2PROMWP_PLAT_DEINIT(void) {
	AM18C_BASE_SYSCFG0->PINMUX[5] &= ~(0xF << 12);
}
#endif /* if (GPIOR_E2PROM) */

#endif /* ifndef _IOCFG_H_ */
