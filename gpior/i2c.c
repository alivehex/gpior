/*
============================================================
	名称: i2c.c
	描述: 模拟I2C总线协议底层函数
	作者: L.H.R		jk36125@gmail.com
	时间: 2009-12-9 15:20:58
	版本: 1.0
	备注: 无
============================================================
*/
#include "iocfg.h"

#if (GPIOR_I2C)

/* I2C timeing parameters in ns */
typedef struct _I2C_TIMING {
	uint8 	start_hold;
	uint8 	clk_low;
	uint8 	clk_high;
	uint8 	dat_hold;
	uint8 	dat_setup;
	uint8 	stop_hold;
	uint8 	bus_free;
	uint8 	clk_low_sub_dat_hold;
} I2C_TIMING_T;

/* STD speed config parameters */
/*const static I2C_TIMING_T I2C_STD_SPDPA = {4000, 4700, 4000, 5, 250, 4000, 4700, 4600}*/
static I2C_TIMING_T I2C_STD_SPDPA  = {40, 47, 40, 1, 3, 40, 47, 47};

/* HIGH speed config parameters */
/* const static I2C_TIMING_T I2C_HIGH_SPDPA = { 600, 1300,  600, 0, 100,  600, 1300, 1300}; */
static I2C_TIMING_T I2C_HIGH_SPDPA = {6, 13,  6, 1, 1,  6, 13, 13};

/* host only mode */
static I2C_TIMING_T  *I2C_TP = &I2C_STD_SPDPA;

/*
 * @ init I2C GPIO
 */
static __inline void I2C_GPIO_INIT() {
	I2C_PLAT_INIT();
	GPIOR_CFGIO_OUTPUT(I2C_PORT, I2C_SCL_PIN);
	GPIOR_CFGIO_OUTPUT(I2C_PORT, I2C_SDA_PIN);
} 

/*
 * @ open I2C port
 */
int I2C_open(i2c_mode_t type) {
	I2C_GPIO_INIT();
	I2C_config(type);
	return 0;
}

//*----------------------------------------------------------------------------
//*  Name     : i2c_config()
//*  Brief    : Config I2C bus speed
//*  Argument : I2CSPD_STD or I2CSPD_HIGH
//*  Return   : None
//*----------------------------------------------------------------------------
void I2C_config(i2c_mode_t t) {
	switch(t) {
	case I2C_STANDARD_MODE:
		I2C_TP = &I2C_STD_SPDPA;
		break;
	case I2C_HIGHSPEED_MODE:
		I2C_TP = &I2C_HIGH_SPDPA;
		break;
	default:
		I2C_TP = &I2C_STD_SPDPA;	
	}
} 

//*----------------------------------------------------------------------------
//*  Name     : i2c_start()
//*  Brief    : Write a start conditon on I2C bus
//*  Argument : None
//*  Return   : None
//*----------------------------------------------------------------------------
void I2C_start() {
	// Start condition: CLK = H, DAT = H -> L
	GPIOR_CFGIO_OUTPUT(I2C_PORT, I2C_SCL_PIN);
	GPIOR_CFGIO_OUTPUT(I2C_PORT, I2C_SDA_PIN);
	
	/*  SDA  L/H -> H -> L -> L
 	 *	SCL	   L -> H -> H -> L
 	 * */
	GPIOR_SETIO(I2C_PORT, I2C_SDA_PIN);
	GPIOR_DELAYUS(I2C_TP->dat_hold);
	GPIOR_SETIO(I2C_PORT, I2C_SCL_PIN);
	GPIOR_SETIO(I2C_PORT, I2C_SCL_PIN);
	GPIOR_SETIO(I2C_PORT, I2C_SCL_PIN);
	GPIOR_CLRIO(I2C_PORT, I2C_SDA_PIN);
	GPIOR_DELAYUS(I2C_TP->start_hold);
	GPIOR_CLRIO(I2C_PORT, I2C_SCL_PIN);
}

//*----------------------------------------------------------------------------
//*  Name     : i2c_stop()
//*  Brief    : Write a stop condition on I2C bus
//*  Argument : None 
//*  Return   : None
//*----------------------------------------------------------------------------
void I2C_stop() {
	// Stop condition: CLK = H, DAT = L -> H
	GPIOR_CFGIO_OUTPUT(I2C_PORT, I2C_SDA_PIN);
	GPIOR_CFGIO_OUTPUT(I2C_PORT, I2C_SCL_PIN);	
	
	
	/*	SDA	 L/H -> L -> L -> H
 	 *	SCL	   L -> L -> H -> H	
 	 * */
	GPIOR_CLRIO(I2C_PORT, I2C_SDA_PIN);
	GPIOR_DELAYUS(I2C_TP->clk_low_sub_dat_hold);
	GPIOR_SETIO(I2C_PORT, I2C_SCL_PIN);
	GPIOR_DELAYUS(I2C_TP->stop_hold);
	GPIOR_SETIO(I2C_PORT, I2C_SDA_PIN);
	GPIOR_DELAYUS(I2C_TP->bus_free);
}

//*----------------------------------------------------------------------------
//*  Name     : i2c_write_bit()
//*  Brief    : Write a bit on I2C bus
//*  Argument : bit = 0 or bit > 0
//*  Return   : None
//*----------------------------------------------------------------------------
static void i2c_write_bit(boolean bit) {					   		  	
	GPIOR_DELAYUS(I2C_TP->dat_hold);
	if(bit) {
		GPIOR_SETIO(I2C_PORT, I2C_SDA_PIN);
	} else {
		GPIOR_CLRIO(I2C_PORT, I2C_SDA_PIN);
	}
	// remain clock low
	GPIOR_DELAYUS(I2C_TP->clk_low_sub_dat_hold);
	// clock high
	GPIOR_SETIO(I2C_PORT, I2C_SCL_PIN);
	GPIOR_DELAYUS(I2C_TP->clk_high);
	GPIOR_CLRIO(I2C_PORT, I2C_SCL_PIN);	
}



//*----------------------------------------------------------------------------
//*  Name     : i2c_read_bit()
//*  Brief    : read a bit from I2C bus
//*  Argument : None
//*  Return   : bit value: 0 or NOT
//*----------------------------------------------------------------------------
static int i2c_read_bit() {
	int state;
	
	GPIOR_DELAYUS(I2C_TP->clk_low);
	GPIOR_SETIO(I2C_PORT, I2C_SCL_PIN);
	GPIOR_DELAYUS(I2C_TP->clk_high);
	state = GPIOR_GETIO(I2C_PORT, I2C_SDA_PIN);
	GPIOR_CLRIO(I2C_PORT, I2C_SCL_PIN);
	return state;
}



//*----------------------------------------------------------------------------
//*  Name     : i2c_write_byte()
//*  Brief    : Write a byte on I2C bus
//*  Argument : Data value which is 8bits
//*  Return   : ACK = 0, NACK = -1
//*----------------------------------------------------------------------------
int I2C_write_byte(uint8 c) {
	int i;
	
	GPIOR_CFGIO_OUTPUT(I2C_PORT, I2C_SDA_PIN);
	for( i = 0; i < 8; i++) {
		i2c_write_bit((c << i) & 0x80);
	}
	GPIOR_CFGIO_INPUT(I2C_PORT, I2C_SDA_PIN);
	// check ack
	if(i2c_read_bit()) {
		// no ack
		return -1;
	} else {
		// ack
		return 0;
	}
}



//*----------------------------------------------------------------------------
//*  Name     : i2c_read_byte() 
//*  Brief    : Read a byte from I2C bus
//*  Argument : FALSE = No ACK, TRUE = ACK  
//*  Return   : Data read from I2C bus
//*----------------------------------------------------------------------------
int I2C_read_byte(int ACK) {
	uint8 buf = 0;
	int i;
	
	GPIOR_CFGIO_INPUT(I2C_PORT, I2C_SDA_PIN);
	for( i = 0; i < 8; i ++) {
		buf <<= 1;
		if(i2c_read_bit()) {
			buf |= 1;
		}
	}
	// ack or not
	GPIOR_CFGIO_OUTPUT(I2C_PORT, I2C_SDA_PIN);
	if(ACK) {
		i2c_write_bit(0);
	} else {
		i2c_write_bit(1);
	}
	return buf;	
}

/*
 * @ i2c close
 */
int I2C_close() {
 	GPIOR_CFGIO_INPUT(I2C_PORT, I2C_SDA_PIN);
	GPIOR_CFGIO_INPUT(I2C_PORT, I2C_SCL_PIN);
	I2C_PLAT_DEINIT();
	return 0;
}
#else
#error SET GPIOR_I2C TO 1 TO ENABLE I2C DRIVER(IN iocfg.h)
#endif // if (GPIOR_I2C)


