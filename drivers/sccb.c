/*
============================================================
	名称: sccb.c
	描述: SCCB 总线协议
	作者: L.H.R		jk36125@gmail.com
	时间: 2009-12-10 9:44:51
	版本: 1.0
	备注: 无
============================================================
*/

#include "iocfg.h"

#if (GPIOR_SCCB)

static unsigned char SCCB_RADDR;
static unsigned char SCCB_WADDR;
	
/*
 * @ sccb open
 */
int SCCB_open(unsigned char dev_addr) {
	I2C_open(I2C_STANDARD_MODE);
 	SCCB_RADDR = dev_addr | 0x01;
	SCCB_WADDR = dev_addr & 0xfe;
	return 0;
}

//*----------------------------------------------------------------------------
//*  Name     : sccb_write_byte()
//*  Brief    : Write single byte on SCCB bus
//*  Argument : dat, internal address addr
//*  Return   : Error = -1, OK = 0
//*----------------------------------------------------------------------------
int SCCB_write_byte(unsigned char dat, unsigned char addr) {
	// write start condition
	I2C_start();	
	// write device address
	if(I2C_write_byte(SCCB_WADDR) != 0) {
		I2C_stop();
		return -1;
	}
	// write internal address
	if(I2C_write_byte(addr) != 0) {
		I2C_stop();
		return -1;
	}
	// write data
	if(I2C_write_byte(dat) != 0) {
	 	I2C_stop();
		return -1;
	}
	I2C_stop();
	return 0;
}

//*----------------------------------------------------------------------------
//*  Name     : sccb_read_byte()
//*  Brief    : Read a byte from internal address
//*  Argument : Internal address addr
//*  Return   : Data
//*----------------------------------------------------------------------------
int SCCB_read_byte(unsigned char addr) {
	int ret = 0;
	
	// dummy write for address
	I2C_start();
	if(I2C_write_byte(SCCB_WADDR) != 0) {
		I2C_stop();
		return -1;
	}
	if(I2C_write_byte(addr) != 0) {
		I2C_stop();
		return -1;
	}
	I2C_stop();
	// random read
	I2C_start();
	if(I2C_write_byte(SCCB_RADDR) != 0) {
		I2C_stop();
		return -1;
	}
	ret = I2C_read_byte(FALSE);
	I2C_stop();
	return ret;
}

/*
** @ close SCCB
**/
int SCCB_close() {
	I2C_close();
	return 0;
}

#else
#error SET GPIOR_SCCB TO 1 TO ENABLE SCCB DRIVER
#endif // if (GPIOR_SCCB)




