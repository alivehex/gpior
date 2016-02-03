#ifndef NRF24L01_H
#define NRF24L01_H

#include "types.h"

/****** NRF24l01 commands ******/

// Read command and status registers
#define NRFCMD_RREG				0x1f
// Write command and status registers
#define NRFCMD_WREG				0x20
// Read RX-payload: 1 ¨C 32 bytes
#define NRFCMD_RRX_PAYLOAD		0x61
// Write TX-payload: 1 ¨C 32 bytes
#define NRFCMD_WTX_PAYLOAD		0xa0
// Flush TX FIFO, used in TX mode
#define NRFCMD_FLUSH_TX			0xe1
// Flush RX FIFO
#define NRFCMD_FLUSH_RX			0xe2
// Reuse last transmitted payload
#define NRFCMD_REUSE_TXPL		0xe3
#define NRFCMD_ACTIVATE			0x50
// Read RX-payload
#define NRFCMD_RRX_PLWID		0x60
#define NRFCMD_WACK_PAYLOAD		0xa8
// Disables AUTOACK on this specific packet.
#define NRFCMD_WTX_PLNACK		0xb0 
// used to read the STATUS register
#define NRFCMD_NOP				0xff

/****** NRF24L01 registers define ******/

// Configuration Register
#define NRFREG_CFG			0x00
// Enable ¡®Auto Acknowledgment¡¯ Function 
#define NRFREG_EN_AA		0x01 
// Enabled RX Addresses
#define NRFREG_EN_RXADDR	0x02
// Setup of Address Widths
#define NRFREG_SETUP_AW		0x03
#	define NRF_ADDR_WIDTHS_ILLEGAL	0
#	define NRF_ADDR_WIDTHS_3BYTES	1
#	define NRF_ADDR_WIDTHS_4BYTES	2
#	define NRF_ADDR_WIDTHS_5BYTES	3
// Setup of Automatic Retransmission
#define NRFREG_SETUP_RETR	0x04
// RF channel
#define NRFREG_SETUP_RFCH	0x05
// RF setup
#define NRFREG_RFSETUP		0x06
#	define NRF_DRATE_1M	0
#	define NRF_DRATE_2M	1
#	define NRF_RFPOWER_N18DBM	0
#	define NRF_RFPOWER_N12DBM	1
#	define NRF_RFPOWER_N6DBM	2
#	define NRF_RFPOWER_N0DBM	3
// Status
#define NRFREG_STATUS		0x07
// Transmit observe register
#define NRFREG_OBSERVE_TX	0x08	
// Carrier Detect
#define NRFREG_CD			0x09
// Receive address data pipe 0. 5 Bytes maximum length
#define NRFREG_RX_ADDR_P0	0x0a
// Receive address data pipe 1. 5 Bytes maximum length
#define NRFREG_RX_ADDR_P1	0x0b
// Receive address data pipe [2:5]
#define NRFREG_RX_ADDR_P2	0x0c
#define NRFREG_RX_ADDR_P3	0x0d
#define NRFREG_RX_ADDR_P4	0x0e
#define NRFREG_RX_ADDR_P5	0x0f
// Transmit address
#define NRFREG_TX_ADDR		0x10
// Number of bytes in RX payload in data pipe[0:5]
#define NRFREG_RX_PW_P0		0x11
#define NRFREG_RX_PW_P1		0x12
#define NRFREG_RX_PW_P2		0x13
#define NRFREG_RX_PW_P3		0x14
#define NRFREG_RX_PW_P4		0x15
#define NRFREG_RX_PW_P5		0x16
// FIFO Status Register
#define NRFREG_FIFO_STATUS	0x17
// Enable dynamic payload length
#define NRFREG_DYNPD		0x1c
// Feature Register
#define NRFREG_FEATURE		0x1d 

/****** SPI read&write function define ******/
// #include "iocfg.h"

//#define GPIOR_RF_CHANNEL	0

#define NRF_SPI_WRITE(data)	SPI_write(0, data)
#define NRF_SPI_READ()		SPI_write(0, 0xFF)
#define NRF_SPI_ENABLE()	SPI_enable(0)
#define NRF_SPI_DISABLE()	SPI_disable(0)
#define NRF_DELAYUS(us)		GPIOR_DELAYUS(us) 

// NRF24L01 chip enable
#define NRF_CHIP_ENABLE()	GPIOR_SETIO(RF_PORT, RF_CE_PIN)
#define NRF_CHIP_DISABLE()	GPIOR_CLRIO(RF_PORT, RF_CE_PIN)

// NRF24L01 power control, this is phy power control
#define NRF_POWER_ON()	
#define NRF_POWER_OFF()

/* FIXME *******************************************************************
	SPI bus, one operate is done, #CS must be HIGH before the next
	transmit. Or the reading value will be wrong.
****************************************************************************/

/****** NRF functions ******/

// NRF24L01 read
__inline void NRF_READ(uint8 cmd, uint8 * buffer, uint8 len) {
	NRF_SPI_ENABLE();
	NRF_SPI_WRITE(cmd);
	while(len --) {
		//NRF_SPI_WRITE(0xff);
		*(buffer ++) = NRF_SPI_READ();
	}
	NRF_SPI_DISABLE();	
}

// NRF24L01 write
__inline void NRF_WRITE(uint8 cmd, uint8 * buffer, uint8 len) {
	NRF_SPI_ENABLE();
	NRF_SPI_WRITE(cmd);
	while(len --) {
		NRF_SPI_WRITE(*(buffer ++));
	}
	NRF_SPI_DISABLE();
}

// clear register bit
__inline void NRF_CLR_REGBIT(uint8 addr, uint8 mask) {
	uint8 buffer;
	
	NRF_READ(addr, &buffer, 1);
	buffer &= ~mask;
	NRF_WRITE(addr | NRFCMD_WREG, &buffer, 1);
}

// set register bit
__inline void NRF_SET_REGBIT(uint8 addr, uint8 mask) {
	uint8 buffer;
	
	NRF_READ(addr, &buffer, 1);
	buffer |= mask;
	NRF_WRITE(addr | NRFCMD_WREG, &buffer, 1);
}

// write register
#define NRF_WRITE_REG(reg, value) \
	NRF_SPI_ENABLE(); \
	NRF_SPI_WRITE(reg | NRFCMD_WREG); \
	NRF_SPI_WRITE(value); \
	NRF_SPI_DISABLE();

// enable && disable tx end interrupt	
#define NRF_ENABLE_TXINT() \
	NRF_CLR_REGBIT(NRFREG_CFG, 1 << 5)
#define NRF_DISABLE_TXINT() \
	NRF_SET_REGBIT(NRFREG_CFG, 1 << 5)

// enable && disable max rt interrupt
#define NRF_ENABLE_RXINT() \
	NRF_CLR_REGBIT(NRFREG_CFG, 1 << 6)
#define NRF_DISABLE_RXINT() \
	NRF_SET_REGBIT(NRFREG_CFG, 1 << 6)

// enable && disable tx end interrupt	
#define NRF_ENABLE_MAXRTINT() \
	NRF_CLR_REGBIT(NRFREG_CFG, 1 << 4)
#define NRF_DISABLE_MAXRTINT() \
	NRF_SET_REGBIT(NRFREG_CFG, 1 << 4)
	
// enable && disable CRC, force enable when auto-ack enable
#define NRF_ENABLE_CRC() \
	NRF_SET_REGBIT(NRFREG_CFG, 1 << 3)
#define NRF_DISABLE_CRC() \
	NRF_CLR_REGBIT(NRFREG_CFG, 1 << 3)

// software power-up 
#define NRF_POWER_UP() \
	NRF_SET_REGBIT(NRFREG_CFG, 1 << 1) 
	
// sofrware power-down
#define NRF_POWER_DOWN() \
	NRF_CLR_REGBIT(NRFREG_CFG, 1 << 1)

// setupt tx mode
#define NRF_SETUP_TXMODE() \
	NRF_CLR_REGBIT(NRFREG_CFG, 1 << 0)

// setup rx mode
#define NRF_SETUP_RXMODE() \
	NRF_SET_REGBIT(NRFREG_CFG, 1 << 0)
	
// enable auto-ack
#define NRF_ENABLE_AUTOACK(mask) \
	NRF_SET_REGBIT(NRFREG_EN_AA, mask & 0x3f)
	
// enable auto-ack, pipe = [0:5]
#define NRF_DISABLE_AUTOACK(mask) \
	NRF_CLR_REGBIT(NRFREG_EN_AA, mask & 0x3f)

// enable PIPE, pipe = [0:5], pipe_length = [1:32]
#define NRF_ENABLE_PIPE(pipe, pipe_length) \
	NRF_SET_REGBIT(NRFREG_EN_RXADDR, (1 << pipe)); \
	NRF_WRITE_REG(NRFREG_RX_PW_P0 + pipe, pipe_length)

// disable PIPE, pipe = [0:5]
#define NRF_DISABLE_PIPE(pipe) \
	NRF_CLR_REGBIT(NRFREG_EN_RXADDR, (1 << pipe)); \
	NRF_WRITE_REG(NRFREG_RX_PW_P0 + pipe, 0x00);

// setup rx/tx address widths
#define NRF_SETUP_ADDR_WIDTHS(widths) \
	NRF_WRITE_REG(NRFREG_SETUP_AW, widths)

// setup auto-repeat	
#define NRF_SETUP_RETR(wait_time, auto_retransmit_count) \
	NRF_WRITE_REG(NRFREG_SETUP_RETR, wait_time | auto_retransmit_count)
	
// setup RF channel
#define NRF_SETUP_CH(channel) \
	NRF_WRITE_REG(NRFREG_SETUP_RFCH, channel & 0x7f)
	
// setup data rate
#define NRF_SETUP_DRATE(rate) \
	NRF_CLR_REGBIT(NRFREG_RFSETUP, (1 << 3)); \
	NRF_SET_REGBIT(NRFREG_RFSETUP, (rate << 3))
		
// setup output power
#define NRF_SETUP_RFPOWER(level) \
	NRF_CLR_REGBIT(NRFREG_RFSETUP, (3 << 1)); \
	NRF_SET_REGBIT(NRFREG_RFSETUP, (level << 1))

// LNA gain
#define NRF_SETUP_LNA(enable) \
	NRF_CLR_REGBIT(NRFREG_RFSETUP, (1 << 0)); \
	NRF_SET_REGBIT(NRFREG_RFSETUP, (enable << 1))

// setup rx address
#define NRF_SETUP_RXADDR_P0(addr_buf, len) \
	NRF_WRITE(NRFREG_RX_ADDR_P0 | NRFCMD_WREG, addr_buf, len)
#define NRF_SETUP_RXADDR_P1(addr_buf, len) \
	NRF_WRITE(NRFREG_RX_ADDR_P1 | NRFCMD_WREG, addr_buf, len)
#define NRF_SETUP_RXADDR_P2(address) \
	NRF_WRITE_REG(NRFREG_RX_ADDR_P2, address)
#define NRF_SETUP_RXADDR_P3(address) \
	NRF_WRITE_REG(NRFREG_RX_ADDR_P3, address)
#define NRF_SETUP_RXADDR_P4(address) \
	NRF_WRITE_REG(NRFREG_RX_ADDR_P4, address)
#define NRF_SETUP_RXADDR_P5(address) \
	NRF_WRITE_REG(NRFREG_RX_ADDR_P5, address)

// setup tx address
#define NRF_SETUP_TXADDR(addr_buf, len) \
	NRF_WRITE(NRFREG_TX_ADDR | NRFCMD_WREG, addr_buf, len)

// nrf state machine
typedef enum {
	NRF_STATE_RX,	// RX mode
	NRF_STATE_TX,	// TX mode
	NRF_STATE_PWDN, // POWER DOWN mode
} nrf_state_t;


extern int nrf_open(void);
extern int nrf_write(uint8 buffer[32], boolean wait_ack);
#if (!RF_INT_MODE)
extern int nrf_read(uint8 buffer[32], uint8 *pipe);
#endif
extern int nrf_close(void);


#endif // NRF24L01
