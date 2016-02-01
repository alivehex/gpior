#include "nrf24l01.h"

#if (GPIOR_SPI)

#if (1)
void SPI_GPIOR_DELAYUS(volatile int t) {
	while (t --);
}
#endif


#define GPIOR_SPI_MAXCH		4

static int SPI_CHEN[GPIOR_SPI_MAXCH] = {0};

#if !(SPI_MAX_FREQ)
static int SPI_CLK_DELAY[GPIOR_SPI_MAXCH] = {0};
static void (*SPI_DELAYUS)(int) = NULL;
static void SPI_DUMMY(int t) {
	while (t --);
}
#else
#	define SPI_DELAYUS(us)	while (0)
#endif /* if !(SPI_MAX_FREQ) */

/*
 * @ check if all channels are closed
 */
static __inline int IS_SPI_ALLCH_DISABLED() {
 	int i;

	for(i = 0; i < GPIOR_SPI_MAXCH; i ++) {
	 	if(SPI_CHEN[i]) {
			return FALSE;
		}
	}
	return TRUE; 
}

/*
 * @ config gpio input mode, etc...
 */
static void SPI_GPIO_INIT(int ch) {
 	SPI_PLAT_INIT(ch);
	if(IS_SPI_ALLCH_DISABLED()) {
		GPIOR_CFGIO_OUTPUT(SPI_PORT, SPI_MOSI_PIN);
		GPIOR_CFGIO_OUTPUT(SPI_PORT, SPI_SPCK_PIN);
		GPIOR_CFGIO_INPUT(SPI_PORT, SPI_MISO_PIN);
	}
	GPIOR_CFGIO_OUTPUT(SPI_PORT, SPI_NPCS_PIN(ch));
	
#if (SPI_CS) /* SPI_CS_ACTIVE_HIGH */
	GPIOR_CLRIO(SPI_PORT, SPI_NPCS_PIN(ch));
#else 		 /* SPI_CS_ACTIVE_LOW */
	GPIOR_SETIO(SPI_PORT, SPI_NPCS_PIN(ch));
#endif /* if (SPI_CS) */
	
#if (SPI_CPOL)	/* SPI_CLOCK_INACTIVE_HIGH */
	GPIOR_SETIO(SPI_PORT, SPI_SPCK_PIN);
#else			/* SPI_CLOCK_INACTIVE_LOW */
	GPIOR_CLRIO(SPI_PORT, SPI_SPCK_PIN);
#endif /* if (SPI_CPOL) */
}


/*
 * @ open SPI
 */
int SPI_open(int ch, unsigned int clock_freq_hz) {
	SPI_GPIO_INIT(ch);
#if !(SPI_MAX_FREQ)
	SPI_CLK_DELAY[ch] = GPIOR_MAX_FREQHZ / clock_freq_hz / 2;
	if(SPI_CLK_DELAY[ch] == 0)
		SPI_DELAYUS = SPI_DUMMY;
	else
		SPI_DELAYUS = SPI_GPIOR_DELAYUS;
#endif
	SPI_CHEN[ch] = TRUE;
	return 0;
}


/*
** @ Active the SPI CS line
**/
void SPI_enable(int ch) {
#if (SPI_CS) /* SPI_CS_ACTIVE_HIGH */
	GPIOR_SETIO(SPI_PORT, SPI_NPCS_PIN(ch));
#else 		 /* SPI_CS_ACTIVE_LOW */
	GPIOR_CLRIO(SPI_PORT, SPI_NPCS_PIN(ch));
#endif /* if (SPI_CS) */
}


/*
** @ Inactive the SPI CS line
**/
void SPI_disable(int ch) {
#if (SPI_CS) /* SPI_CS_ACTIVE_HIGH */
	GPIOR_CLRIO(SPI_PORT, SPI_NPCS_PIN(ch));
#else 		 /* SPI_CS_ACTIVE_LOW */
	GPIOR_SETIO(SPI_PORT, SPI_NPCS_PIN(ch));
#endif /* if (SPI_CS) */	
}


/*
 * @ software SPI write
 * @ return read value
 */
unsigned char SPI_write(int ch, unsigned char byte) {
	unsigned char buf = 0;
	unsigned char i;

	for(i = 0; i < 8; i++) {
#if (SPI_CPOL)		/* SPI_CLOCK_INACTIVE_HIGH */
#	if (SPI_CLPH) 	/* SPI_CAPTURE_ON_RISING_EDGE */
		/* CLOCK line low */
		GPIOR_CLRIO(SPI_PORT, SPI_SPCK_PIN);
		/* MOSI line output */
		if(byte & 0x80) { // MSB
			GPIOR_SETIO(SPI_PORT, SPI_MOSI_PIN);
		} else {
			GPIOR_CLRIO(SPI_PORT, SPI_MOSI_PIN);
		}
		byte <<= 1;
		SPI_DELAYUS(SPI_CLK_DELAY[ch]);
		/* CLOCK line high */
		GPIOR_SETIO(SPI_PORT, SPI_SPCK_PIN);
		/* MISO line input */
		buf <<= 1;
		if(GPIOR_GETIO(SPI_PORT, SPI_MISO_PIN)) {
			buf |= 1;
		}
		SPI_DELAYUS(SPI_CLK_DELAY[ch]);		
#	else 			/* SPI_CAPTURE_ON_FALLING_EDGE */
		/* MOSI line output */
		if(byte & 0x80) { // MSB
			GPIOR_SETIO(SPI_PORT, SPI_MOSI_PIN);
		} else {
			GPIOR_CLRIO(SPI_PORT, SPI_MOSI_PIN);
		}
		byte <<= 1;
		SPI_DELAYUS(SPI_CLK_DELAY[ch]);
		/* CLOCK line low */
		GPIOR_CLRIO(SPI_PORT, SPI_SPCK_PIN);
		/* MISO line input */
		buf <<= 1;
		if(GPIOR_GETIO(SPI_PORT, SPI_MISO_PIN)) {
			buf |= 1;
		}
		SPI_DELAYUS(SPI_CLK_DELAY[ch]);	
		/* CLOCK line high */
		GPIOR_SETIO(SPI_PORT, SPI_SPCK_PIN);
		
#	endif /* if (SPI_CPOL) */		
#else 				/* SPI_CLOCK_INACTIVE_LOW */
#	if (SPI_CLPH)	/* SPI_CAPTURE_ON_RISING_EDGE */
		/* MOSI line output */
		if(byte & 0x80) { // MSB
			GPIOR_SETIO(SPI_PORT, SPI_MOSI_PIN);
		} else {
			GPIOR_CLRIO(SPI_PORT, SPI_MOSI_PIN);
		}
		byte <<= 1;
		SPI_DELAYUS(SPI_CLK_DELAY[ch]);
		/* CLOCK line high */
		GPIOR_SETIO(SPI_PORT, SPI_SPCK_PIN);
		/* MISO line input */
		buf <<= 1;
		if(GPIOR_GETIO(SPI_PORT, SPI_MISO_PIN)) {
			buf |= 1;
		}
		SPI_DELAYUS(SPI_CLK_DELAY[ch]);
		/* CLOCK line low */
		GPIOR_CLRIO(SPI_PORT, SPI_SPCK_PIN);
#	else 			/* SPI_CAPTURE_ON_FALLING_EDGE */
		/* CLOCK line high */
		GPIOR_SETIO(SPI_PORT, SPI_SPCK_PIN);
		/* MOSI line output */
		if(byte & 0x80) { // MSB
			GPIOR_SETIO(SPI_PORT, SPI_MOSI_PIN);
		} else {
			GPIOR_CLRIO(SPI_PORT, SPI_MOSI_PIN);
		}
		byte <<= 1;
		SPI_DELAYUS(SPI_CLK_DELAY[ch]);
		/* CLOCK line low */
		GPIOR_CLRIO(SPI_PORT, SPI_SPCK_PIN);
		/* MISO line input */
		buf <<= 1;
		if(GPIOR_GETIO(SPI_PORT, SPI_MISO_PIN)) {
			buf |= 1;
		}
		SPI_DELAYUS(SPI_CLK_DELAY[ch]);		
#	endif /* if (SPI_CPOL) */		
#endif /* if (SPI_CLPH) */
	}
	return buf;
}

/*
 * @ SPI close
 */	
int SPI_close(int ch) {
	if(IS_SPI_ALLCH_DISABLED()) {
		/* all channels are closed */
		GPIOR_CFGIO_INPUT(SPI_PORT, SPI_MOSI_PIN);
		GPIOR_CFGIO_INPUT(SPI_PORT, SPI_MISO_PIN);
		GPIOR_CFGIO_INPUT(SPI_PORT, SPI_SPCK_PIN);
		SPI_PLAT_DEINIT(ch);
	}
	GPIOR_CFGIO_INPUT(SPI_PORT, SPI_NPCS_PIN(ch));
	SPI_CHEN[ch] = TRUE;
 	return 0;
}

#else
#error SET GPIOR_SPI TO 1 (IN iocfg.h) TO ENABLE SPI DRIVER
#endif // if (GPIOR_SPI)

