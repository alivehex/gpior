/*
============================================================
	名称: i2c.c
	描述: I2C Driver
	作者: L.H.R		jk36125@gmail.com
	时间: 2009-12-30 11:08:34
	版本: 0.1
	备注: 无
============================================================
*/


#include "iocfg.h"

#if (GPIOR_E2PROM)

#if (GPIOR_E2PROM_WP)
static __inline void EEPROM_WPIO_INIT() {
 	E2PROMWP_PLAT_INIT();
	GPIOR_CFGIO_OUTPUT(E2PROM_WP_PORT, E2PROM_WP_PIN);
}
#endif // if (GPIOR_EEPROM_WP)

/* device sub address length in byte */
static uint8 I2C_SUBADDR_LENB;
/* device address for reading */	
static uint8 I2C_RADDR;
/* device address for writing */		
static uint8 I2C_WADDR;		 


//*----------------------------------------------------------------------------
//*  Name     : write_sub_addr()
//*  Brief    : Write sub address
//*  Argument : I2C host pointer
//*  Return   : -1 / 0
//*----------------------------------------------------------------------------
static int write_sub_addr(uint32 sub_addr) {
	int i;
	
	for(i = 0; i < I2C_SUBADDR_LENB; i++) {
		if(I2C_write_byte((sub_addr >> ((I2C_SUBADDR_LENB - 1 - i) * 8) & 0xff)) != 0) {
			return -1;
		}	
	}
	return 0;
}

/* 
** @ E2PROM open with sub-address length and device address
**/
int E2PROM_open(uint8 sub_addr_lenb, uint8 dev_addr) {
	if(I2C_open(I2C_HIGHSPEED_MODE) != 0)
		return -1;
#if (GPIOR_E2PROM_WP)
	EEPROM_WPIO_INIT();
#endif // if (GPIOR_EEPROM_WP)
	I2C_SUBADDR_LENB = sub_addr_lenb;
	I2C_RADDR = dev_addr | 0x01;
	I2C_WADDR = dev_addr & 0xfe;
	return 0;
}


/*
** @ E2PROM read with sub-address
**/
int E2PROM_read(uint32 sub_addr, uint8 * buf, uint32 len) {
	int i;
	
	// start condition
	I2C_start();
	// command: write
	if(I2C_write_byte(I2C_WADDR) != 0) {
		I2C_stop();
		return -1;
	}
	// sub sddress
	if(write_sub_addr(sub_addr) != 0) {
		I2C_stop();
		return -1;
	}
	// retart condition
	I2C_start();
	// commond: read
	if(I2C_write_byte(I2C_RADDR) != 0) {
		I2C_stop();
		return -1;
	}	
	// read datas
	for(i = 0; i < (len - 1); i ++) {
		*(buf++) = I2C_read_byte(TRUE);	
	}
	// last byte without ack
	*buf = I2C_read_byte(FALSE);
	I2C_stop();
	return 0;	
}


/*
** @ E2PROM write with sub-address
**/
int E2PROM_write(uint32 sub_addr, uint8 * buf,	uint32 len) {
	int i;
	
	// start condition
	I2C_start();
	// command: write
	if(I2C_write_byte(I2C_WADDR) != 0) {
		I2C_stop();
		return -1;
	}
	// sub sddress
	if(write_sub_addr(sub_addr) != 0) {
		I2C_stop();
		return -1;
	}
	// start write
	for(i = 0; i < len; i ++) {
		if(I2C_write_byte(*(buf++)) != 0) {
			I2C_stop();
			return -1;
		}	
	}
	I2C_stop();
	return 0;			  
}

/*
** @ enable the write protect
**/
void E2PROM_wp_enable() {
#if (GPIOR_E2PROM_WP)
	GPIOR_SETIO(E2PROM_WP_PORT, E2PROM_WP_PIN);	
#endif
}

/*
** @ disable the write protect
**/
void E2PROM_wp_disable() {
#if (GPIOR_E2PROM_WP)	
	GPIOR_CLRIO(E2PROM_WP_PORT, E2PROM_WP_PIN);
#endif
}


/*
** @ close E2PROM
**/
int E2PROM_close(void) {
	I2C_close();
#if (GPIOR_E2PROM_WP)
	E2PROMWP_PLAT_DEINIT();
#endif // if (GPIOR_EEPROM_WP)
	return 0;
}

#else
#error SET GPIOR_E2PROM TO 1 TO ENABLE E2PROM DRIVER
#endif
