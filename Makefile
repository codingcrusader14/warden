CC = aarch64-elf-gcc
AS = aarch64-elf-as
LD = aarch64-elf-ld
QEMU = qemu-system-aarch64

KERNEL_SRC = kernel
DRIVERS_SRC = drivers
FS_SRC = fs
USER_SRC = user
BUILD_DIR := build
SYSROOT := sysroot

KERNEL_C := $(shell find $(KERNEL_SRC) -type f -name '*.c')
KERNEL_S := $(shell find $(KERNEL_SRC) -type f -name '*.S')
DRIVERS_C := $(shell find $(DRIVERS_SRC) -type f -name '*.c')
DRIVERS_S := $(shell find $(DRIVERS_SRC) -type f -name '*.S')
USER_C := $(USER_SRC)/init.c
USER_S := $(USER_SRC)/syscall_stub.S
FS_C := $(shell find $(FS_SRC) -type f -name '*.c')
FS_S := $(shell find $(FS_SRC) -type f -name '*.S')

KERNEL_OBJ := $(patsubst $(KERNEL_SRC)/%.c, $(BUILD_DIR)/kernel/%.o, $(KERNEL_C))
KERNEL_OBJ += $(patsubst $(KERNEL_SRC)/%.S, $(BUILD_DIR)/kernel/%.o, $(KERNEL_S))
DRIVERS_OBJ := $(patsubst $(DRIVERS_SRC)/%.c, $(BUILD_DIR)/drivers/%.o, $(DRIVERS_C))
DRIVERS_OBJ += $(patsubst $(DRIVERS_SRC)/%.S, $(BUILD_DIR)/drivers/%.o, $(DRIVERS_S))
USER_OBJ := $(patsubst $(USER_SRC)/%.c, $(BUILD_DIR)/user/%.o, $(USER_C))
USER_OBJ += $(patsubst $(USER_SRC)/%.S, $(BUILD_DIR)/user/%.o, $(USER_S))
FS_OBJ := $(patsubst $(FS_SRC)/%.c, $(BUILD_DIR)/$(FS_SRC)/%.o, $(FS_C))
FS_OBJ += $(patsubst $(FS_SRC)/%.S, $(BUILD_DIR)/$(FS_SRC)/%.o, $(FS_S))

USER_LIB := $(BUILD_DIR)/user/user_libc.o
ALL_KERNEL_OBJ := $(KERNEL_OBJ) $(DRIVERS_OBJ) $(FS_OBJ)

CFLAGS := -Wall -Werror -Wextra -ffreestanding -nostdlib -std=gnu2x -O0 -g3 -ggdb -fno-omit-frame-pointer -fno-inline -mcpu=cortex-a57 -march=armv8-a -I kernel -I drivers -I $(FS_SRC) -I kernel/libk/includes -mgeneral-regs-only -MMD -MP
USER_CFLAGS := -Wall -Werror -Wextra -ffreestanding -nostdlib -std=gnu2x -O0 -g3 -mcpu=cortex-a57 -march=armv8-a -mgeneral-regs-only -I user -MMD -MP
USER_PROGRAMS := ls pwd clear cat wc echo touch rm mkdir bench execf


ASFLAGS := -mcpu=cortex-a57
LDFLAGS := -T $(KERNEL_SRC)/linker.ld -nostdlib
USER_LDFLAGS := -T $(USER_SRC)/user.ld -nostdlib
QEMUFLAGS := -M virt -m 512M -cpu cortex-a57 -nographic

$(BUILD_DIR) $(BUILD_DIR)/kernel $(BUILD_DIR)/drivers:
	mkdir -p $@

$(SYSROOT)/boot $(SYSROOT)/usr/include $(SYSROOT)/usr/lib:
	mkdir -p $@

kernel.elf: $(ALL_KERNEL_OBJ) | init.bin $(BUILD_DIR) $(SYSROOT)/boot
	$(LD) $(LDFLAGS) -o $(SYSROOT)/boot/kernel.elf $^
	@echo "=== Build complete: $(SYSROOT)/boot/kernel.elf ==="

init.elf: $(USER_OBJ) $(USER_LIB) | $(BUILD_DIR)/user
	$(LD) $(USER_LDFLAGS) -o $(BUILD_DIR)/user/init.elf $^
	@echo "=== Userspace build complete ==="

init.bin: init.elf
	aarch64-elf-objcopy -O binary $(BUILD_DIR)/user/init.elf $(BUILD_DIR)/user/init.bin

all: init.bin kernel.elf

install: kernel.elf
	@echo "=== Kernel installed to $(SYSROOT)/boot/ ==="

$(BUILD_DIR)/drivers/%.o: $(DRIVERS_SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/drivers/%.o: $(DRIVERS_SRC)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -g -c $< -o $@

$(BUILD_DIR)/kernel/%.o: $(KERNEL_SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/kernel/%.o: $(KERNEL_SRC)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -g -c $< -o $@

$(BUILD_DIR)/user/%.o: $(USER_SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(USER_CFLAGS) -c $< -o $@

$(BUILD_DIR)/user/%.o: $(USER_SRC)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(ASFLAGS) -g -c $< -o $@

$(BUILD_DIR)/$(FS_SRC)/%.o: $(FS_SRC)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR)/$(FS_SRC)/%.o: $(FS_SRC)/%.S
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) -g -c $< -o $@

$(BUILD_DIR)/kernel/user_init.o: init.bin

-include $(ALL_KERNEL_OBJ:.o=.d)
-include $(USER_OBJ:.o=.d)
-include $(USER_LIB:.o=.d)

$(BUILD_DIR)/user/%.elf: $(USER_SRC)/programs/%.c $(BUILD_DIR)/user/syscall_stub.o $(USER_LIB)
	@mkdir -p $(dir $@)
	$(CC) $(USER_CFLAGS) -c $< -o $(BUILD_DIR)/user/$*.o
	$(LD) $(USER_LDFLAGS) -o $@ $(BUILD_DIR)/user/$*.o $(BUILD_DIR)/user/syscall_stub.o $(USER_LIB)

install-user: $(patsubst %, $(BUILD_DIR)/user/%.elf, $(USER_PROGRAMS))
	for f in $^; do mcopy -i disk.img -D o $$f ::$$(basename $$f); done

install-headers: | $(SYSROOT)/usr/include
	@echo "=== No headers to install yet ==="

disk.img:
	dd if=/dev/zero of=disk.img bs=1M count=256
	mkfs.fat -F 32 -s 8 disk.img

qemu: kernel.elf disk.img
	$(QEMU) $(QEMUFLAGS) -kernel $(SYSROOT)/boot/kernel.elf -drive file=disk.img,if=none,format=raw,id=hd0 -device virtio-blk-device,drive=hd0,

qemu-gdb: kernel.elf disk.img
	$(QEMU) $(QEMUFLAGS) -kernel $(SYSROOT)/boot/kernel.elf -drive file=disk.img,if=none,format=raw,id=hd0 -device virtio-blk-device,drive=hd0, -S -s

dumpdtb:
	$(QEMU) $(QEMUFLAGS) -machine virt,dumpdtb=virt.dtb
	dtc -I dtb -O dts -o virt.dts virt.dtb
	@echo "Device tree dumped to virt.dts."

clean:
	rm -rf $(BUILD_DIR) kernel.elf

.PHONY: all clean qemu qemu-lldb
