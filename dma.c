#include "dma_defs.h"
#include "dma.h"

#include "mem.h"
#include "barrier.h"

void dma_reset(unsigned int chan) {
	if(chan > 14)
		return;

	*CS(chan) = RESET;
	while(*CS(chan) & RESET);
}

int dma_initiate(unsigned int chan, struct dma_cb *ctrl_block) {
	if(chan > 14)
		return 1;

	ctrl_block->ti |= DMA_TI_INTEN;
	ctrl_block->nextconbk = 0;

	dsb();
	*CONBLK_AD(chan) = virt_to_phy(ctrl_block);
	*CS(chan) = ACTIVE | END | INT;

	// For the moment, wait until the transfer completes.
	while((*CS(chan) & (INT | ERROR)) == 0);
	if(*CS(chan) & ERROR)
		return 2;

	return 0;
}

void dma_reset_all() {
	for(int i=0 ; i<=14 ; ++i)
		*CS(i) = RESET;

	int reset = 0;
	while(reset != 0x7fff) {
		for(int i=0 ; i<= 14 ; ++i) {
			if(reset & (1<<i))
				continue;
			if((*CS(i) & RESET) == 0)
				reset |= (1<<i);
		}
	}
}
