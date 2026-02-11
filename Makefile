CC = aarch64-elf-gcc
AS = aarch64-elf-as
LD = aarch64-elf-ld
QEMU = qemu-system-aarch64

KERNEL_SRC = kernel
DRIVERS_SRC = drivers
BUILD_DIR := build
SYSROOT := sysroot

KERNEL_C := $(shell find $(KERNEL_SRC) -type f -name '*.c')
KERNEL_S := $(shell find $(KERNEL_SRC) -type f -name '*.S')
DRIVERS_C := $(shell find $(DRIVERS_SRC) -type f -name '*.c')
DRIVERS_S := $(shell find $(DRIVERS_SRC) -type f -name '*.S')

KERNEL_OBJ := $(patsubst $(KERNEL_SRC)/%.c, $(BUILD_DIR)/kernel/%.o, $(KERNEL_C))
KERNEL_OBJ += $(patsubst $(KERNEL_SRC)/%.S, $(BUILD_DIR)/kernel/%.o, $(KERNEL_S))
DRIVERS_OBJ := $(patsubst $(DRIVERS_SRC)/%.c, $(BUILD_DIR)/drivers/%.o, $(DRIVERS_C))
DRIVERS_OBJ += $(patsubst $(DRIVERS_SRC)/%.S, $(BUILD_DIR)/drivers/%.o, $(DRIVERS_S))

ALL_OBJ := $(KERNEL_OBJ) $(DRIVERS_OBJ)

CFLAGS := -Wall -Wextra -Werror -ffreestanding -nostdlib -std=gnu23 -O0 -g3 -ggdb -fno-omit-frame-pointer -fno-inline -mcpu=cortex-a57 -I kernel -I drivers -I kernel/libk/includes -mgeneral-regs-only  -MMD -MP
ASFLAGS := -mcpu=cortex-a57
LDFLAGS := -T $(KERNEL_SRC)/linker.ld -nostdlib
QEMUFLAGS := -M virt -cpu cortex-a57 -nographic

all: kernel.elf


$(BUILD_DIR) $(BUILD_DIR)/kernel $(BUILD_DIR)/drivers:
	mkdir -p $@

$(SYSROOT)/boot $(SYSROOT)/usr/include $(SYSROOT)/usr/lib:
	mkdir -p $@

kernel.elf: $(ALL_OBJ) | $(BUILD_DIR) $(SYSROOT)/boot
	$(LD) $(LDFLAGS) -o $(SYSROOT)/boot/kernel.elf $^
	@echo "=== Build complete: $(SYSROOT)/boot/kernel.elf ==="

install: kernel.elf
	@echo "=== Kernel installed to $(SYSROOT)/boot/ ==="

$(BUILD_DIR)/drivers/%.o: $(DRIVERS_SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/%.o: $(KERNEL_SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/%.o: $(KERNEL_SRC)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -g -c $< -o $@

$(BUILD_DIR)/drivers/%.o: $(DRIVERS_SRC)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -g -c $< -o $@

-include $(ALL_OBJ:.o=.d)

install-headers: | $(SYSROOT)/usr/include
	@echo "=== No headers to install yet ==="

qemu: kernel.elf
	$(QEMU) $(QEMUFLAGS) -kernel $(SYSROOT)/boot/kernel.elf 

qemu-gdb: kernel.elf
	$(QEMU) $(QEMUFLAGS) -kernel $(SYSROOT)/boot/kernel.elf -S -s

clean:
	rm -rf $(BUILD_DIR) kernel.elf

.PHONY: all clean qemu qemu-lldb

