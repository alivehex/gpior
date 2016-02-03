/***************************************************
*    文件名称: 18B20.c                               
*        用途: 温度传感器18b20               
*        作者: jk36125                             
*    创建日期: 2009年4月12日                           
*    最后修改: 2009年4月12日                                   
*        备注: 无    
****************************************************/


#include "iocfg.h"
#include "crc7.h"

#if (GPIOR_DS18B20)


//------ DS18B20 Command define
#define DS_CMD_READ_ROM		0x33
#define DS_CMD_MATCH_ROM	0x55
#define DS_SEARCH_ROM		0xf0
#define DS_CMD_SKIP_ROM		0xcc
#define DS_CMD_START_CON	0x44
#define	DS_CMD_READ_RAM		0xbe
#define DS_CMD_WRITE_RAM	0x4e


/******************************************************** example 

DS18B20_REG_T ds18b20;

if(ds_init(&ds18b20) < 0) {
	return -1;
}
if(ds_start_con(&ds18b20) < 0) {
	return -1;
}
wait_ms(ds18b20.con_time);
if(ds_read_temp(&ds18b20) {
	return -1;
}
// now tempreture is in ds18b20.temp_lsb and .temp_msb

****************************************************************/

#if (0)
//*----------------------------------------------------------------------------
//* Name     : ds_host_io_test()
//* Brief    : Host io test
//* Argument : None  
//* Return   : None
//*----------------------------------------------------------------------------
void ds_host_io_test() {
	
	GPIOR_CFGIO_OUTPUT(DS18B20_PORT, DS18B20_PIN);
	while(1) {
		GPIOR_SETIO(DS18B20_PORT, DS18B20_PIN);
		GPIOR_CLRIO(DS18B20_PORT, DS18B20_PIN);
	}	
}
#endif



//*----------------------------------------------------------------------------
//* Name     : ds_reset()
//* Brief    : Reset DS18B20
//* Argument : None  
//* Return   : Error = -1, OK = 0
//*----------------------------------------------------------------------------
static int ds_reset() {
	unsigned int buf = 0;	

	GPIOR_CFGIO_OUTPUT(DS18B20_PORT, DS18B20_PIN);

	// Start with H state, End with H state
	GPIOR_SETIO(DS18B20_PORT, DS18B20_PIN);
	GPIOR_DELAYUS(10);
	
	// Set data = L at least 480 us 
	GPIOR_CLRIO(DS18B20_PORT, DS18B20_PIN);
	GPIOR_DELAYUS(600);
	
	// Set dat = H, wait 80us
	GPIOR_SETIO(DS18B20_PORT, DS18B20_PIN);
	GPIOR_DELAYUS(80);
	
	// DS18B20 should return a L state here
	GPIOR_CFGIO_INPUT(DS18B20_PORT, DS18B20_PIN);
	buf = GPIOR_GETIO(DS18B20_PORT, DS18B20_PIN);
	
	// End with H state
	GPIOR_CFGIO_OUTPUT(DS18B20_PORT, DS18B20_PIN);
	GPIOR_SETIO(DS18B20_PORT, DS18B20_PIN);
	GPIOR_DELAYUS(600);
	
	if(buf) {
		return -1;	
	}
	return buf;
} 



//*----------------------------------------------------------------------------
//* Name     : ds_write_bit()
//* Brief    : Write a bit data on bus
//* Argument : bit = 0 or bit > 0
//* Return   : None
//*----------------------------------------------------------------------------
static void ds_write_bit(unsigned char bit) {
	
	GPIOR_CFGIO_OUTPUT(DS18B20_PORT, DS18B20_PIN);
	
	// Start with H state, End with H state
	GPIOR_SETIO(DS18B20_PORT, DS18B20_PIN);
	GPIOR_DELAYUS(10);
	
	// Clear dat, hold for 5us ~ as start condition 
	GPIOR_CLRIO(DS18B20_PORT, DS18B20_PIN);
	GPIOR_DELAYUS(5);
	
	if(bit) {
		// write a H
		GPIOR_SETIO(DS18B20_PORT, DS18B20_PIN);	
	}
	
	// hold for 60, DS18B20 will read bus state here
	GPIOR_DELAYUS(100); 	 
	
	// End with H state
	GPIOR_SETIO(DS18B20_PORT, DS18B20_PIN);
	GPIOR_DELAYUS(10);
}



//*----------------------------------------------------------------------------
//* Name     : ds_read_bit()
//* Brief    : Read one bit data from bus
//* Argument : None   
//* Return   : 0 or > 0
//*----------------------------------------------------------------------------
static uint8 ds_read_bit() {
	uint8 buf;
	
	GPIOR_CFGIO_OUTPUT(DS18B20_PORT, DS18B20_PIN);
	
	// Start with H state, End with H state
	GPIOR_SETIO(DS18B20_PORT, DS18B20_PIN);
	GPIOR_DELAYUS(10);
	
	// Clear dat, hold for 5us ~ as start condition 
	GPIOR_CLRIO(DS18B20_PORT, DS18B20_PIN);
	GPIOR_DELAYUS(5);
	GPIOR_SETIO(DS18B20_PORT, DS18B20_PIN);
	
	// Data will be read at 10us after  
	GPIOR_CFGIO_INPUT(DS18B20_PORT, DS18B20_PIN);
	GPIOR_DELAYUS(5);
	
	buf = GPIOR_GETIO(DS18B20_PORT, DS18B20_PIN);
	
	GPIOR_DELAYUS(50);
	GPIOR_CFGIO_OUTPUT(DS18B20_PORT, DS18B20_PIN);
	
	// End with H state
	GPIOR_SETIO(DS18B20_PORT, DS18B20_PIN);
	GPIOR_DELAYUS(10);
	
	return buf;
}



//*----------------------------------------------------------------------------
//* Name     : ds_write_byte()
//* Brief    : Write a byte data on bus
//* Argument : 8 bits data  
//* Return   : None
//*----------------------------------------------------------------------------
static void ds_write_byte(unsigned char c) {
	int i;
	
	for(i = 0; i < 8; i ++) {
		// LSB first
		ds_write_bit((c >> i) & 0x01);
	}	
}



//*----------------------------------------------------------------------------
//* Name     : ds_read_byte()
//* Brief    : Read byte from bus
//* Argument : None
//* Return   : 8 bits Data
//*----------------------------------------------------------------------------
static int ds_read_byte() {
	int i = 0;
	uint8 buf = 0;
	
	for(i = 0; i < 8; i ++) {
		buf >>= 1;
		// LSB First	
		if(ds_read_bit()) {
			buf |= 0x80;	
		}
	}
	return buf;	
}


//*----------------------------------------------------------------------------
//* Name     : ds_read_all_rom() 
//* Brief    : Read all DS18B20 ROM registers
//* Argument : DS_REG pointer    
//* Return   : Error = -1, OK = 0
//*----------------------------------------------------------------------------
static int ds_read_all_rom(DS18B20_REG_T *ds) {
	int i;
	
	if(ds == NULL) {
		return -1;	
	}
	
	// reset DS18B20
	if(ds_reset() < 0) {
		return -1;	
	}
	
	// Write commands
	ds_write_byte(DS_CMD_READ_ROM);
	
	// read 8 + 48 + 8 bits
	ds->ds_id = ds_read_byte();
	for(i = 0; i < 6; i++) {
		ds->serial_num[i] = ds_read_byte(); 
	}
	ds->rom_crc7 = ds_read_byte();
	
	// check crc7 here
	if(ds->rom_crc7 != crc7(0, (unsigned char *)ds, 7)) {
		return -1;
	}
	return 0;
}

//*----------------------------------------------------------------------------
//* Name     : ds_write_match_rom()
//* Brief    : Write match rom command and 64 bit rom datas
//* Argument : DS_REG pointer    
//* Return   : Error = -1, OK = 0
//*----------------------------------------------------------------------------
static void ds_write_match_rom(DS18B20_REG_T *ds) {
	int i;
	uint8 *c = &ds->ds_id;	
	
	// Write commands
	ds_write_byte(DS_CMD_MATCH_ROM);
	// Write 64 bis ROM 
	for(i = 0; i < 8; i ++) {
		ds_write_byte(*(c ++));
	}		
}

//*----------------------------------------------------------------------------
//* Name     : ds_calculate_conver_time()
//* Brief    : Calculate the DS18B20 current conver time
//* Argument : DS_REG pointer  
//* Return   : Error = -1, OK = 0
//*----------------------------------------------------------------------------
static int ds_calculate_conver_time(DS18B20_REG_T *ds) {
	
	if(ds == NULL) {
		return -1;	
	}
	
	// check ds->config illegal
	if((ds->config & ~(3 << 5)) != 0x1f) {
		return -1;
	}
	// calculate conver time
	switch((ds->config >> 5) & 0x03) {
		case DS_9BIT_TYPE:
			ds->con_time = 100;
			return 0;
		case DS_10BIT_TYPE:
			ds->con_time = 200;
			return 0;
		case DS_11BIT_TYPE:
			ds->con_time = 400;
			return 0;
		case DS_12BIT_TYPE:
			ds->con_time = 800;
			return 0;
		default:
			return -1;			
	}
}

//*----------------------------------------------------------------------------
//* Name     : ds_read_all_ram()
//* Brief    : Read all DS18B20 RAM register
//* Argument : DS_REG pointer  
//* Return   : Error = -1, OK = 0
//*----------------------------------------------------------------------------
static ds_read_all_ram(DS18B20_REG_T *ds) {
	int i;
	uint8 *c = NULL;
	
	if(ds == NULL) {
		return -1;	
	}	
	
	// reset DS18B20
	if(ds_reset() < 0) {
		return -1;	
	}
	
	ds_write_match_rom(ds);
	ds_write_byte(DS_CMD_READ_RAM);
	
	c = &ds->temp_lsb;
	for(i = 0; i < 9; i ++) {
		*(c ++) = ds_read_byte();	
	}
	// reset DS18B20
	if(ds_reset() < 0) {
		return -1;	
	}
	// check crc here
	if(ds->ram_crc7 !=  crc7(0, &ds->temp_lsb, 8)) {
		return -1;	
	}
	
	// calculate conver time
	return ds_calculate_conver_time(ds);
}


//*----------------------------------------------------------------------------
//* Name     : ds_configcon_type()
//* Brief    : Config conver BIT type
//* Argument : DS_TYPE  
//* Return   : Error = -1, OK = 0
//*----------------------------------------------------------------------------
static int ds_config_con_type(DS18B20_REG_T * ds, DS_TEMP_T type) {
	uint8 buf = 0;
	
	if(ds == NULL) {
		return -1;	
	}	
	
	// reset DS18B20
	if(ds_reset() < 0) {
		return -1;	
	}
	buf |= (type << 5);	
	
	// Write commands
	ds_write_match_rom(ds);
	ds_write_byte(DS_CMD_WRITE_RAM);
	
	ds_write_byte(ds->usr_byte[0]);
	ds_write_byte(ds->usr_byte[1]);
	ds_write_byte(buf);
	
	// reset DS18B20
	if(ds_reset() < 0) {
		return -1;	
	}
	
	// read back
	if(ds_read_all_ram(ds) < 0) {
		return -1;	
	}
	
	// check if write successed
	if(ds->config != (buf | 0x1f)) {
		return -1;
	} 
	return 0;
}


//*----------------------------------------------------------------------------
//* Name     : ds_init() 
//* Brief    : Init DS18B20, read ROM register
//* Argument : DS_REG pointer  
//* Return   : Error = -1, OK = 0
//*----------------------------------------------------------------------------
int DS_open(DS18B20_REG_T * ds) {
	DS18B20_PLAT_INIT();
	
	if(ds_read_all_rom(ds) < 0) {
		return -1;
	}
		
	if(ds_read_all_ram(ds) < 0) {
		return -1;
	}

	if(ds_config_con_type(ds, DS_12BIT_TYPE) < 0) {
		return -1;
	}
	return 0;
}



//*----------------------------------------------------------------------------
//* Name     : ds_start_con()
//* Brief    : Write command "Start convert tempreture"
//* Argument : DS_REG pointer    
//* Return   : Error = -1, OK = 0
//*----------------------------------------------------------------------------
int DS_start_con(DS18B20_REG_T * ds) {
	if(ds == NULL) {
		return -1;	
	}
	// reset first
	if(ds_reset() < 0) {
		return -1;	
	}
	// write match rom command
	ds_write_match_rom(ds);
	// write convert command
	ds_write_byte(DS_CMD_START_CON);
	return 0;
}



//*----------------------------------------------------------------------------
//* Name     : ds_read_temp()
//* Brief    : Read tempreture from DS18B20, should be called after ds_star-
//*			   t_con(), time para is in ds->con_time
//* Argument : DS_REG pointer    
//* Return   : Error = -1, OK = 0
//*----------------------------------------------------------------------------
int DS_read_temp(DS18B20_REG_T * ds) {
	
	if(ds == NULL) {
		return -1;	
	}	
	// reset DS18B20
	if(ds_reset() < 0)
		return -1;
	// we read all ram and check crc
	if(ds_read_all_ram(ds) != 0)
		return -1;
	return ((ds->temp_msb << 8) | ds->temp_lsb);
}


/*
** @ ds close
**/
int DS_close(void) {
	GPIOR_CFGIO_INPUT(DS18B20_PORT, DS18B20_PIN);
	DS18B20_PLAT_DEINIT();
	return 0;
}


#endif // if (GPIOR_DS18B20)

