#include "fat32.h"
#include "../drivers/virtio.h"
#include "../kernel/libk/includes/string.h"
#include "../kernel/libk/includes/stdio.h"
#include "../kernel/libk/includes/stdlib.h"

static uint32 sector_size;
static uint32 total_sectors; 
static uint32 reserved_sectors;
static uint32 total_fat_sector_size;
static uint32 root_dir_sectors;
static uint32 first_data_sector;
static uint32 first_fat_sector;
static uint32 total_data_sectors;
static uint32 sectors_per_cluster;
static uint32 total_clusters;
static uint32 root_cluster;
static uint32 cluster_size;
enum   type   fat_type;

static void set_fs_metadata(fat32_bpb* disk) {
  sector_size = disk->bytes_per_sector;
  total_sectors = (disk->total_sector_count == 0) ? disk->total_sector_count32 : disk->total_sector_count;
  reserved_sectors = disk->reserved_sectors;
  total_fat_sector_size = (disk->sectors_per_fat_16 == 0) ? disk->sectors_per_fat_32 * disk->fat_count: disk->sectors_per_fat_16 * disk->fat_count;
  root_dir_sectors = ((disk->dir_entry_count * 32) + (sector_size - 1)) / sector_size;
  first_data_sector = reserved_sectors + total_fat_sector_size + root_dir_sectors;
  first_fat_sector = reserved_sectors;
  total_data_sectors = total_sectors - first_data_sector;
  sectors_per_cluster = disk->sectors_per_cluster;
  total_clusters = total_data_sectors / sectors_per_cluster;
  root_cluster = disk->cluster_num_root_dir;
  cluster_size = sectors_per_cluster * sector_size;;
  if (disk->sectors_per_fat_16 == 0 && disk->sectors_per_fat_32 != 0) {
    fat_type = FAT32;
  }
}

fat32_bpb* init_disk_bpb() {
  fat32_bpb* disk = kmalloc(sizeof(fat32_bpb));
  uint8 disk_buf[DEFAULT_SECTOR_SIZE];
  int ret = virtio_disk_rw(disk_buf, 0, 0);
  if (ret != 0) {
    kfree(disk);
    return NULL;
  }
  memcpy(disk, disk_buf, sizeof(fat32_bpb));
  set_fs_metadata(disk);
  return disk;
}

static uint32 cluster_to_lba(uint32 cluster) {
  return ((cluster - 2) * sectors_per_cluster) + first_data_sector;
}

static uint32 fat_next_cluster(uint32 cluster) {
  uint32 fat_sector = first_fat_sector + (cluster * 4) / sector_size;
  uint32 fat_offset = ((cluster * 4) % sector_size);
  uint8 fat_buffer[DEFAULT_SECTOR_SIZE];
  int ret = virtio_disk_rw(fat_buffer, fat_sector, 0);
  if (ret != 0) {
    return 0;
  }
  uint32 raw_entry = *(uint32*)(fat_buffer + fat_offset);
  return raw_entry & FAT32_MASK;
}

static int read_cluster(uint32 cluster, void* buf) {
  int ret;
  uint32 lba;
  uint8 sector_buf[DEFAULT_SECTOR_SIZE];
  void* dst;

  dst = buf;
  lba = cluster_to_lba(cluster);
  for (size_t i = 0; i < sectors_per_cluster; ++i, lba += 1) {
    ret = virtio_disk_rw(sector_buf, lba , 0);
    if (ret != 0) {
      return -1;
    }
    memcpy(dst, sector_buf, DEFAULT_SECTOR_SIZE);
    dst += DEFAULT_SECTOR_SIZE;
  }
  return 0;
}

static char toupper(const char c) {
  if (c >= 'a' && c <= 'z') {
    char offset = c - 'a';
    char upper = 'A' + offset;
    return upper;
  }
  return c;
}

static void copy_cstr_fat(const char* str, void* buf) {
  char fat_buf[11]; 

  int i = 0, j = 0;
  while (str[j] != '\0' && str[j] != '.') {
    fat_buf[i] = toupper(str[j]);
    i++;
    j++;
    if (j == 8) break; // 
  }

  while (i < 8) {
    fat_buf[i] = ' ';
    i++;
  }

  if (str[j] == '.') {
    j++;
  }

  while (str[j] != '\0' && i < 11) {
    fat_buf[i] = toupper(str[j]);
    i++;
    j++;
  }

  while (i < 11) {
    fat_buf[i] = ' ';
    i++;
  }

  memcpy(buf, fat_buf, 11);
}

int read_directory(uint32 start_cluster, fat32_dir_entry* entries, uint32 max_entries) {
  uint32 count = 0, current_cluster = start_cluster;
  uint32 entries_per_cluster = cluster_size / 32;
  uint8 cluster_buffer[cluster_size];
  
  while (1) {
    read_cluster(current_cluster, cluster_buffer);

    for (size_t i = 0; i < entries_per_cluster; ++i) {
      fat32_dir_entry entry = *(fat32_dir_entry*)(cluster_buffer + (i * 32));

      if (entry.file_name[0] == 0x00) // no more entries
        return count;

      if (entry.file_name[0] == 0xE5) // deleted
        continue;

      if (entry.attribute == 0x0F) // Long file name
        continue;
      
      if (count >= max_entries) // output array full
        return count;
      
      entries[count] = entry;
      count++;
    }
    uint32 next_cluster = fat_next_cluster(current_cluster);
    if (next_cluster >= FAT32_MASK) // end of chian
      break;
    current_cluster = next_cluster;
  }
  return count;
}

int dir_lookup(uint32 dir_cluster, const char* name, fat32_dir_entry* result) {
  uint32 current_cluster = dir_cluster;
  uint32 entries_per_cluster = cluster_size / 32;
  uint8 cluster_buffer[cluster_size];

  uint8 filename[11];
  copy_cstr_fat(name, filename);
  while (1) {
    read_cluster(current_cluster, cluster_buffer);

    for (size_t i = 0; i < entries_per_cluster; ++i) {
      fat32_dir_entry entry = *(fat32_dir_entry*)(cluster_buffer + (i * 32));

      if (entry.file_name[0] == 0x00) // no more entries
        return -1;

      if (entry.file_name[0] == 0xE5) // deleted
        continue;

      if (entry.attribute == 0x0F) // Long file name
        continue;


      if (memcmp(filename, entry.file_name, sizeof(filename)) == 0) {
        *result = entry;
        return 0;
      }

    }
    uint32 next_cluster = fat_next_cluster(current_cluster);
    if (next_cluster >= FAT32_MASK) // end of chian
      break;
    current_cluster = next_cluster;
  }
  return -1;
}

int path_lookup(const char* path, fat32_dir_entry* result) {
  uint32 starting_cluster = root_cluster;
  char sub_dir[MAX_PATH];
  int i = 0, j = 0;

  if (path[i] == '/') 
    i++;

  while (path[i] != '\0') {
    j = 0;
    memset(sub_dir, 0, MAX_PATH);
    while (path[i] != '\0' && path[i] != '/') {
      sub_dir[j] = path[i];
      i++;
      j++;
    }
    sub_dir[j] = '\0';

    if (path[i] == '/' ) {
      i++;
    }

    if (dir_lookup(starting_cluster, sub_dir, result) != 0) 
      return -1;

    if (path[i] != '\0') { // more path left
      if (result->attribute != 0x10) 
        return -1;
      starting_cluster = (result->high_entry_first_cluster << 16) | (result->low_entry_first_cluster);
    }     
  }
  return 0;
}

int fat32_read(fat32_dir_entry* entry, void* buf, uint32 offset, uint32 size) {
  if (offset >= entry->size) return 0;

  if (offset + size > entry->size) {
    size = entry->size - offset;
  }
  uint32 bytes_read = 0;
  uint32 current_cluster = (entry->high_entry_first_cluster << 16) | (entry->low_entry_first_cluster);
  uint32 cluster_offset = offset / (sectors_per_cluster * DEFAULT_SECTOR_SIZE); 
  for (size_t i = 0; i < cluster_offset; ++i) { // get to correct cluster
    uint32 next_cluster = fat_next_cluster(current_cluster);
    if (next_cluster >= FAT32_MASK)
      return 0;
    current_cluster = next_cluster;
  }

  uint32 byte_offset = offset % (sectors_per_cluster * DEFAULT_SECTOR_SIZE);
  uint8 cluster_buf[sectors_per_cluster * DEFAULT_SECTOR_SIZE];

  while (1) {
    int rc = read_cluster(current_cluster, cluster_buf);
    if (rc != 0)
      return bytes_read;

    uint32 bytes_remaining = (sectors_per_cluster * DEFAULT_SECTOR_SIZE) - byte_offset;
    uint32 bytes_requested = size - bytes_read;
    uint32 copy_bytes = (bytes_remaining < bytes_requested) ? bytes_remaining : bytes_requested;
    memcpy(buf + bytes_read, cluster_buf + byte_offset, copy_bytes);

    byte_offset = 0;
    bytes_read += copy_bytes;

    if (bytes_read >= size) 
      return bytes_read;

    uint32 next_cluster = fat_next_cluster(current_cluster);
    if (next_cluster >= FAT32_MASK)
      return bytes_read;

    current_cluster = next_cluster;
  }

  return bytes_read;
}

void print_metadata() {
  kprintf("Sector Size %d\n", sector_size);
  kprintf("Total Sectors %d\n", total_sectors);
  kprintf("FAT Sector Size %d\n", total_fat_sector_size);
  kprintf("Reserved Sectors %d\n", reserved_sectors);
  kprintf("Root Directory Entries %d\n", root_dir_sectors);
  kprintf("First Data Sector %d\n", first_data_sector);
  kprintf("First FAT sector %d\n", first_fat_sector);
  kprintf("Total Data Sectors %d\n", total_data_sectors);
  kprintf("Total clusters %d\n", total_clusters);
  kprintf("Sector Per Cluster %d\n", sectors_per_cluster);
  kprintf("FAT Type %s\n", (fat_type == FAT32) ? "FAT32" : "NOT FAT32");
}
