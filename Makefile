.PHONY: clean distclean

SRC=led2

LD=arm-none-eabi-ld
OBJCOPY=arm-none-eabi-objcopy
AS=arm-none-eabi-as

all: kernel.img

%.o: %.s
	$(AS) -mfloat-abi=hard -mcpu=arm1176jz-s $< -o $@

%.elf: %.o
	$(LD) -T lscript --no-gc-sections $< -o $@

kernel.img: $(SRC).elf
	$(OBJCOPY) $< -O binary $@

clean:
	rm -f *.o *.elf
distclean: clean
	rm -f kernel.img
