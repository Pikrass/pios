.PHONY: clean distclean

ASM=led.s mailbox.s screen.s font.s term.s debug.s logo.s
C=init.c main.c sd.c dma.c atags.c mem.c term_printf.c

LD=arm-none-eabi-ld
OBJCOPY=arm-none-eabi-objcopy
AS=arm-none-eabi-as
CC=arm-none-eabi-gcc

ASFLAGS=-mfloat-abi=hard -mcpu=arm1176jz-s
CFLAGS=-O2 -std=c99 -nostdlib $(ASFLAGS)

all: kernel.img

sd.o: sd.c
	$(CC) $(CFLAGS) -O0 -c -o $@ $<

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

kernel.elf: $(C:.c=.o) $(ASM:.s=.o)
	$(LD) -T lscript1 $^ -o tmp.elf
	$(LD) -T lscript2 tmp.elf -o $@

kernel.img: kernel.elf
	$(OBJCOPY) $< --change-section-lma '.main.*-0xbfff7000' -O binary $@

clean:
	rm -f *.o *.elf
distclean: clean
	rm -f kernel.img


.mkdepends:
	gcc -MM $(C) > $@


# Dependencies
font.s: font.bin

include .mkdepends
