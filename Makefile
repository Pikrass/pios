.PHONY: clean distclean

MAIN=main
ASM=led.s mailbox.s screen.s font.s term.s debug.s logo.s
C=init.c main.c sd.c dma.c atags.c

LD=arm-none-eabi-ld
OBJCOPY=arm-none-eabi-objcopy
AS=arm-none-eabi-as
CC=arm-none-eabi-gcc

ASFLAGS=-mfloat-abi=hard -mcpu=arm1176jz-s
CFLAGS=-O2 -std=c99 $(ASFLAGS)

all: kernel.img

sd.o: sd.c
	$(CC) $(CFLAGS) -O0 -c -o $@ $<

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%.elf: $(C:.c=.o) $(ASM:.s=.o)
	$(LD) -T lscript $^ -o $@

kernel.img: $(MAIN).elf
	$(OBJCOPY) $< -O binary $@

clean:
	rm -f *.o *.elf
distclean: clean
	rm -f kernel.img


.mkdepends:
	gcc -MM $(C) > $@


# Dependencies
font.s: font.bin

include .mkdepends
