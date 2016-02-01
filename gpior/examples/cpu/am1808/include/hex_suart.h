#ifndef _HEX_SUART_H_
#define _HEX_SUART_H_

int uart_open(int baudrate, int word_len, int cache_size);
int uart_rxd_enable(char *buf, int len, int timeout);
int uart_txd_enable(char *buf, int len, int timeout);
int uart_close(void);

void atchip_reset(int ms);
void atchip_clock_enable(void);
void atchip_clock_disable(void);

#endif /* ifndef _HEX_SUART_H_ */
