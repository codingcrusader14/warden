CC  = aarch64-elf-gcc
AS  = aarch64-elf-as
LD  = aarch64-elf-ld

CFLAGS = -ffreestanding -nostdlib -Wall -Werror
LDFLAGS = -T src/linker.ld

all: kernel.elf

kernel.elf: src/boot.o src/main.o
	$(LD) $(LDFLAGS) -o kernel.elf src/boot.o src/main.o

src/main.o: src/main.c
	$(CC) $(CFLAGS) -c -std=gnu23 src/main.c -o src/main.o

src/boot.o: src/boot.S
	$(AS) src/boot.S -o src/boot.o

make qemu:
	qemu-system-aarch64 -M virt -cpu cortex-a57 -kernel kernel.elf -nographic

clean:
	rm -f src/*.o kernel.elf
	


