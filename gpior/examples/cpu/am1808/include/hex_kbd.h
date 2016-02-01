#ifndef _HEX_KBD_H_
#define _HEX_KBD_H_


#include "types.h"

extern int KBD_open(unsigned int kbdLbufSizeb);
extern int KBD_read(uint8 *buffer, uint32 lenb);
extern int KBD_close(void);

#endif /* ifndef _HEX_KBD_H_ */
