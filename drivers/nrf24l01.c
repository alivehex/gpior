/******
	drivers for nrf24l01
	2011-3-14
	alivehex@gmail.com
******/


#include "iocfg.h"

#include <string.h>

#if (GPIOR_RF24L)

#undef TRACE_LEVEL
#undef TRACE_TITLE
#undef TRACE_TITLE
#define TRACE_LEVEL	TRACE_LEVEL_DEBUG
#define TRACE_NAME  TRACE_NAME_DISABLE
#define TRACE_TITLE "RF"
#include "trace.h"

static volatile nrf_state_t	nrf_state;
static volatile boolean nrf_tx_nack = FALSE;
static void RF_IRQ_HANDLER(void);
static uint8 RF_CACHE[32];
static uint8 RF_PIPE;
extern void NRF_RXEND_CALLBACK(uint8 * buffer, uint8 len, uint8 pipe);



// IOs initalization
static void NRF_IO_INIT() {
	GPIOR_CFGIO_INPUT(RF_PORT, RF_IRQ_PIN);
	GPIOR_CFGIO_OUTPUT(RF_PORT, RF_CE_PIN);
	SPI_open(0, 1382400);
	RF_PLAT_INIT();
}

int nrf_open(void) {

#if (TRACE_LEVEL > TRACE_LEVEL_NO_TRACE)
	uint8 i;
	uint8 buffer[32];
#endif	
	
	NRF_IO_INIT();
	NRF_CHIP_DISABLE();
	NRF_POWER_ON();		// hardware power on
	NRF_DELAYUS(15000); // 15ms power on reset, Min 10.3ms

#if (TRACE_LEVEL > TRACE_LEVEL_NO_TRACE)	
	TRACE_DEBUG("output register value: ");
	for(i = 0; i < 0x1e; i ++) {
		NRF_READ(i, &buffer[i], 1);
		TRACE_DEBUG_WP("0x%x ", buffer[i]);
	}	
#	endif

	NRF_SETUP_RXMODE(); // start with rx mode
	NRF_ENABLE_PIPE(0, 32); // pipe fifo width = 32
	NRF_ENABLE_AUTOACK(1 << 0); // enable pipe 0 auto-ack
	NRF_SETUP_RETR(0, 3); // 250us, 3 times retries
	NRF_ENABLE_MAXRTINT();
	NRF_SETUP_DRATE(NRF_DRATE_2M); // 2Mbps
	NRF_SETUP_ADDR_WIDTHS(NRF_ADDR_WIDTHS_5BYTES);
	NRF_POWER_UP();		// software power on
	NRF_DELAYUS(2000); 	// min 1.5ms
	nrf_state = NRF_STATE_RX;
	
#if (TRACE_LEVEL > TRACE_LEVEL_NO_TRACE)	
	TRACE_DEBUG("output register value: ");
	for(i = 0; i < 0x1e; i ++) {
		NRF_READ(i, &buffer[i], 1);
		TRACE_DEBUG_WP("0x%x ", buffer[i]);
	}	
#	endif
	NRF_CHIP_ENABLE(); // listen to package
	return 0;
}

int nrf_close() {
	NRF_CHIP_DISABLE();
	NRF_POWER_DOWN();
	NRF_POWER_OFF();
	return 0;
}

/*
 * @ write one package 32 bytes
 */
int nrf_write(uint8 buffer[32], boolean wait_ack) {
	// one package always be 32 bytes
#if (RF_INT_MODE)
	while(nrf_state == NRF_STATE_TX); // wait for last transfer
#endif
	NRF_CHIP_DISABLE();
	NRF_SETUP_TXMODE();
	NRF_WRITE(NRFCMD_WTX_PAYLOAD, buffer, 32);
	/****** FIXME:
		write tx fifo must finish in one transfer
	******/
	nrf_state = NRF_STATE_TX;
	/* start transmit */
	NRF_CHIP_ENABLE();
#if (RF_INT_MODE)
	while(nrf_state == NRF_STATE_TX); // wait for transfer end
#else
	while(GPIOR_GETIO(RF_PORT, RF_IRQ_PIN));
    RF_IRQ_HANDLER();
#endif
	if(wait_ack == TRUE) { 
		if(nrf_tx_nack == TRUE) { // no ack fram rcv
			return -1;
		}
	}
	return 0;
}

void RF_IRQ_HANDLER(void) {// nrf_interrupt_handler
	uint8 status;
	
	//dbg_str("INT1 interrupt: ");
	NRF_CHIP_DISABLE();
	NRF_READ(NRFREG_STATUS, &status, 1);
	if(status & (1 << 5)) { // TX end interrupt
		TRACE_DEBUG("TX END");
		NRF_SET_REGBIT(NRFREG_STATUS, (1 << 5));
		nrf_tx_nack = FALSE;
	}
	if(status & (1 << 6)) { // RX end interrupt
		TRACE_DEBUG("RX END");
		NRF_SET_REGBIT(NRFREG_STATUS, (1 << 6));
		NRF_READ(NRFCMD_RRX_PAYLOAD, RF_CACHE, 32);
		RF_PIPE = (status >> 1) & 7;
#	if (RF_INT_MODE)
		NRF_RXEND_CALLBACK(RF_CACHE, RF_PIPE);
#	endif
	}
	if(status & (1 << 4)) {
		TRACE_DEBUG("TX LOST");
		NRF_SET_REGBIT(NRFREG_STATUS, (1 << 4));
		NRF_WRITE(NRFCMD_FLUSH_TX, NULL, 0); // flush all fifo datas
		nrf_tx_nack = TRUE;
	}
	NRF_SETUP_RXMODE();
	nrf_state = NRF_STATE_RX;
	NRF_CHIP_ENABLE();
}

#if (!RF_INT_MODE)
/*
 * @ RF read 32 bytes 
 */
int nrf_read(uint8 buffer[32], uint8 *pipe) {
	while(GPIOR_GETIO(RF_PORT, RF_IRQ_PIN));
    RF_IRQ_HANDLER();
	*pipe = RF_PIPE;
	memcpy(buffer, RF_CACHE, 32);
	return 0;
}
#endif 



/* EXAMPLE CODE TO ENABLE AUTO ACK:
	NRF_ENABLE_AUTOACK(1 << 0);
	NRF_SETUP_RETR(0, 3); // 250us, 3 times retries
	NRF_ENABLE_MAXRTINT();
***********************************/
#if (RF_INT_MODE)
void NRF_RXEND_CALLBACK(uint8 buffer[32], uint8 pipe) {
	int i;
	
	//dbg_hex(len & 0xff);
	//dbg_str(" Bytes received(pipe=");
	//dbg_hex(pipe & 0xff);
	//dbg_str("): ");
	for(i = 0; i < len; i ++) {
		//dbg_hex(buffer[i] & 0xff);
		//dbg_str(" ");
	}
	//dbg_str("\r\n");
}
#endif // if (RF_INT_MODE)

#if (0)
void nrf_test() {
	uint8 buffer[32];
	int i;
	
	for(i = 0; i < sizeof(buffer); i ++)
		buffer[i] = i;
	nrf_open();
#	if(1)
	while(1) {
		//dbg_str("write data\r\n");
		nrf_write(buffer, TRUE);
		nrf_write(buffer, TRUE);
		nrf_write(buffer, TRUE);
		NRF_DELAYUS(1000000);
	}
#	else
	while(1);
#	endif
}
#endif

#else
#	error SET GPIOR_RF24L TO 1 TO ENABLE NRF DRIVER
#endif // if (GPIOR_RF24L)








