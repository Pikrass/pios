.PHONY: clean distclean

MAIN=main
ASM=init.s led.s mailbox.s screen.s font.s term.s debug.s
C=main.c sd.c

LD=arm-none-eabi-ld
OBJCOPY=arm-none-eabi-objcopy
AS=arm-none-eabi-as
CC=arm-none-eabi-gcc

ASFLAGS=-mfloat-abi=hard -mcpu=arm1176jz-s
CFLAGS=-O0 -std=c99 $(ASFLAGS)

all: kernel.img

%.o: %.s
	$(AS) $(ASFLAGS) $< -o $@

%.elf: $(ASM:.s=.o) $(C:.c=.o)
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
init.s: logo.bin
font.s: font.bin

include .mkdepends
