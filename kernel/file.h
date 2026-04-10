#ifndef FILE_H
#define FILE_H

#include "types.h"

#define MAX_FDS 16
#define GLOBAL_FILE_CAP 64

enum file_types {
  FILE_INODE,
  FILE_CONSOLE,
  FILE_PIPE_READ,
  FILE_PIPE_WRITE,
};

struct file_ops;

typedef struct {
  enum file_types type;
  uint32 refcount;
  struct file_ops* ops;
  void* private_data;
} file;

typedef struct file_ops {
  int64 (*read)(file*, void*, size_t);
  int64 (*write)(file*, const void*, size_t);
  int64 (*close)(file*);
} file_ops;

file* file_alloc(enum file_types type, file_ops* ops, void* private_data);
void file_ref(file* f);
int32 file_close(file* f);

#endif
