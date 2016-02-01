#ifndef _GPIOR_H_
#define _GPIOR_H_


#include "iocfg.h"



// <<< Use Configuration Wizard in Context Menu >>>

//*----------------------------------------------------------------
//¡Á		TARGET DEFINED
//*----------------------------------------------------------------
// <h> TARGET
// 	<o> CPU
//	<0=> AT91RM9200
//	<1=> AM1808
//	<2=> STM32F107
#define GPIOR_CPU		1
// </h>

// PLAT headers
#if 	(GPIOR_CPU == 0)
#	include "gpior_at91.h"
#elif 	(GPIOR_CPU == 1)
#	include "gpior_am18.h"
#elif	(GPIOR_CPU == 2)
#	include "gpior_stm32.h"
#endif


//*----------------------------------------------------------------
//*		common functions for everybody
//*----------------------------------------------------------------
/* GPIO max frequency */
#define GPIOR_MAX_FREQHZ		340000
/* test frequency port */		
#define GPIOR_FREQTEST_PORT		GPIOR_PORT(A)
/* test frequency pin */
#define GPIOR_FREQTEST_PIN		GPIOR_PIN(0)

/*
 * @ get GPIOR max IO freqency: GPIOR_MAX_FREQHZ
 * @ run this function and get value with a frequency counter
 * @ MAX IO output freq for AT91RM9200 is 1.79MHZ
 */
__inline void _gpio_freq_rest(void) {
	/* any platform code should be here */

	/* config IO output */
	GPIOR_CFGIO_OUTPUT(GPIOR_FREQTEST_PORT, GPIOR_FREQTEST_PIN);
	while(1) {
	 	GPIOR_SETIO(GPIOR_FREQTEST_PORT, GPIOR_FREQTEST_PIN);
		GPIOR_DELAYUS(2); // 1.5us test
		GPIOR_CLRIO(GPIOR_FREQTEST_PORT, GPIOR_FREQTEST_PIN);
		GPIOR_DELAYUS(2);
	}
}

// <h> GPIOR LOW-LEVEL DRIVERS

//*----------------------------------------------------------------
//*		SPI
//*----------------------------------------------------------------
// <e> SPI
#define GPIOR_SPI			0

#if (GPIOR_SPI)
// <o> 	GPIOR SPI max channel <0-8>
#define GPIOR_SPI_MAXCH		8
// </e>

#define SPI_PORT    	 	GPIOR_PORT(A) 	
/* SPI #CS line */
#if (GPIOR_SPI_MAXCH > 0)
#define SPI_NPCS0_PIN		GPIOR_PIN(3)
#endif
#if (GPIOR_SPI_MAXCH > 1)
#define SPI_NPCS1_PIN		GPIOR_PIN(4)
#endif
#if (GPIOR_SPI_MAXCH > 2)
#define SPI_NPCS2_PIN		GPIOR_PIN(5)
#endif
#if (GPIOR_SPI_MAXCH > 3)
#define SPI_NPCS3_PIN		GPIOR_PIN(6)
#endif
#if (GPIOR_SPI_MAXCH > 4)
#define SPI_NPCS4_PIN		GPIOR_PIN(6)
#endif
#if (GPIOR_SPI_MAXCH > 5)
#define SPI_NPCS5_PIN		GPIOR_PIN(6)
#endif
#if (GPIOR_SPI_MAXCH > 6)
#define SPI_NPCS6_PIN		GPIOR_PIN(6)
#endif
#if (GPIOR_SPI_MAXCH > 7)
#define SPI_NPCS7_PIN		GPIOR_PIN(6)
#endif

/* SPI MOSI line */
#define SPI_MOSI_PIN		GPIOR_PIN(7)
/* SPI MISO line */
#define SPI_MISO_PIN		GPIOR_PIN(6)
/* SPI SPCK line */
#define SPI_SPCK_PIN		GPIOR_PIN(5)
/*
 * @ FIXED 4 channels
 */
__inline uint32 SPI_NPCS_PIN(int ch) {
	switch(ch) {
#if (GPIOR_SPI_MAXCH > 0)
	case 0: return SPI_NPCS0_PIN;
#endif
#if (GPIOR_SPI_MAXCH > 1)
	case 1: return SPI_NPCS1_PIN;
#endif
#if (GPIOR_SPI_MAXCH > 2)
	case 2: return SPI_NPCS2_PIN;
#endif
#if (GPIOR_SPI_MAXCH > 3)
	case 3: return SPI_NPCS3_PIN;
#endif
#if (GPIOR_SPI_MAXCH > 4)
	case 4: return SPI_NPCS4_PIN;
#endif
#if (GPIOR_SPI_MAXCH > 5)
	case 5: return SPI_NPCS5_PIN;
#endif
#if (GPIOR_SPI_MAXCH > 6)
	case 6: return SPI_NPCS6_PIN;
#endif
#if (GPIOR_SPI_MAXCH > 7)
	case 7: return SPI_NPCS7_PIN;
#endif
	default: return SPI_NPCS0_PIN;
	}
}
/* 
 * @ any platform init should be here
 * @ called at the beginning of sd_gpio_init()
 */
__inline void SPI_PLAT_INIT(int ch) {
#if 	(GPIOR_CPU == 0)
	/* input-filter enable */
	AT91F_PIO_CfgInputFilter(SPI_PORT, SPI_MISO_PIN);
	/* pull-up enable */
	AT91F_PIO_CfgPullup(SPI_PORT, (SPI_NPCS_PIN(ch) | SPI_MOSI_PIN | SPI_MISO_PIN | SPI_SPCK_PIN));
#elif 	(GPIOR_CPU == 1)
	
#elif	(GPIOR_CPU == 2)
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	/* NPCS */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* SPCK */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* MISO */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* MOSI */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_WriteBit(GPIOA, GPIO_Pin_4, Bit_SET);
#endif 
}

#include "spi.h"
#endif // if (GPIOR_SPI)

//*----------------------------------------------------------------
//*		SD-CARD (NOT SPI MODE)
//*----------------------------------------------------------------
// <q> SD-CARD (NOT SPI MODE)
#define GPIOR_SD			0

#if (GPIOR_SD)

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
__inline SDIO_PLAT_INIT(void) {
#if 	(GPIOR_CPU == 0)
	/* input-filter enable */
	AT91F_PIO_CfgInputFilter(SDIO_PORT, (SDIO_D0_PIN | SDIO_D1_PIN | SDIO_D2_PIN | SDIO_D3_PIN));
	/* pull-up enable */
	AT91F_PIO_CfgPullup(SDIO_PORT, 
		(SDIO_CMD_PIN | SDIO_CLK_PIN | SDIO_D0_PIN | SDIO_D1_PIN | SDIO_D2_PIN | SDIO_D3_PIN));	
#elif 	(GPIOR_CPU == 1)

#elif	(GPIOR_CPU == 2)

#endif	
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

#include "sd.h"
#endif // if((GPIOR_SD)

//*----------------------------------------------------------------
//*		I2C
//*----------------------------------------------------------------
// <q> I2C
#define GPIOR_I2C		1

#if (GPIOR_I2C)

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
#if 	(GPIOR_CPU == 0)
	AT91F_PIO_CfgPullup(I2C_PORT, (I2C_SCL_PIN | I2C_SDA_PIN));
#elif 	(GPIOR_CPU == 1)
	
#elif	(GPIOR_CPU == 2)
	GPIO_InitTypeDef GPIO_InitStructure;
	
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
	
	/* SLK */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	/* SDA */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_WriteBit(GPIOE, GPIO_Pin_10, Bit_SET);
	GPIO_WriteBit(GPIOE, GPIO_Pin_11, Bit_SET);
#endif	 
}

#include "i2c.h"
#endif // #if (GPIOR_I2C)

//*----------------------------------------------------------------
//*		UART
//*----------------------------------------------------------------
// <e> UART
#define GPIOR_UART		0

#if (GPIOR_UART)
/* UART TXD and RXD */
#define UART_PORT		GPIOR_PORT(A)
#define UART_TXD_PIN	GPIOR_PIN(31)
#define UART_RXD_PIN	GPIOR_PIN(30)

// <o> UART Baudrate
//	<4800=> 4800	<9600=> 9600
#define UART_BAUDRATE	9600
// </e>

/* 
 * @ any platform init should be here
 * @ called at the beginning of uart_gpio_init()
 */	
__inline UART_PLAT_INIT(void) {
}

#include "uart.h"
#endif // if (GPIOR_UART)

//*----------------------------------------------------------------
//*		PWM 
//*----------------------------------------------------------------
// <q> PWM
#define GPIOR_PWM	0

#if (GPIOR_PWM)

#define PWM_PORT		GPIOR_PORT(A)
#define PWM_PIN			GPIOR_PIN(0)

/* 
 * @ any platform init should be here
 * @ called at pwm_open()
 */	
__inline PWM_PLAT_INIT(void) {
#if 	(GPIOR_CPU == 0)
	AT91F_PIO_CfgPullup(PWM_PORT, PWM_PIN);
#elif 	(GPIOR_CPU == 1)

#elif	(GPIOR_CPU == 2)

#endif	
}

#include "pwm.h"
#endif // if (GPIOR_PWM)

//*----------------------------------------------------------------
//*		I2S 
//*----------------------------------------------------------------
// <e> I2S   
#define GPIOR_I2S		0
//	<o> I2S type
//		<0=> LEFT-ALIGN	 <1=> PHILIPS
//		<2=> RIGHT-ALIGN
#define GPIOR_I2S_TYPE	0
// </e>

#if (GPIOR_I2S)
#define I2S_PORT		GPIOR_PORT(B)
#define I2S_WS_PIN		GPIOR_PIN(0)
#define I2S_SDO_PIN		GPIOR_PIN(1)
#define I2S_SCK_PIN		GPIOR_PIN(2)

/* 
 * @ any platform init should be here
 * @ called at i2s_open()
 */	
__inline I2S_PLAT_INIT(void) {
#if 	(GPIOR_CPU == 0)
	AT91F_PIO_CfgPullup(I2S_PORT, (I2S_WS_PIN | I2S_SDO_PIN | I2S_SCK_PIN));
#elif 	(GPIOR_CPU == 1)

#elif	(GPIOR_CPU == 2)

#endif
}

#include "i2s.h" 
#endif // if (GPIOR_I2S)

// </h> END of GPIOR low-level driver settings

// <h> GPIOR DRIVERS

//*----------------------------------------------------------------
//*		SD-SPI
//*----------------------------------------------------------------
// 	<e> SD-SPI
#define GPIOR_SDSPI		0

#if	(GPIOR_SDSPI)
// 	<i> SD-SPI mode driver
//	<o> SD-SPI channel
//	<0=> SPI channel 0	<1=> SPI channel 1
//	<2=> SPI channel 2	<3=> SPI channel 3
#define GPIOR_SD_CH		0

#include "sdspi.h"
#endif // if (GPIOR_SDSPI)
//	</e>

//*----------------------------------------------------------------
//*		FATFS
//*----------------------------------------------------------------
// 	<e> FATFS
#define GPIOR_FATFS		0
// 	<i> FATFS on SD-CARD

#if (GPIOR_FATFS)
#include "ff.h"
#endif
//	</e>


//*----------------------------------------------------------------
//*		STDOUT
//*----------------------------------------------------------------
//	<e> UART stdout
#define GPIOR_UART_STDOUT	0

#if (GPIOR_UART_STDOUT)
// 	<o>	UART stdout channel <0-1>
#define GPIOT_STDOUT_UARTCH			0

#include "stdio.h"
#endif 
//	</e>

//*----------------------------------------------------------------
//*		LCD5110
//*----------------------------------------------------------------
//	<e> NOKIA LCD5110
#define GPIOR_LCD5510		0
//	<i> 84 * 48 LCD 

#if (GPIOR_LCD5510)
//	<o> LCD-SPI channel
//	<0=> SPI channel 0	<1=> SPI channel 1
//	<2=> SPI channel 2	<3=> SPI channel 3
#define GPIOR_LCD_CH 		0
//	<q> LCD 8 * 5 Bitmap Fonts
#define GPIOR_LCD_BITMAP	1
//	<q> LCD icons
#define GPIOR_LCD_ICONS		1
//	</e>

#define LCD_PORT		GPIOR_PORT(A)
/* LCD D/C line */
#define LCD_DC_PIN		GPIOR_PIN(7)
/* LCD reset line */
#define LCD_RST_PIN		GPIOR_PIN(8)
/* 
 * @ any platform init should be here
 * @ called at the beginning of lcd_gpio_init()
 */	
__inline LCD_PLAT_INIT(void) {
#if 	(GPIOR_CPU == 0)
	AT91F_PIO_CfgPullup(LCD_PORT, LCD_DC_PIN | LCD_RST_PIN);
#elif 	(GPIOR_CPU == 1)

#elif	(GPIOR_CPU == 2)

#endif	
}

#include "lcd.h"
#endif // if (GPIOR_LCD5510)

//*----------------------------------------------------------------
//*		DS18B20 IO settings
//*----------------------------------------------------------------
//	<q> DS18B20
#define GPIOR_DS18B20	0
//	<i> 1-Wire Digital Thermometer

#if (GPIOR_DS18B20)
#define DS18B20_PORT	GPIOR_PORT(A)
/* DS18B20 1BIT bus */
#define DS18B20_PIN		GPIOR_PIN(0)
/* 
 * @ any platform init should be here
 * @ called at the beginning of ds_gpio_init()
 */	
__inline DS18B20_PLAT_INIT(void) {
#if 	(GPIOR_CPU == 0)
	 AT91F_PIO_CfgPullup(DS18B20_PORT, DS18B20_PIN);
#elif 	(GPIOR_CPU == 1)

#elif	(GPIOR_CPU == 2)

#endif
}

#include "ds18b20.h"
#endif // if (GPIOR_DS18B20)

//*----------------------------------------------------------------
//*		NRF24L01 IO settings
//*----------------------------------------------------------------
//	<e> nRF24L01(+)
#define GPIOR_NRF24L01	0

//	<i> Single Chip 2.4GHz Transceiver
#if (GPIOR_NRF24L01)
//	<o> RF-SPI channel
//	<0=> SPI channel 0	<1=> SPI channel 1
//	<2=> SPI channel 2	<3=> SPI channel 3
#define GPIOR_RF_CH		0
//	<q> RF INTERRUPT MODE
#define RF_INT_MODE		0
//	<i> nRF24L01 IRQ line IO interrupt enable
//	</e>

#define RF_PORT		GPIOR_PORT(A)
/* nRF24L01 CE line */
#define RF_CE_PIN		GPIOR_PIN(22)
/* nRF24L01 IRQ line */
#define RF_IRQ_PIN		GPIOR_PIN(23)
/* 
 * @ any platform init should be here
 * @ called at the end of NRF_IO_INIT()
 */	
__inline RF_PLAT_INIT(void) {
#if 	(GPIOR_CPU == 0)
	AT91F_PIO_CfgPullup(RF_PORT, RF_CE_PIN | RF_IRQ_PIN);
#elif 	(GPIOR_CPU == 1)

#elif	(GPIOR_CPU == 2)

#endif
}

#include "nrf24l01.h"
#endif // if(GPIOR_NRF24L01)

//*----------------------------------------------------------------
//*		VS1003(B) IO settings
//*----------------------------------------------------------------
//	<e> VS1003(B)
#define GPIOR_VS1003	0
//	<i> MP3/WMA AUDIO CODEC

#if (GPIOR_VS1003)
//	<o> VS1003-SPI command channel
//	<0=> SPI channel 0	<1=> SPI channel 1
//	<2=> SPI channel 2	<3=> SPI channel 3
#define GPIOR_VS1003_CMDCH		1
//	<o> VS1003-SPI data channel
//	<0=> SPI channel 0	<1=> SPI channel 1
//	<2=> SPI channel 2	<3=> SPI channel 3
#define GPIOR_VS1003_DATCH		2
//	</e>

#define VS_PORT			GPIOR_PORT(A)
/* nRF24L01 CE line */
#define VS_RST_PIN		GPIOR_PIN(22)
/* nRF24L01 IRQ line */
#define VS_DREQ_PIN		GPIOR_PIN(23)
/* 
 * @ any platform init should be here
 * @ called at the end of VS_IO_INIT()
 */	
__inline VS_PLAT_INIT(void) {
#if 	(GPIOR_CPU == 0)
	AT91F_PIO_CfgPullup(VS_PORT, (VS_DREQ_PIN | VS_RST_PIN));
	AT91F_PIO_CfgInputFilter(VS_PORT, VS_DREQ_PIN);
#elif 	(GPIOR_CPU == 1)

#elif	(GPIOR_CPU == 2)

#endif	
}

#include "vs1003.h"
#endif // if (GPIOR_VS1003)

//*----------------------------------------------------------------
//*		SCCB 
//*----------------------------------------------------------------
//	<q> SCCB
#define GPIOR_SCCB		1
// 	<i>	Serial Camera Control Bus

#if (GPIOR_SCCB)
#include "sccb.h"
#endif // if (GPIOR_SCCB)

//*----------------------------------------------------------------
//*		EEPROM IO settings 
//*----------------------------------------------------------------
//	<e> EEPROM (AT24CXX)
#define GPIOR_EEPROM	0

#if (GPIOR_EEPROM)
// 	<i> AT24CXX like EEPROM
//	<q> Write Protect for E2POM
#define GPIOR_EEPROM_WP	1
//	</e>

#if (GPIOR_EEPROM_WP)

#define EEPROM_WP_PORT	GPIOR_PORT(A)
#define EEPROM_WP_PIN	GPIOR_PIN(2)
#endif // if (GPIOR_EEPROM_WP)
/* 
 * @ any platform init should be here
 * @ called at the end of e2prom_open()
 */	
__inline EEPROM_WPPLAT_INIT(void) {
#if 	(GPIOR_CPU == 0)
	AT91F_PIO_CfgPullup(EEPROM_WP_PORT, EEPROM_WP_PIN);
#elif 	(GPIOR_CPU == 1)

#elif	(GPIOR_CPU == 2)

#endif	
}

#include "e2prom.h"
#endif // if (GPIOR_EEPROM)

//</h> END of GPIOR DRIVERS

#endif // ifndef _GPIOR_H_
