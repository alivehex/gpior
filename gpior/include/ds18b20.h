#ifndef _DS18B20_H_
#define _DS18B20_H_


/* simple driver */
#define DS_SIMDRV

//------ DS18B20 Config type define
typedef enum _TEMP_CONBITS {
	DS_9BIT_TYPE,
	DS_10BIT_TYPE,
	DS_11BIT_TYPE,
	DS_12BIT_TYPE	
} DS_TEMP_T;


//------ DS18B20 registers define 
typedef struct _DS_REG {
	// ROM register
	uint8 ds_id;			// 8 bits, should be 0x19
	uint8 serial_num[6];	// 48 bits serial number
	uint8 rom_crc7;
	// RAM register
	uint8 temp_lsb;
	uint8 temp_msb;
	uint8 usr_byte[2];
	uint8 config;
	uint8 reserve[3];
	uint8 ram_crc7;	

	// conver time, in ms
	unsigned int con_time;				
} DS18B20_REG_T;


extern int DS_open(DS18B20_REG_T * ds);
extern int DS_start_con(DS18B20_REG_T * ds);
extern int DS_read_temp(DS18B20_REG_T * ds);
extern int DS_close(void);


#endif
