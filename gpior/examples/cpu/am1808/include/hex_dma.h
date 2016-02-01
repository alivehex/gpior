#ifndef _HEX_DMA_H_
#define _HEX_DMA_H_

extern void DMA_init(void);
extern void DMA_register_compleate_callback(unsigned int dma_channel, 
	void (*callback)(unsigned int, unsigned int));
extern void DMA_request_channel(unsigned int tcc_num);

#endif /* ifndef _HEX_DMA_H_ */
