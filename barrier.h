#ifndef BARRIER_H
#define BARRIER_H

#define dmb() __asm("mcr p15, 0, %0, c7, c10, 5" : : "r" (0) : "memory")
#define dsb() __asm("mcr p15, 0, %0, c7, c10, 4" : : "r" (0) : "memory")

#endif
