#include "file.h"
#include "libk/includes/string.h"

static file global_file_table[GLOBAL_FILE_CAP];

file* file_alloc(enum file_types type, file_ops* ops, void* private_data) {
  for (size_t i = 0; i < GLOBAL_FILE_CAP; ++i) {
    file* entry = &global_file_table[i];
    if (entry->refcount == 0) {
        entry->refcount = 1;
        entry->type = type;
        entry->ops = ops;
        entry->private_data = private_data;
        return entry;
    }
  }
  return NULL;
}

void file_ref(file *f) {
  f->refcount += 1;
}

int32 file_close(file* f) {
  f->refcount -= 1;
  if (f->refcount == 0) {
    int64 rc = (f->ops->close(f));
    memset(f, 0, sizeof(file));
    return rc;
  }
  return 0;
}
