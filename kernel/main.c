#include "../drivers/qemu/pl011.h" 
#include "../drivers/qemu/timer.h"
#include "../drivers/qemu/gic.h"
#include "libk/includes/stdio.h"
#include "vmm.h"
#include "pmm.h"
#include "schedule.h" 
#include "../drivers/virtio.h"
#include "../fs/fat32.h"

void kernel_main(void) {
    uart_init();
    pmm_init(QEMU_DRAM_START, QEMU_DRAM_END);
    vmm_init();
    virtio_disk_init();
    init_disk_bpb();
    scheduler_init();
    scheduler_boot();
    gic_init();
    timer_init();
    scheduler_start();
}
