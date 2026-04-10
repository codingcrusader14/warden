#include "vfs.h"
#include "fat32.h"
#include "../kernel/file.h"
#include "../kernel/libk/includes/string.h"
#include "../kernel/libk/includes/stdlib.h"

int64 vfs_file_read(file* f, void* buf, size_t size) {
  if (!f || !buf) return -1;

  vfs_file_data* fdata = (vfs_file_data*) f->private_data;
  int bytes_read = fat32_read(&fdata->inode->entry, buf, fdata->offset, size);
  fdata->offset += bytes_read;
  return bytes_read;
}

int64 vfs_file_write(file* f, const void* buf, size_t size) {
  return 0;
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
  fat32_dir_entry found_entry; 
  if (path_lookup(path, &found_entry) != 0)
    return NULL;

  vfs_inode* inode = kmalloc(sizeof(vfs_inode));
  if (!inode) 
    return NULL;

  inode->entry = found_entry;
  inode->ref_count = 1;
  inode->start_cluster = (found_entry.high_entry_first_cluster << 16) | (found_entry.low_entry_first_cluster);

  vfs_file_data* vfs_data = kmalloc(sizeof(vfs_file_data));
  if (!vfs_data) {
    kfree(inode);
    return NULL;
  }
  vfs_data->inode = inode;
  vfs_data->offset = 0;
  file* f = file_alloc(FILE_INODE, &vfs_fops, vfs_data);
  if (!f) {
    kfree(inode);
    kfree(vfs_data);
    return NULL;
  }
  return f;
}
