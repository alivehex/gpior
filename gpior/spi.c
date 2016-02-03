#include "gpior.h"
#include "spi.h"

#if (SPI_IGNORE_FREQ_SETTING)
# define spi_delay_us(n)	while (0)
#else
static uint32_t spi_freq_delay[SPI_TOTAL_CHANNELS] = {0};
# define spi_delay_us(n)	gpior_delay_us(n)
#endif

#define spi_output_bit() \
	if(byte & 0x80) { \
		GPIOR_SETIO(SPI_PORT, SPI_MOSI_PIN); \
	} else { \
		GPIOR_CLRIO(SPI_PORT, SPI_MOSI_PIN); \
	} \
	byte <<= 1; \
	spi_delay_us(spi_freq_delay[ch])
	
#define spi_input_bit() \
	buf <<= 1; \
	if(GPIOR_GETIO(SPI_PORT, SPI_MISO_PIN)) { \
		buf |= 1; \
	} \
	spi_delay_us(spi_freq_delay[ch])

// mark for spi slave select enable
static uint8_t spi_ss_mask = 0;
		
/*
 * @ config gpio input mode, etc...
 */
//static void SPI_GPIO_INIT(int ch) {
static void spi_gpio_init(SpiSlaveSelect ch) {
 	spi_lowlevel_init(ch);
	
	GPIOR_CFGIO_OUTPUT(SPI_PORT, SPI_MOSI_PIN);
	GPIOR_CFGIO_OUTPUT(SPI_PORT, SPI_SPCK_PIN);
	GPIOR_CFGIO_INPUT_PULLUP(SPI_PORT, SPI_MISO_PIN);
	
	if (ch == SPI_SS_0) {
		GPIOR_CFGIO_OUTPUT(SPI_PORT, SPI_NPCS0_PIN);
	} else if (ch == SPI_SS_1) {
		GPIOR_CFGIO_OUTPUT(SPI_PORT, SPI_NPCS1_PIN);
	} else if (ch == SPI_SS_2) {
		GPIOR_CFGIO_OUTPUT(SPI_PORT, SPI_NPCS2_PIN);
	} else if (ch == SPI_SS_3) {
		GPIOR_CFGIO_OUTPUT(SPI_PORT, SPI_NPCS3_PIN);
	}
	
	/* disable spi channel */
	spi_ss_high(ch);
	
	/* spi clock line level in idle mode */
#if (SPI_CPOL==1)	
	GPIOR_SETIO(SPI_PORT, SPI_SPCK_PIN);
#else				
	GPIOR_CLRIO(SPI_PORT, SPI_SPCK_PIN);
#endif
}

/*
 * @ open SPI
 */
//int SPI_open(int ch, unsigned int clock_freq_hz) {
void spi_open(SpiSlaveSelect ch, uint32_t freq_hz) {
	spi_gpio_init(ch);
	
#if !(SPI_IGNORE_FREQ_SETTING)
	spi_freq_delay[ch] = 1000000 / freq_hz;
#endif
	
	spi_ss_mask |= (1 << ch);
}

/*
** @ Active the SPI CS line
**/
void spi_ss_low(SpiSlaveSelect ch) {
	if (ch == SPI_SS_0) {
		GPIOR_CLRIO(SPI_PORT, SPI_NPCS0_PIN);
	} else if (ch == SPI_SS_1) {
		GPIOR_CLRIO(SPI_PORT, SPI_NPCS1_PIN);
	} else if (ch == SPI_SS_2) {
		GPIOR_CLRIO(SPI_PORT, SPI_NPCS2_PIN);
	} else if (ch == SPI_SS_3) {
		GPIOR_CLRIO(SPI_PORT, SPI_NPCS3_PIN);
	}
}


/*
** @ Inactive the SPI CS line
**/
void spi_ss_high(SpiSlaveSelect ch) {
	if (ch == SPI_SS_0) {
		GPIOR_SETIO(SPI_PORT, SPI_NPCS0_PIN);
	} else if (ch == SPI_SS_1) {
		GPIOR_SETIO(SPI_PORT, SPI_NPCS1_PIN);
	} else if (ch == SPI_SS_2) {
		GPIOR_SETIO(SPI_PORT, SPI_NPCS2_PIN);
	} else if (ch == SPI_SS_3) {
		GPIOR_SETIO(SPI_PORT, SPI_NPCS3_PIN);
	}
}

/*
 * @ software SPI write
 * @ return read value
 */
uint8_t spi_write_byte(SpiSlaveSelect ch, uint8_t byte) {
	uint8_t buf = 0;
	uint8_t i;

	for(i = 0; i < 8; i++) {
#if (SPI_CPOL==1)		/* clock low in idle */
# if (SPI_CPHA==1) 		/* captured on rising, output on falling */
		/** SPI MODE 3 **/
		
		GPIOR_CLRIO(SPI_PORT, SPI_SPCK_PIN);
		/* output to MOSI line, MSB format */
		spi_output_bit();
		GPIOR_SETIO(SPI_PORT, SPI_SPCK_PIN);
		/* input from MISO line, MSB format */
		spi_input_bit();
		
# elif (SPI_CPHA==0) 	/* captured on falling, output on rising */
		/** SPI MODE 2 **/
		
		spi_output_bit();
		GPIOR_CLRIO(SPI_PORT, SPI_SPCK_PIN);
		spi_input_bit();	
		GPIOR_SETIO(SPI_PORT, SPI_SPCK_PIN);
		
# endif	// if (SPI_CPHA==1)	
#elif (SPI_CPOL==0)		/* clock low in idle */
# if (SPI_CPHA==1)		/* captured on falling, output on rising */
		/** SPI MODE 1 **/
		
		GPIOR_SETIO(SPI_PORT, SPI_SPCK_PIN);
		spi_output_bit();
		GPIOR_CLRIO(SPI_PORT, SPI_SPCK_PIN);
		spi_input_bit();		
		
# elif (SPI_CPHA==0)	/* captured on rising, output on falling */
		/** SPI MODE 0 **/
		
		spi_output_bit();
		GPIOR_SETIO(SPI_PORT, SPI_SPCK_PIN);
		spi_input_bit();
		GPIOR_CLRIO(SPI_PORT, SPI_SPCK_PIN);
			
# endif // if (SPI_CPHA==1)		
#endif  // if (SPI_CPOL==1)
	}
	return buf;
}

/** Write buffer **/
void spi_write(SpiSlaveSelect ch, uint8_t *output, uint8_t* input, int length) {
	for(int i = 0; i < length; i ++) {
		if (input) {
			if (output) {
				input[i] = spi_write_byte(ch, output[i]);
			} else {
				input[i] = spi_write_byte(ch, 0xff);
			}
		} else {
			if (output) {
				spi_write_byte(ch, output[i]);
			} else {
				spi_write_byte(ch, 0xff);
			}
		}
	}
}

/*
 * @ SPI close
 */	
void spi_close(SpiSlaveSelect ch) {	
	if (ch == SPI_SS_0) {
		GPIOR_CFGIO_INPUT(SPI_PORT, SPI_NPCS0_PIN);
	} else if (ch == SPI_SS_1) {
		GPIOR_CFGIO_INPUT(SPI_PORT, SPI_NPCS1_PIN);
	} else if (ch == SPI_SS_2) {
		GPIOR_CFGIO_INPUT(SPI_PORT, SPI_NPCS2_PIN);
	} else if (ch == SPI_SS_3) {
		GPIOR_CFGIO_INPUT(SPI_PORT, SPI_NPCS3_PIN);
	}
	
	spi_ss_mask &= ~(1 << ch);
	
	if(spi_ss_mask == 0) {
		GPIOR_CFGIO_INPUT(SPI_PORT, SPI_MOSI_PIN);
		GPIOR_CFGIO_INPUT(SPI_PORT, SPI_MISO_PIN);
		GPIOR_CFGIO_INPUT(SPI_PORT, SPI_SPCK_PIN);
		
		spi_lowlevel_deinit(ch);
	}
}

