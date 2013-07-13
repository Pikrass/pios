#ifndef DMA_H
#define DMA_H

#define IO_TO_BUS(x)     (void*)((unsigned int)(x) | 0x7e000000)
#define ARM_TO_BUS(x)    (void*)((unsigned int)(x) | 0xc0000000)

// Assigned channels
#define DMA_CHAN_EMMC  0

// Control block
struct dma_cb {
	unsigned int ti;
	void *source_ad;
	void *dest_ad;
	unsigned int txfr_len;
	unsigned int stride;
	void *nextconbk;
};

// Transfer Information
#define DMA_TI_INTEN       1<<0
#define DMA_TI_WAIT_RESP   1<<3
#define DMA_TI_DEST_INC    1<<4
#define DMA_TI_DEST_WIDTH  1<<5
#define DMA_TI_DEST_DREQ   1<<6
#define DMA_TI_SRC_INC     1<<8
#define DMA_TI_SRC_WIDTH   1<<9
#define DMA_TI_SRC_DREQ    1<<10

#define DMA_TI_PERMAP_EMMC DMA_TI_PERMAP(11)
#define DMA_TI_PERMAP(x)   (x)<<16

// Functions
void dma_reset(unsigned int chan);
int dma_initiate(unsigned int chan, struct dma_cb *ctrl_block);

#endif
