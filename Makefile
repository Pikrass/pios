.PHONY: clean distclean

MAIN=main
SRC=main.s led.s mailbox.s screen.s font.s term.s

LD=arm-none-eabi-ld
OBJCOPY=arm-none-eabi-objcopy
AS=arm-none-eabi-as

all: kernel.img

%.o: %.s
	$(AS) -mfloat-abi=hard -mcpu=arm1176jz-s $< -o $@

%.elf: $(SRC:.s=.o)
	$(LD) -T lscript $^ -o $@

kernel.img: $(MAIN).elf
	$(OBJCOPY) $< -O binary $@

clean:
	rm -f *.o *.elf
distclean: clean
	rm -f kernel.img


# Dependencies
main.s: logo.bin
font.s: font.bin
