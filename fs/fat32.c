#include "fat32.h"
#include "../drivers/virtio.h"
#include "../kernel/libk/includes/string.h"
#include "../kernel/libk/includes/stdio.h"
#include "../kernel/libk/includes/stdlib.h"
#include "../kernel/schedule.h"

static uint32 sector_size;
static uint32 total_sectors; 
static uint32 reserved_sectors;
static uint32 total_fat_sector_size;
static uint32 root_dir_sectors;
static uint32 first_data_sector;
static uint32 first_fat_sector;
static uint32 total_data_sectors;
static uint32 sectors_per_cluster;
static uint32 sectors_per_fat;
static uint32 total_clusters;
uint32 root_cluster;
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
  cluster_size = sectors_per_cluster * sector_size;
  sectors_per_fat = disk->sectors_per_fat_32;
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

static int write_cluster(uint32 cluster, const void* buf) {
  int ret;
  uint32 lba;
  uint8 sector_buf[DEFAULT_SECTOR_SIZE];

  lba = cluster_to_lba(cluster);
  for (size_t i = 0; i < sectors_per_cluster; ++i, lba += 1) {
    memcpy(sector_buf, buf, DEFAULT_SECTOR_SIZE);
    ret = virtio_disk_rw(sector_buf, lba, 1);
    if (ret != 0) 
      return -1;

    buf += DEFAULT_SECTOR_SIZE;
  }
  return 0;
}

static int fat_set_entry(uint32 cluster, uint32 value) {
  uint32 fat_sector = first_fat_sector + (cluster * 4) / sector_size;
  uint32 fat_offset = ((cluster * 4) % sector_size);
  uint8 fat_buffer[DEFAULT_SECTOR_SIZE];
  int ret = virtio_disk_rw(fat_buffer, fat_sector, 0);
  if (ret != 0) {
    return -1;
  }
  *(uint32*)(fat_buffer + fat_offset) = value;
  virtio_disk_rw(fat_buffer, fat_sector, 1);
  virtio_disk_rw(fat_buffer, fat_sector + sectors_per_fat, 1); // write to backup FAT copy
  return 0;
}

static int32 alloc_cluster() { // find a free cluster
  uint32 entries_per_sector = DEFAULT_SECTOR_SIZE / 4, cluster_number;
  uint8 sector_buffer[DEFAULT_SECTOR_SIZE];
  int ret, entry;

  for (size_t sector = first_fat_sector; sector < first_fat_sector + sectors_per_fat; ++sector) {
    ret = virtio_disk_rw(sector_buffer, sector, 0);
    if (ret != 0) 
      return -1;

    for (size_t i = 0; i < entries_per_sector; i++) {
      cluster_number = ((sector - first_fat_sector) * entries_per_sector) + i;

      if (cluster_number < 2) 
        continue;

      if (cluster_number >= total_clusters + 2) 
        return -1;

      entry = *(uint32*)(sector_buffer + (i * 4));
      if (entry == FREE_CLUSTER_ENTRY) {
        fat_set_entry(cluster_number, EOC);
        return cluster_number;
      }
    }
  }
  return -1;
}


static int free_chain(uint32 start_cluster) {
  uint32 current_cluster = start_cluster;

  while (current_cluster >= 2 && current_cluster < EOC ) {
    uint32 next = fat_next_cluster(current_cluster);
    if (fat_set_entry(current_cluster, FREE_CLUSTER_ENTRY) < 0)
      return -1;
    current_cluster = next;
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

int update_directory(uint32 dir_cluster, fat32_dir_entry* updated) {
  uint32 current_cluster = dir_cluster;
  uint32 entries_per_cluster = cluster_size / 32;
  uint8 cluster_buffer[cluster_size];

  while (1) {
    read_cluster(current_cluster, cluster_buffer);

    for (size_t i = 0; i < entries_per_cluster; ++i) {
      fat32_dir_entry entry = *(fat32_dir_entry*)(cluster_buffer + (i * 32));

      if (entry.high_entry_first_cluster == updated->high_entry_first_cluster && entry.low_entry_first_cluster == updated->low_entry_first_cluster) {
        *(fat32_dir_entry*)(cluster_buffer + (i * 32)) = *updated;
        write_cluster(current_cluster, cluster_buffer);
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

int path_lookup(const char* path, fat32_dir_entry* result, uint32* parent_cluster) {
  if (!path) return -1;

  if (strcmp(path, ".") == 0) {
    memset(result, 0, sizeof(fat32_dir_entry));
    result->attribute = 0x10;
    uint32 cluster = current_task ? current_task->cwd_cluster : root_cluster;
    result->high_entry_first_cluster = (cluster >> 16) & 0xFFFF;
    result->low_entry_first_cluster = cluster & 0xFFFF;
    if (parent_cluster)
        *parent_cluster = cluster;
    return 0;
  }

  uint32 starting_cluster = root_cluster;
  char sub_dir[MAX_PATH];
  int i = 0, j = 0;

  if (path[0] == '/') {
    starting_cluster = root_cluster;
    i++;
  } else {
    starting_cluster = current_task ? current_task->cwd_cluster : root_cluster;
  }

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

    if (parent_cluster) 
      *parent_cluster = starting_cluster;

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

int fat32_write(fat32_dir_entry* entry, const void* buf, size_t offset, size_t size) { // write data to a file
  uint32 bytes_written = 0;
  uint32 current_cluster = (entry->high_entry_first_cluster << 16) | (entry->low_entry_first_cluster);
  uint32 cluster_offset = offset / cluster_size;
  for (size_t i = 0; i < cluster_offset; ++i) {
    uint32 next_cluster = fat_next_cluster(current_cluster); // find right cluster
    if (next_cluster >= FAT32_MASK) {
      return 0;
    }
    current_cluster = next_cluster;
  }

  uint32 byte_offset = offset % cluster_size;
  uint8 cluster_buf[cluster_size];

  while (1) {
    int rc = read_cluster(current_cluster, cluster_buf);
    if (rc != 0)
      return bytes_written;

    uint32 bytes_remaining = cluster_size - byte_offset;
    uint32 bytes_requested = size - bytes_written;
    uint32 copy_bytes = (bytes_remaining < bytes_requested) ? bytes_remaining : bytes_requested;
    memcpy(cluster_buf + byte_offset, buf + bytes_written, copy_bytes);
    write_cluster(current_cluster, cluster_buf);
    byte_offset = 0;
    bytes_written += copy_bytes;

    if (bytes_written >= size) {
      break;
    }

    uint32 next_cluster = fat_next_cluster(current_cluster);
    if (next_cluster >= FAT32_MASK) {
      if (bytes_written >= size) {
        return bytes_written;
      }
      int32 new_cluster = alloc_cluster();
      if (new_cluster == -1) 
        return bytes_written;
      fat_set_entry(current_cluster, new_cluster);
      current_cluster = new_cluster;
    } else {
      current_cluster = next_cluster;
    }

  }
  if (offset + bytes_written > entry->size) {
    entry->size = offset + bytes_written;
  }

  return bytes_written;
}

// create a new file within directory
int fat32_create(uint32 dir_cluster, const char* name, fat32_dir_entry* res) { 
  int32 free_cluster = alloc_cluster(); // find a free cluster 
  if (free_cluster == -1)  // no cluster available
    return -1;
  uint32 entries_per_cluster = cluster_size / 32;
  uint8 cluster_buffer[cluster_size];

  uint32 current_cluster = dir_cluster;
  uint8 filename[11];
  copy_cstr_fat(name, filename);
  while (1) {
    if (read_cluster(current_cluster, cluster_buffer) < 0) 
      return -1;
    for (size_t i = 0; i < entries_per_cluster; ++i) {
      fat32_dir_entry entry = *(fat32_dir_entry*)(cluster_buffer + (i * 32));

      if (entry.file_name[0] == EMPTY_CLUSTER || entry.file_name[0] == DELETED_CLUSTER ) { // free or deleted entry
        memset(&entry, 0, sizeof(entry));
        memcpy(entry.file_name, filename, 11);
        entry.size = 0;
        entry.attribute = 0x20;
        entry.high_entry_first_cluster = (free_cluster >> 16) & 0xFFFF;
        entry.low_entry_first_cluster = free_cluster & 0xFFFF;
        *(fat32_dir_entry*)(cluster_buffer + (i * 32)) = entry;
        if (write_cluster(current_cluster, cluster_buffer) < 0) {
          return -1;
        }
        *res = entry;
        return 0;
      }
    }
    uint32 next_cluster = fat_next_cluster(current_cluster);
    if (next_cluster >= FAT32_MASK)
      return -1;
    current_cluster = next_cluster; 
  }
  return 0;
}

// create a new subdirectoy with . (self) and .. (parent)
int fat32_mkdir(uint32 dir_cluster, const char* name) {
  int32 free_cluster = alloc_cluster(); // find a free cluster 
  if (free_cluster == -1)  // no cluster available
    return -1;
  uint32 entries_per_cluster = cluster_size / 32;
  uint8 cluster_buffer[cluster_size];

  uint32 current_cluster = dir_cluster;
  uint8 filename[11];
  copy_cstr_fat(name, filename);

  while (1) {
    if (read_cluster(current_cluster, cluster_buffer) < 0) 
      return -1;

    for (size_t i = 0; i < entries_per_cluster; ++i) {
      fat32_dir_entry entry = *(fat32_dir_entry*)(cluster_buffer + (i * 32));

      if (entry.file_name[0] == EMPTY_CLUSTER || entry.file_name[0] == DELETED_CLUSTER ) { // free or deleted entry
        memset(&entry, 0, sizeof(entry));
        memcpy(entry.file_name, filename, 11);
        entry.size = 0;
        entry.attribute = DIRECTORY;
        entry.high_entry_first_cluster = (free_cluster >> 16) & 0xFFFF;
        entry.low_entry_first_cluster = free_cluster & 0xFFFF;
        *(fat32_dir_entry*)(cluster_buffer + (i * 32)) = entry;
        if (write_cluster(current_cluster, cluster_buffer) < 0) {
          return -1;
        }
        uint8 new_buf[cluster_size]; 
        memset(new_buf, 0, cluster_size); 
        
        fat32_dir_entry* self = (fat32_dir_entry*)(new_buf);
        memcpy(self->file_name, SELF, 11);
        self->attribute = DIRECTORY;
        self->high_entry_first_cluster = (free_cluster >> 16) & 0xFFFF;
        self->low_entry_first_cluster  =  free_cluster & 0xFFFF;

        fat32_dir_entry* parent = (fat32_dir_entry*)(new_buf + 32);
        uint32 parent_cluster = (dir_cluster == root_cluster) ? 0 : dir_cluster;
        memcpy(parent->file_name, PARENT, 11);
        parent->attribute = DIRECTORY;
        parent->high_entry_first_cluster = (parent_cluster >> 16) & 0xFFFF;
        parent->low_entry_first_cluster  =  parent_cluster & 0xFFFF;

        write_cluster(free_cluster, new_buf);
        return 0;
      }
    }
    uint32 next_cluster = fat_next_cluster(current_cluster);
    if (next_cluster >= FAT32_MASK)
      return -1;
    current_cluster = next_cluster; 
  }
  return 0;
}

int fat32_unlink(uint32 dir_cluster, const char* name) {
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

      if (memcmp(entry.file_name, filename, 11) == 0) {
        uint32 file_cluster = (entry.high_entry_first_cluster << 16) | (entry.low_entry_first_cluster);
        entry.file_name[0] = DELETED_CLUSTER;
        *(fat32_dir_entry*)(cluster_buffer + (i * 32)) = entry;
        write_cluster(current_cluster, cluster_buffer);
        free_chain(file_cluster);
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

int fat32_rmdir(uint32 dir_cluster, const char* dname) {
  uint32 current_cluster = dir_cluster;
  uint32 entries_per_cluster = cluster_size / 32;
  uint8 cluster_buffer[cluster_size];

  uint8 dirname[11];
  copy_cstr_fat(dname, dirname);
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

      if (entry.attribute == DIRECTORY && memcmp(entry.file_name, dirname, 11) == 0) {
        uint32 target_cluster = (entry.high_entry_first_cluster << 16) | (entry.low_entry_first_cluster);

        uint32 scan_cluster = target_cluster;
        uint8 scan_buf[cluster_size];
        while (1) {
          read_cluster(scan_cluster, scan_buf);
          for (size_t k = 0; k < entries_per_cluster; ++k) {
            fat32_dir_entry* e = (fat32_dir_entry*)(scan_buf + (k * 32));
            if (e->file_name[0] == EMPTY_CLUSTER) break;
            if (e->file_name[0] == DELETED_CLUSTER) continue;
            if (e->attribute == 0x0F) continue;
            if (memcmp(e->file_name, SELF, 11) == 0) continue;
            if (memcmp(e->file_name, PARENT, 11) == 0) continue;
            return -1;  // not empty
          }
          uint32 next = fat_next_cluster(scan_cluster);
          if (next >= FAT32_MASK) break;
          scan_cluster = next;
        }

        entry.file_name[0] = DELETED_CLUSTER;
        *(fat32_dir_entry*)(cluster_buffer + (i * 32)) = entry;
        write_cluster(current_cluster, cluster_buffer);
        free_chain(target_cluster);
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
