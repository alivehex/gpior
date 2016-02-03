#ifndef _GPIOR_H_
#define _GPIOR_H_

#include <stdint.h>
#include "at91sam3u4.h"
#include "pmc/pmc.h"

//#include "uart.h"
//#include "pwm.h"
//#include "i2s.h"
//#include "spi.h"
//#include "i2c.h"
//#include "sd.h"

//#include "nrf24l01.h"
//#include "ds18b20.h"
//#include "vs1003.h"
//#include "e2prom.h"
//#include "sdspi.h"
//#include "stdio.h"
//#include "sccb.h"
//#include "lcd.h"
//#include "ff.h"

//*----------------------------------------------------------------
//*		Low-level IO functions for GPIOR drivers
//*----------------------------------------------------------------
/** @ the GPIO port macro **/
#define GPIOR_PORT(PORT)					AT91C_BASE_PIO##PORT

/** @ the GPIO pin macro **/
#define GPIOR_PIN(PIN)						(1 << PIN)

/** @ config IO input float **/
#define GPIOR_CFGIO_INPUT(PORT, PIN) \
	PORT->PIO_ODR = PIN; \
    PORT->PIO_PER = PIN
	
/** @ config IO input pull-up */
#define GPIOR_CFGIO_INPUT_PULLUP(PORT, PIN)	\
	PORT->PIO_ODR = PIN; \
    PORT->PIO_PER = PIN; \
	PORT->PIO_PPUER = PIN
	
/** @ config IO output **/
#define GPIOR_CFGIO_OUTPUT(PORT, PIN) \
	PORT->PIO_OER = PIN; \
    PORT->PIO_PER = PIN

/** @ set IO **/
#define GPIOR_SETIO(PORT, PIN) \
	PORT->PIO_SODR = PIN;

/** @ reset IO **/
#define GPIOR_CLRIO(PORT, PIN) \
	PORT->PIO_CODR = PIN;

/** @ get IO, return ZERO or NOT **/
#define GPIOR_GETIO(PORT, PIN) \
	((PORT->PIO_PDSR & (PIN)) ? 1 : 0)

/** @ delay microsecond **/
extern void gpior_delay_us(uint32_t us);
extern void gpio_led_test(void);


//*----------------------------------------------------------------
//*		GPIO LED test
//*----------------------------------------------------------------
/* GPIO max frequency */
// #define GPIOR_MAX_FREQHZ		340000
/* test frequency port */		
#define GPIOR_LED_PORT		GPIOR_PORT(A)
/* test frequency pin */
#define GPIOR_LED_PIN		GPIOR_PIN(26)

//*----------------------------------------------------------------
//*		GPIOR LOW-LEVEL SPI DRIVERS 
//*----------------------------------------------------------------
/**
SPI-Mode	CPOL	CPHA
	0		 0		 0
	1		 0		 1
	2		 1		 0
	3		 1		 1

CPOL: the base value of the clock in idle state.

CPOL = 0:
	- CPHA = 0: data are captured on the clock's rising edge (low?high transition) and data is output on a falling edge
	- CPHA = 1: data are captured on the clock's falling edge and data is output on a rising edge.
CPOL = 1:
	- CPHA = 0: data are captured on clock's falling edge and data is output on a rising edge.
	- CPHA = 1: data are captured on clock's rising edge and data is output on a falling edge.
**/
#define SPI_MODE		0

#if (SPI_MODE==0)
# define SPI_CPOL		0
# define SPI_CPHA		0
#elif (SPI_MODE==1)
# define SPI_CPOL		0
# define SPI_CPHA		1
#elif (SPI_MODE==2)
# define SPI_CPOL		1
# define SPI_CPHA		0
#elif (SPI_MODE==3)
# define SPI_CPOL		1
# define SPI_CPHA		1
#endif

/* Fixed 4 channels SPI mode */
#define SPI_TOTAL_CHANNELS				4

/* Ignore the SPCK frequency setting, go as far as we could */
#define SPI_IGNORE_FREQ_SETTING			0

#define SPI_PORT    	 	GPIOR_PORT(A) 

/* SPI #CS line */
#define SPI_NPCS0_PIN		GPIOR_PIN(8)
#define SPI_NPCS1_PIN		GPIOR_PIN(9)
#define SPI_NPCS2_PIN		GPIOR_PIN(10)
#define SPI_NPCS3_PIN		GPIOR_PIN(11)
/* SPI MOSI line */
#define SPI_MOSI_PIN		GPIOR_PIN(4)
/* SPI MISO line */
#define SPI_MISO_PIN		GPIOR_PIN(5)
/* SPI SPCK line */
#define SPI_SPCK_PIN		GPIOR_PIN(3)

/* 
** @ called at the beginning of SPI_open()
**/
inline static void spi_lowlevel_init(int ch)
{
	PMC_EnablePeripheral(AT91C_ID_PIOA);
}

/* 
** @ called at the end of SPI_close()
**/
inline static void spi_lowlevel_deinit(int ch)
{
	
}

#if (0)
//*----------------------------------------------------------------
//*		SD-CARD (NOT SPI MODE)
//*----------------------------------------------------------------
#define GPIOR_SD_CH			0 
#define SDIO_PORT    	 	GPIOR_PORT(A) 	
/* SD-BUS: CMD line */
#define SDIO_CMD_PIN		GPIOR_PIN(5)
/* SD-BUS: CLOCK line */
#define SDIO_CLK_PIN		GPIOR_PIN(4)
/* SD-BUS: D[0,3] lines */
#define SDIO_D0_PIN			GPIOR_PIN(0)
#define SDIO_D1_PIN			GPIOR_PIN(1)
#define SDIO_D2_PIN			GPIOR_PIN(2)
#define SDIO_D3_PIN			GPIOR_PIN(3)
/* clock control value for iden mode, with GPIOR_DELAYUS() */
#define SDIO_IDEN_CLKDEL_VALUE		2
/* clock control value for tran mode, with GPIOR_DELAYUS() */
#define SDIO_TRAN_CLKDEL_VALUE		1

/* 
 * @ any platform init should be here
 * @ called at the beginning of sd_gpio_init()
 */
__inline void SDIO_PLAT_INIT(void) {
	
}

/* 
 * @ read 4bits from SD[0,3]
 * @ can be optimized for different platform 
 */
__inline uint8 SDIO_GET_D4BIT() {
	return GPIOR_GETIO(SDIO_PORT, (SDIO_D0_PIN | SDIO_D1_PIN | SDIO_D2_PIN | SDIO_D3_PIN));
}

/* 
 * @ write 4bits to SD[0,3]
 * @ can be optimized for different platform 
 */
__inline void SDIO_SET_D4BIT(uint8 data) {
	GPIOR_CLRIO(SDIO_PORT, ~data);
	GPIOR_SETIO(SDIO_PORT, data);
}

//*----------------------------------------------------------------
//*		I2C
//*----------------------------------------------------------------
#define I2C_PORT		GPIOR_PORT(A) 	
/* I2C clock line */
#define I2C_SCL_PIN		GPIOR_PIN(10)
/* I2C data line */
#define I2C_SDA_PIN		GPIOR_PIN(11)

/* 
 * @ any platform init should be here
 * @ called at the beginning of i2c_gpio_init()
 */
__inline I2C_PLAT_INIT(void) {	 
}


//*----------------------------------------------------------------
//*		UART
//*----------------------------------------------------------------
/* UART TXD and RXD */
#define UART_PORT		GPIOR_PORT(A)
#define UART_TXD_PIN	GPIOR_PIN(31)
#define UART_RXD_PIN	GPIOR_PIN(30)

// UART Baudrate
#define UART_BAUDRATE	9600

/* 
 * @ any platform init should be here
 * @ called at the beginning of uart_gpio_init()
 */	
__inline void UART_PLAT_INIT(void) {
}

//*----------------------------------------------------------------
//*		PWM 
//*----------------------------------------------------------------
#define PWM_PORT		GPIOR_PORT(A)
#define PWM_PIN			GPIOR_PIN(0)

/* 
 * @ any platform init should be here
 * @ called at pwm_open()
 */	
__inline void PWM_PLAT_INIT(void) {	
}

//*----------------------------------------------------------------
//*		I2S 
//*----------------------------------------------------------------  
//	I2S type
//		<0=> LEFT-ALIGN	 <1=> PHILIPS
//		<2=> RIGHT-ALIGN
#define GPIOR_I2S_TYPE	0
#define I2S_PORT		GPIOR_PORT(B)
#define I2S_WS_PIN		GPIOR_PIN(0)
#define I2S_SDO_PIN		GPIOR_PIN(1)
#define I2S_SCK_PIN		GPIOR_PIN(2)

/* 
 * @ any platform init should be here
 * @ called at i2s_open()
 */	
__inline void I2S_PLAT_INIT(void) {
}

// END of GPIOR low-level driver settings

// GPIOR DRIVERS

//*----------------------------------------------------------------
//*			SD-SPI mode driver
//*----------------------------------------------------------------
//	SD-SPI channel
//	<0=> SPI channel 0	<1=> SPI channel 1
//	<2=> SPI channel 2	<3=> SPI channel 3
#define GPIOR_SD_CH		0

//*----------------------------------------------------------------
//*		UART stdout
//*----------------------------------------------------------------
// 	UART stdout channel <0-1>
#define GPIOT_STDOUT_UARTCH			0

//*----------------------------------------------------------------
//*		NOKIA LCD5110 84 * 48 LCD 
//*----------------------------------------------------------------
//	LCD-SPI channel
//	<0=> SPI channel 0	<1=> SPI channel 1
//	<2=> SPI channel 2	<3=> SPI channel 3
#define GPIOR_LCD_CH 		0
//	<q> LCD 8 * 5 Bitmap Fonts
#define GPIOR_LCD_BITMAP	1
//	<q> LCD icons
#define GPIOR_LCD_ICONS		1

#define LCD_PORT		GPIOR_PORT(A)
/* LCD D/C line */
#define LCD_DC_PIN		GPIOR_PIN(7)
/* LCD reset line */
#define LCD_RST_PIN		GPIOR_PIN(8)
/* 
 * @ any platform init should be here
 * @ called at the beginning of lcd_gpio_init()
 */	
__inline void LCD_PLAT_INIT(void) {	
}

//*----------------------------------------------------------------
//*		DS18B20 1-Wire Digital Thermometer
//*----------------------------------------------------------------
#define DS18B20_PORT	GPIOR_PORT(A)
/* DS18B20 1BIT bus */
#define DS18B20_PIN		GPIOR_PIN(0)
/* 
 * @ any platform init should be here
 * @ called at the beginning of ds_gpio_init()
 */	
__inline void DS18B20_PLAT_INIT(void) {
}

//*----------------------------------------------------------------
//*		NRF24L01 Single Chip 2.4GHz Transceiver
//*----------------------------------------------------------------
//	RF-SPI channel
//	<0=> SPI channel 0	<1=> SPI channel 1
//	<2=> SPI channel 2	<3=> SPI channel 3
#define GPIOR_RF_CH		0
//	RF INTERRUPT MODE
#define RF_INT_MODE		0
//	nRF24L01 IRQ line IO interrupt enable
#define RF_PORT			GPIOR_PORT(A)
/* nRF24L01 CE line */
#define RF_CE_PIN		GPIOR_PIN(22)
/* nRF24L01 IRQ line */
#define RF_IRQ_PIN		GPIOR_PIN(23)
/* 
 * @ any platform init should be here
 * @ called at the end of NRF_IO_INIT()
 */	
__inline void RF_PLAT_INIT(void) {
}

//*----------------------------------------------------------------
//*		VS1003(B) MP3/WMA AUDIO CODEC
//*----------------------------------------------------------------
//	VS1003-SPI command channel
//	<0=> SPI channel 0	<1=> SPI channel 1
//	<2=> SPI channel 2	<3=> SPI channel 3
#define GPIOR_VS1003_CMDCH		1
//	<o> VS1003-SPI data channel
//	<0=> SPI channel 0	<1=> SPI channel 1
//	<2=> SPI channel 2	<3=> SPI channel 3
#define GPIOR_VS1003_DATCH		2

#define VS_PORT			GPIOR_PORT(A)
/* reset line */
#define VS_RST_PIN		GPIOR_PIN(22)
/* irq line */
#define VS_DREQ_PIN		GPIOR_PIN(23)
/* 
 * @ any platform init should be here
 * @ called at the end of VS_IO_INIT()
 */	
__inline void VS_PLAT_INIT(void) {	
}

//*----------------------------------------------------------------
//*		AT24CXX like EEPROM
//*---------------------------------------------------------------- 
//	<q> Write Protect for E2POM
#define GPIOR_EEPROM_WP	1
#define EEPROM_WP_PORT	GPIOR_PORT(A)
#define EEPROM_WP_PIN	GPIOR_PIN(2)
/* 
 * @ any platform init should be here
 * @ called at the end of e2prom_open()
 */	
__inline void EEPROM_WPPLAT_INIT(void) {	
}

// END of GPIOR DRIVERS

#endif // ifndef _GPIOR_H_

#endif // if (0)
