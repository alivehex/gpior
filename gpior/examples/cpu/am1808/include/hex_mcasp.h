#ifndef _HEX_MCASP_H_
#define _HEX_MCASP_H_


#include "types.h"

extern int AUDIO_open(void);
extern int AUDIO_write(void *buffer, unsigned int len, int wait) ;
extern int AUDIO_close(void);

#endif
