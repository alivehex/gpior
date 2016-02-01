#include "gpior.h"

#if (GPIOR_I2S)

#define I2S_WRITE_BIT(dat) \
	GPIOR_CLRIO(I2S_PORT, I2S_SCK_PIN); \
	GPIOR_DELAY1US();\
	(dat) ? (GPIOR_SETIO(I2S_PORT, I2S_SDO_PIN)) : (GPIOR_CLRIO(I2S_PORT, I2S_SDO_PIN)); \
	GPIOR_SETIO(I2S_PORT, I2S_SCK_PIN); \
	GPIOR_DELAY1US()

static __inline void I2S_IO_INIT() {
	I2S_PLAT_INIT();
 	GPIOR_CFGIO_OUTPUT(I2S_PORT, I2S_WS_PIN);
	GPIOR_CFGIO_OUTPUT(I2S_PORT, I2S_SDO_PIN);
	GPIOR_CFGIO_OUTPUT(I2S_PORT, I2S_SCK_PIN);
}

__inline static void I2S_WS() {
 	static int WS = 0;
	if(WS) {
		GPIOR_CLRIO(I2S_PORT, I2S_WS_PIN);
		WS = 0;
	} else {
		GPIOR_SETIO(I2S_PORT, I2S_WS_PIN);
		WS = 1;
	}
}

__inline static void I2S_WRITE_FRAME(uint16 dat) {
#if (GPIOR_I2S_TYPE == 0)
	I2S_WS();
#endif
	I2S_WRITE_BIT(dat & (1 << 15));
#if (GPIOR_I2S_TYPE == 1) // PHILIPS
	I2S_WS();
#endif 
	I2S_WRITE_BIT(dat & (1 << 14));
	I2S_WRITE_BIT(dat & (1 << 13));
	I2S_WRITE_BIT(dat & (1 << 12));
	I2S_WRITE_BIT(dat & (1 << 11));
	I2S_WRITE_BIT(dat & (1 << 10));
	I2S_WRITE_BIT(dat & (1 <<  9));
	I2S_WRITE_BIT(dat & (1 <<  8));
	I2S_WRITE_BIT(dat & (1 <<  7));
	I2S_WRITE_BIT(dat & (1 <<  6));
	I2S_WRITE_BIT(dat & (1 <<  5));
	I2S_WRITE_BIT(dat & (1 <<  4));
	I2S_WRITE_BIT(dat & (1 <<  3));
	I2S_WRITE_BIT(dat & (1 <<  2));
	I2S_WRITE_BIT(dat & (1 <<  1));
	I2S_WRITE_BIT(dat & (1 <<  0));
}

/*
 * @ I2S open with samples per second
 * @ GPIOR FIXED 16bit, 16KPS, Stereo
 */
int i2s_open(int bits, int sps, I2S_CH_T type) {
	I2S_IO_INIT();
	return 0;
}

/*
 * @ I2S write
 */
int i2s_write(uint16 *buffer, uint32 lenb) {
	lenb >>= 1;
	while(lenb --) {
		I2S_WRITE_FRAME(*(buffer ++));
	}
	return 0; 								  
}

/*
 * @ I2S close
 */
int i2s_close() {
	GPIOR_CFGIO_INPUT(I2S_PORT, I2S_WS_PIN);
	GPIOR_CFGIO_INPUT(I2S_PORT, I2S_SDO_PIN);
	GPIOR_CFGIO_INPUT(I2S_PORT, I2S_SCK_PIN);
 	return 0;
}

#endif // if (GPIOR_I2S)
