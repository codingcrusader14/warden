#include "virtio.h" 
#include "../kernel/libk/includes/stdio.h"
#include "../kernel/libk/includes/string.h"
#include "../kernel/mmu_defs.h"
#include "../kernel/spinlock.h"
#include "../kernel/pmm.h"
#include <stdbool.h>

static inline uint32 virtio_read(uint64 base, uint16 offset) {
  return *(volatile uint32*)(base + offset);
}

static inline void virtio_write(uint64 base, uint16 offset, uint32 value) {
  *(volatile uint32*)(base + offset) = value;
}

static struct disk {
  uint64 base_addr;
  virtq_desc *desc;
  virtq_avail *avail; // driver to device
  virtq_used *used; // device to driver
  virtio_blk_req ops[VIRTIO_QUEUE_SIZE];
  uint16 last_used_idx;
  bool free_q[VIRTIO_QUEUE_SIZE];
  uint8 status[VIRTIO_QUEUE_SIZE];
  lock_t lock;
} disk;

static uint64 probe_virtio_base() {
  for (uint64 i = PA_TO_KVA(QEMU_VIRTIO_BASE); i <= (PA_TO_KVA(QEMU_VIRTIO_END)); i += QEMU_VIRTIO_STRIDE) {
    uint32 magic    = virtio_read(i, VIRTIO_MMIO_MAGIC);
    uint32 device   = virtio_read(i, VIRTIO_MMIO_DEVICE_ID);
    uint32 version  = virtio_read(i, VIRTIO_MMIO_VERSION);
    if (magic == MAGIC_NUM && device == DEVICE_DISK_ID && version == LEGACY) {
      return i;
    }
  }
  return 0;
} 

void virtio_disk_init() {
  lock_init(&disk.lock);
  uint64 device_addr = probe_virtio_base();
  if (!device_addr) {
    kprintf("failed to find disk device\n");
    return;
  }
  disk.base_addr = device_addr;

  uint32 status = 0;
  virtio_write(device_addr, VIRTIO_MMIO_STATUS, status); // reset device

  status |= VIRTIO_ACKNOWLEDGE;
  virtio_write(device_addr, VIRTIO_MMIO_STATUS, status); // set acknowledge status bit

  status |= VIRTIO_DRIVER;
  virtio_write(device_addr, VIRTIO_MMIO_STATUS, status); // set driver status bit

  // opting out of device features
  uint64 features = virtio_read(device_addr, VIRTIO_MMIO_DEVICE_FEATURES);
  features &= ~(1 << VIRTIO_BLK_F_RO);
  features &= ~(1 << VIRTIO_BLK_F_SCSI);
  features &= ~(1 << VIRTIO_BLK_F_CONFIG_WCE);
  features &= ~(1 << VIRTIO_F_RING_EVENT_IDX);
  features &= ~(1 << VIRTIO_F_RING_INDIRECT_DESC);

  virtio_write(device_addr, VIRTIO_MMIO_DRIVER_FEATURES, features);

  virtio_write(device_addr, VIRTIO_MMIO_GUEST_PAGE_SIZE, PAGE_SIZE);
  virtio_write(device_addr, VIRTIO_MMIO_QUEUE_SELECT, 0); // initialize queue 0

  // check max queue size
  uint32 max_size = virtio_read(device_addr, VIRTIO_MMIO_QUEUE_NUM_MAX);
  if (max_size == 0) {
    kprintf("virtio has no queue 0.\n");
    return;
  }
  if (max_size < VIRTIO_QUEUE_SIZE) {
    kprintf("virtio disk max queue to small.\n");
    return;
  }

  pa_t page0 = (pa_t)pmm_alloc();
  pa_t page1 = (pa_t)pmm_alloc();

  if (page1 != page0 + PAGE_SIZE) {
    kprintf("pages are not contigious.\n");
    return;
  }

  char *base_kva0 = (char*) PA_TO_KVA(page0);
  char *base_kva1 = (char*) PA_TO_KVA(page1);

  disk.desc = (virtq_desc*)(base_kva0);
  disk.avail = (virtq_avail*)(base_kva0 + VIRTIO_QUEUE_SIZE * 16);
  disk.used = (virtq_used*)(base_kva1);

  virtio_write(device_addr, VIRTIO_MMIO_QUEUE_NUM, VIRTIO_QUEUE_SIZE); // set queue size
  virtio_write(device_addr, VIRTIO_MMIO_QUEUE_ALIGN, PAGE_SIZE); 
  virtio_write(device_addr, VIRTIO_MMIO_QUEUE_PFN, (uint32)(page0 >> 12)); // set legacy mode physical frame

  for (size_t i = 0; i < VIRTIO_QUEUE_SIZE; ++i) { // set all descriptors unused
    disk.free_q[i] = 1;
  }

  status |= VIRTIO_DRIVER_OK;
  virtio_write(device_addr, VIRTIO_MMIO_STATUS, status);
}

// find free descriptor, mark non free and return index
static int find_free_descriptor() {
  for (size_t i = 0; i < VIRTIO_QUEUE_SIZE; ++i) {
    if (disk.free_q[i]) {
      disk.free_q[i] = false;
      return i;
    }
  }
  return -1;
}

static void free_descriptor(int i) {
  if (i < 0 || i >= VIRTIO_QUEUE_SIZE)
    kprintf("free descriptor error: 1\n");
  if (disk.free_q[i]) 
    kprintf("free descriptor error: 2\n");
  disk.desc[i].addr = 0;
  disk.desc[i].flags = 0;
  disk.desc[i].len = 0;
  disk.desc[i].next = 0;
  disk.free_q[i] = true;
}

// free chain of descriptors
static void free_chain(int i) {
  while (1) {
    int flags = disk.desc[i].flags;
    int nxt = disk.desc[i].next;
    free_descriptor(i);
    if (flags & VIRTQ_DESC_F_NEXT) {
      i = nxt;
    } else {
        break;
    }
  }
}

int virtio_disk_rw(void *buf, uint64 sector, int write) {
  lock(&disk.lock);
  
  int d0, d1, d2; 
  d0 = find_free_descriptor();
  d1 = find_free_descriptor();
  d2 = find_free_descriptor();
  if (d0 < 0 || d1 < 0 || d2 < 0) {
    kprintf("Failed to find free descriptor\n");
    return VIRTIO_BLK_S_IOERR;
  }

  virtio_blk_req *buf0 = &disk.ops[d0];  
  if (write) 
    buf0->type = VIRTIO_BLK_T_OUT; // write
  else 
    buf0->type = VIRTIO_BLK_T_IN; // read
  buf0->reserved = 0;
  buf0->sector = sector;
  
  disk.desc[d0].addr = (uint64) KVA_TO_PA(buf0);
  disk.desc[d0].len = sizeof(virtio_blk_req);
  disk.desc[d0].flags = VIRTQ_DESC_F_NEXT;
  disk.desc[d0].next = d1; 

  disk.desc[d1].addr = (uint64) KVA_TO_PA(buf);
  disk.desc[d1].len = VIRTIO_BLK_SECTOR_SIZE;
  disk.desc[d1].flags = VIRTQ_DESC_F_NEXT | (write ? 0 : VIRTQ_DESC_F_WRITE);
  disk.desc[d1].next = d2;

  disk.status[d2] = 0xFF; // sentinel
  disk.desc[d2].addr = KVA_TO_PA(&disk.status[d2]);
  disk.desc[d2].len = 1;
  disk.desc[d2].flags = VIRTQ_DESC_F_WRITE;
  disk.desc[d2].next = 0;

  disk.avail->ring[disk.avail->idx % VIRTIO_QUEUE_SIZE] = d0; 
  mb();
  disk.avail->idx += 1; 
  mb();
  virtio_write(disk.base_addr, VIRTIO_MMIO_QUEUE_NOTIFY, 0);  

  while (*(volatile uint16*)&disk.used->idx == disk.last_used_idx) {
  }

  mb();

  disk.last_used_idx += 1;

  free_chain(d0);

  unlock(&disk.lock);
  return disk.status[d2];
}






