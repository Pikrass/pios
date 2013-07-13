#ifndef DMA_DEFS_H
#define DMA_DEFS_H

#define DMA_BASE         (volatile void*)(0x20007000)
#define CS(x)            (volatile int*)(DMA_BASE + (x) * 0x100 + 0x00)
#define CONBLK_AD(x)     (volatile void**)(DMA_BASE + (x) * 0x100 + 0x04)
#define TI(x)            (volatile int*)(DMA_BASE + (x) * 0x100 + 0x08)
#define SOURCE_AD(x)     (volatile int*)(DMA_BASE + (x) * 0x100 + 0x0c)
#define DEST_AD(x)       (volatile int*)(DMA_BASE + (x) * 0x100 + 0x10)
#define TXFR_LEN(x)      (volatile int*)(DMA_BASE + (x) * 0x100 + 0x14)
#define STRIDE(x)        (volatile int*)(DMA_BASE + (x) * 0x100 + 0x18)
#define NEXTCONBK(x)     (volatile int*)(DMA_BASE + (x) * 0x100 + 0x1c)
#define DEBUG(x)         (volatile int*)(DMA_BASE + (x) * 0x100 + 0x20)
#define INT_STATUS       (volatile int*)(DMA_BASE + 0xfe0)
#define ENABLE           (volatile int*)(DMA_BASE + 0xff0)

// CS Control and Status
#define ACTIVE  1<<0
#define END     1<<2
#define INT     1<<2
#define ERROR   1<<8
#define RESET   1<<31

#endif
