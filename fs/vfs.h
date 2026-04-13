#ifndef VFS_H
#define VFS_H

#include "../kernel/file.h" 
#include "fat32.h"

// vfs ops 
int64 vfs_file_read(file* f, void* buf, size_t size);
int64 vfs_file_write(file* f, const void* buf, size_t size);
int64 vfs_file_close(file* f);
file* vfs_file_open(const char* path, int flags);

typedef struct {
  fat32_dir_entry entry;
  uint32 start_cluster;
  uint32 parent_cluster;
  uint32 ref_count;
} vfs_inode;

typedef struct {
  vfs_inode* inode;
  uint32 offset;
} vfs_file_data;


#endif