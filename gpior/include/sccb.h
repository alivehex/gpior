#ifndef _SCCB_H_
#define _SCCB_H_


extern int SCCB_open(unsigned char dev_addr);
extern int SCCB_write_byte(unsigned char dat, unsigned char addr);
extern int SCCB_read_byte(unsigned char addr);
extern int SCCB_close(void);

				   
#endif  // sccb.h
