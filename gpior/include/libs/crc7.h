#ifndef _LINUX_CRC7_H_
#define _LINUX_CRC7_H_

#define CRC7_LOOKUP_TABLE	1

extern unsigned char crc7(unsigned char crc, const unsigned char *buffer, unsigned int len);

/*
 * @ CRC7 for memery sub-opt
 */
extern unsigned char _crc7(unsigned char crc, const unsigned char *buffer, unsigned int len);

#endif /* _LINUX_CRC7_H_ */
