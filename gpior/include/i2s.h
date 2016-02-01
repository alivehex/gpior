#ifndef _I2S_H_
#define _I2S_H_

#include "types.h"

typedef enum _I2S_CH_ {
	I2S_CH_MONO,
	I2S_CH_STEREO,
} I2S_CH_T;

/*
 * @ I2S open with samples per second
 * @ GPIOR FIXED 16bit, 16KPS, Stereo
 */
int i2s_open(int bits, int sps, I2S_CH_T type);

/*
 * @ I2S write
 */
int i2s_write(uint16 *buffer, uint32 lenb);

/*
 * @ I2S close
 */
int i2s_close(void);

#endif // ifndef _I2S_H_
