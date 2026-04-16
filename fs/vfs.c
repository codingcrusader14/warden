#include "vfs.h"
#include "fat32.h"
#include "../kernel/file.h"
#include "../kernel/global.h"
#include "../kernel/schedule.h"
#include "../kernel/libk/includes/string.h"
#include "../kernel/libk/includes/stdlib.h"
#include "../kernel/libk/includes/stdio.h"

int64 vfs_file_read(file* f, void* buf, size_t size) {
  if (!f || !buf) return -1;

  vfs_file_data* fdata = (vfs_file_data*) f->private_data;
  int bytes_read = fat32_read(&fdata->inode->entry, buf, fdata->offset, size);
  fdata->offset += bytes_read;
  return bytes_read;
}

int64 vfs_file_write(file* f, const void* buf, size_t size) {
  if (!f || !buf) return -1;

  vfs_file_data* fdata = (vfs_file_data*) f->private_data;
  int bytes_written = fat32_write(&fdata->inode->entry, buf, fdata->offset, size);
  update_directory(fdata->inode->parent_cluster, &fdata->inode->entry);
  fdata->offset += bytes_written;
  return bytes_written;
}
int64 vfs_file_close(file* f) {
  if (!f) return -1;

  vfs_file_data* fdata = (vfs_file_data*) f->private_data;
  fdata->inode->ref_count -= 1;
  if (fdata->inode->ref_count == 0) {
    kfree(fdata->inode);
  }
  kfree(fdata);

  int rc = file_close(f);
  return rc;
}

static file_ops vfs_fops = {
  .read = vfs_file_read,
  .write = vfs_file_write,
  .close = vfs_file_close,
};

file* vfs_file_open(const char* path, int flags) {
  fat32_dir_entry entry; 
  uint32 parent_cluster;

  if (path_lookup(path, &entry, &parent_cluster) != 0) {
    if (!(flags & O_CREAT))
      return NULL;

    // create the file 
    const char* name = path;
    const char* last_slash = NULL;
    for (size_t i = 0; path[i] != '\0'; ++i) {
      if (path[i] == '/')
        last_slash = &path[i];
    }

    if (last_slash) {
      name = last_slash + 1;
      char parent_path[MAX_PATH];
      int len = last_slash - path;
      if (len == 0) 
      {
        parent_cluster = root_cluster;
      } 
      else 
      {
        memcpy(parent_path, path, len);
        parent_path[len] = '\0';
        fat32_dir_entry parent;
        if (path_lookup(parent_path, &parent , NULL) != 0) 
          return NULL;
        parent_cluster = (parent.high_entry_first_cluster << 16) | (parent.low_entry_first_cluster);
      }
    }
    else 
    {
      parent_cluster = current_task ? current_task->cwd_cluster : root_cluster;
    }

    if (fat32_create(parent_cluster, name, &entry) != 0)
      return NULL;
  }
  

  vfs_inode* inode = kmalloc(sizeof(vfs_inode));
  if (!inode) 
    return NULL;

  inode->entry = entry;
  inode->ref_count = 1;
  inode->start_cluster = (entry.high_entry_first_cluster << 16) | (entry.low_entry_first_cluster);
  inode->parent_cluster = parent_cluster;

  vfs_file_data* vfs_data = kmalloc(sizeof(vfs_file_data));
  if (!vfs_data) {
    kfree(inode);
    return NULL;
  }
  vfs_data->inode = inode;
  vfs_data->offset = 0;
  enum file_types type = (entry.attribute & 0x10) ? FILE_DIRECTORY : FILE_INODE;
  file* f = file_alloc(type, &vfs_fops, vfs_data);
  if (!f) {
    kfree(inode);
    kfree(vfs_data);
    return NULL;
  }
  return f;
}
