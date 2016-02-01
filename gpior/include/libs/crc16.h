#ifndef CRC16_H
#define CRC16_H

#define CRC16_LOOKUP_TABLE	1

extern unsigned short crc16(unsigned short crc, unsigned char *buffer, unsigned int len);

#endif //#ifndef CRC16_H
