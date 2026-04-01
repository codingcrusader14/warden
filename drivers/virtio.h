/*
 * Virtio MMIO device definitions
 * used for QEMU testing only
 * 
 * Specification: https://docs.oasis-open.org/virtio/virtio/v1.1/virtio-v1.1.html
 */

#ifndef VIRTIO_H
#define VIRTIO_H

#include "../kernel/types.h"

// virtio mapped starting at 0x0a000000
// virtio ending at 0x0a003e00 
// 0x200 strides for a total of 32 virtio devices
#define QEMU_VIRTIO_BASE 0x0a000000
#define QEMU_VIRTIO_STRIDE 0x200
#define QEMU_VIRTIO_END 0x0a003e00

#define VIRTIO_BLK_SECTOR_SIZE 512
#define MAGIC_NUM 0x74726976
#define DEVICE_DISK_ID 2
#define LEGACY 1

#define VIRTIO_MMIO_MAGIC             0x000 // 0x74726976
#define VIRTIO_MMIO_VERSION           0x004 // version should be 2, legacy is 1
#define VIRTIO_MMIO_DEVICE_ID         0x008 // device type, 2 for disk
#define VIRTIO_MMIO_VENDOR_ID         0x00c
#define VIRTIO_MMIO_DEVICE_FEATURES   0x010
#define VIRTIO_MMIO_DRIVER_FEATURES   0x020
#define VIRTIO_MMIO_QUEUE_SELECT      0x030 // write only, selects queue
#define VIRTIO_MMIO_QUEUE_NUM_MAX     0x034 // read only, max size of current queue
#define VIRTIO_MMIO_QUEUE_NUM         0x038 // write only, size of current queue
#define VIRTIO_MMIO_QUEUE_PFN         0x040 // where virtqueue memory lives
#define VIRTIO_MMIO_QUEUE_READY       0x044 // read and write, ready bit
#define VIRTIO_MMIO_QUEUE_NOTIFY      0x050 // write only, new buffers to process in a queue
#define VIRTIO_MMIO_INTERRUPT_STATUS  0x060 // read only, 
#define VIRTIO_MMIO_INTERRUPT_ACK     0x064 // write only
#define VIRTIO_MMIO_STATUS            0x070 // read and write
#define VIRTIO_MMIO_QUEUE_DESC_LOW    0x080 // write-only, physical address for descriptor table
#define VIRTIO_MMIO_QUEUE_DESC_HIGH   0x084
#define VIRTIO_MMIO_QUEUE_DRIVER_LOW  0x090 // write-only, physical address for available ring
#define VIRTIO_MMIO_QUEUE_DRIVER_HIGH 0x094
#define VIRTIO_MMIO_QUEUE_DEVICE_LOW  0x0a0 // write-only, physical address for used ring
#define VIRTIO_MMIO_QUEUE_DEVICE_HIGH 0x0a4

#define VIRTIO_QUEUE_SIZE 8 // virtio queue size
#define VIRTIO_MMIO_QUEUE_ALIGN 0x03C
#define VIRTIO_MMIO_GUEST_PAGE_SIZE 0x028

// device status bits
#define VIRTIO_ACKNOWLEDGE 1 // os recognizes the device as a valid virto device
#define VIRTIO_DRIVER 2 // indicates os knows how to drive device
#define VIRTIO_FAILED 128 // indicates a system error and os has given up on device
#define VIRTIO_FEATURES_OK 8 // acknowledged all features i
#define VIRTIO_DRIVER_OK 4 // driver is ready to drive device 
#define VIRTIO_DEVICE_NEEDS_RESET 64 // device reached an unrecoverable error

// virt queue descriptor table per spec
typedef struct {
  uint64 addr;
  uint32 len;
  uint16 flags;
  uint16 next;
} virtq_desc;

#define VIRTQ_DESC_F_NEXT   1 // buffer continues as per next field
#define VIRTQ_DESC_F_WRITE  2 // this marks buffer as a device write only (otherwise read only)

typedef struct {
  uint16 flags;
  uint16 idx;
  uint16 ring[VIRTIO_QUEUE_SIZE];
  uint16 used_event;
} virtq_avail;

#define VIRTQ_AVAIL_F_NO_INTERRUPT      1 

typedef struct {
  uint32 id; // index of of used descriptor chain
  uint32 len; // total length of the descriptor chain which was used
} virtq_used_elem;

typedef struct {
  uint16 flags;
  uint16 idx;
  virtq_used_elem ring[VIRTIO_QUEUE_SIZE];
  uint16 avail_event;
} virtq_used;

typedef struct {
  uint32 type;
  uint32 reserved;
  uint64 sector;
} virtio_blk_req;

// block device feature bits
#define VIRTIO_BLK_F_SIZE_MAX 1 // max size of single segment 
#define VIRTIO_BLK_F_SEG_MAX 2 // max number of segments in a request
#define VIRTIO_BLK_F_RO 5 // disk is read only
#define VIRTIO_BLK_F_BLK_SIZE 6 // block size of disk
#define VIRTIO_BLK_F_SCSI 7 // supports sci packet commands
#define VIRTIO_BLK_F_CONFIG_WCE 11 // device can toggle its cache between writeback and writethrough modes
#define VIRTIO_F_RING_INDIRECT_DESC 28 
#define VIRTIO_F_RING_EVENT_IDX 29

// block final status bytes
#define VIRTIO_BLK_S_OK        0 
#define VIRTIO_BLK_S_IOERR     1 
#define VIRTIO_BLK_S_UNSUPP    2

// type of block device request
#define VIRTIO_BLK_T_IN           0 // read
#define VIRTIO_BLK_T_OUT          1 // write

void virtio_disk_init();
int virtio_disk_rw(void *buf, uint64 sector, int write);

#endif
