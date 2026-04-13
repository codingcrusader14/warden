#ifndef FAT_32_H
#define  FAT_32_H

#include "../kernel/types.h"

#define DEFAULT_SECTOR_SIZE 512
#define DEFAULT_SEC_PER_CLUSTER 8
#define FAT32_MASK 0x0FFFFFFF
#define MAX_PATH 64

#define FREE_CLUSTER_ENTRY 0x00000000
#define EOC 0x0FFFFFFF
#define DELETED_CLUSTER 0xE5
#define EMPTY_CLUSTER 0x00
#define DIRECTORY 0x10

#define SELF   ".          "
#define PARENT "..         "

extern uint32 root_cluster;

enum type {
  ExFAT,
  FAT12,
  FAT16,
  FAT32,
};

typedef struct {
  uint8 file_name[11]; // first 8 are character of name and last 3 are extension 
  uint8 attribute; // READ_ONLY = 0x01, ect... more attributes of file
  uint8 reserved;
  uint8 creation_time_hundredths_second; 
  uint16 file_creation_time; // multiply by 2 seconds, Hour[5], Min[6], Second[5]
  uint16 file_creation_date; // Year[7], Month[4], Day[5]
  uint16 last_accessed_time; // same format as file_creation
  uint16 high_entry_first_cluster; // the high 16 bits of this entry's first cluster number. FAT12 and FAT16 is always zero
  uint16 last_modifed;
  uint16 last_modification_date;
  uint16 low_entry_first_cluster;
  uint32 size; // size of file in bytes
} __attribute__((packed)) fat32_dir_entry;

typedef struct {
  uint8   jmp[3]; // jump over disk format info 
  uint8   oem[8]; // usually ignored but carry name of DOS
  uint16  bytes_per_sector;
  uint8   sectors_per_cluster;
  uint16  reserved_sectors; // boot record sectors are included here
  uint8   fat_count; // number of FAT's on storage media
  uint16  dir_entry_count; // number of root directory entries 
  uint16  total_sector_count; // 0 means more than 65535 sectors in volume
  uint8   media_type; 
  uint16  sectors_per_fat_16; // FAT12/FAT16 only.
  uint16  sectors_per_track; 
  uint16  heads; // number of heads or sides on storage media
  uint32  hidden_sectors; // number of hidden sectors
  uint32  total_sector_count32; // this field is set when there are more than 65535 sectors on volume

  // extended boot record 
  uint32  sectors_per_fat_32; // sectors per FAT32
  uint16  flags; 
  uint16  fat_vsn; // FAT version number. High byte is the major version and low byte minor version.
  uint32  cluster_num_root_dir; // cluster number of root directory, often field set to 2
  uint16  sector_of_fsinfo; // sector number of FSinfo structure
  uint16  sector_backup_boot; // sector number of backup boot sector
  uint8   reserved[12]; // when volume is formatted these bytes should be 0
  uint8   drive_number;
  uint8   flags_windows_nt; // reserved otherwise
  uint8   signature; // must be 0x28 or 0x29
  uint32  volume_id_serial; // volume ID 'Serial' number. Used for tracking volumes between computers
  uint8   volume_label_string[11]; // field is padded with spaces 
  uint8   system_indentifer_string[8]; // always FAT32, dont tryst contents of this string for any use
} __attribute__((packed)) fat32_bpb; // bios parameter block

fat32_bpb* init_disk_bpb();
int read_directory(uint32 start_cluster, fat32_dir_entry* entries, uint32 max_entries);
int dir_lookup(uint32 dir_cluster, const char* name, fat32_dir_entry* result);
int path_lookup(const char* path, fat32_dir_entry* result, uint32* parent_cluster);
int fat32_read(fat32_dir_entry* entry, void* buf, uint32 offset, uint32 size);
int fat32_write(fat32_dir_entry* entry, const void* buf, size_t offset, size_t size);
int fat32_create(uint32 dir_cluster, const char* name, fat32_dir_entry* res);
int update_directory(uint32 dir_cluster, fat32_dir_entry* updated);
int fat32_mkdir(uint32 dir_cluster, const char* name);
int fat32_unlink(uint32 dir_cluster, const char* name);
int fat32_rmdir(uint32 dir_cluster, const char* dname);
void print_metadata();


#endif