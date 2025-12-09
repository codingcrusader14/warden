CC = aarch64-elf-gcc
AS = aarch64-elf-as
LD = aarch64-elf-ld
QEMU = qemu-system-aarch64

KERNEL_SRC = src/kernel
DRIVERS_SRC = src/drivers
BUILD_DIR := build

KERNEL_C := $(wildcard $(KERNEL_SRC)/*.c)
KERNEL_S := $(wildcard $(KERNEL_SRC)/*.S)
DRIVERS_C := $(shell find $(DRIVERS_SRC) -type f -name '*.c')
DRIVERS_S := $(shell find $(DRIVERS_SRC) -type f -name '*.s')

KERNEL_OBJ := $(patsubst $(KERNEL_SRC)/%.c, $(BUILD_DIR)/kernel/%.o, $(KERNEL_C))
KERNEL_OBJ += $(patsubst $(KERNEL_SRC)/%.S, $(BUILD_DIR)/kernel/%.o, $(KERNEL_S))
DRIVERS_OBJ := $(patsubst $(DRIVERS_SRC)/%.c, $(BUILD_DIR)/drivers/%.o, $(DRIVERS_C))
DRIVERS_OBJ += $(patsubst $(DRIVERS_SRC)/%.S, $(BUILD_DIR)/drivers/%.o, $(DRIVERS_S))

ALL_OBJ := $(KERNEL_OBJ) $(DRIVERS_OBJ)

CFLAGS := -Wall -Wextra -Werror -ffreestanding -nostdlib -std=gnu23 -O2 -mcpu=cortex-a57 -I src/kernel -I src/drivers -MMD -MP
ASFLAGS := -mcpu=cortex-a57
LDFLAGS := -T $(KERNEL_SRC)/linker.ld -nostdlib
QEMUFLAGS := -M virt -cpu cortex-a57 -nographic

all: kernel.elf

$(BUILD_DIR) $(BUILD_DIR)/kernel $(BUILD_DIR)/drivers:
	mkdir -p $@

kernel.elf: $(ALL_OBJ) | $(BUILD_DIR)
	$(LD) $(LDFLAGS) -o $@ $^
	@echo "=== Build complete: kernel.elf ==="

$(BUILD_DIR)/kernel/%.o: $(KERNEL_SRC)/%.c | $(BUILD_DIR)/kernel
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/drivers/%.o: $(DRIVERS_SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/%.o: $(KERNEL_SRC)/%.S | $(BUILD_DIR)/kernel
	$(CC) $(ASFLAGS) -c $< -o $@

$(BUILD_DIR)/drivers/%.o: $(DRIVERS_SRC)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -c $< -o $@

-include $(ALL_OBJ:.o=.d)

qemu: kernel.elf
	$(QEMU) $(QEMUFLAGS) -kernel kernel.elf

qemu-gdb: kernel.elf
	$(QEMU) $(QEMUFLAGS) -kernel kernel.elf -S -s

clean:
	rm -rf $(BUILD_DIR) kernel.elf

.PHONY: all clean qemu qemu-lldb

