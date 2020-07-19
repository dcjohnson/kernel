COMPILER=/usr/local/bin/gcc-arm-none-eabi-6-2017-q1-update/bin/arm-none-eabi-gcc
FLAGS=-mcpu=arm1176jzf-s -fpic -ffreestanding
BUILD=$(COMPILER) $(FLAGS)

kernel.elf: ./boot.o ./kernel.o ./linker.ld
	$(COMPILER) -T linker.ld -o kernel.elf -ffreestanding -O2 -nostdlib boot.o kernel.o

kernel7.img: kernel.elf
	/usr/local/bin/gcc-arm-none-eabi-6-2017-q1-update/bin/arm-none-eabi-objcopy $< -O binary $@

boot.o: ./boot.S
	$(BUILD) -c boot.S -o boot.o

kernel.o: ./kernel.c
	$(BUILD) -std=gnu99 -c kernel.c -o kernel.o -O2 -Wall -Wextra 

clean:
	rm -v boot.o kernel.elf kernel.o kernel7.img

