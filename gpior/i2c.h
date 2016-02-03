#ifndef _I2C_H_
#define _I2C_H_


/* I2C speed type */
typedef enum {
	I2C_STANDARD_MODE,
	I2C_HIGHSPEED_MODE,
} i2c_mode_t;


extern int I2C_open(i2c_mode_t mode);
extern int I2C_write_byte(unsigned char c);
extern int I2C_read_byte(int ack);
extern int I2C_close(void);
extern void I2C_config(i2c_mode_t mode);
extern void I2C_start(void);
extern void I2C_stop(void);


#endif  /* ifndef _I2C_H_ */
