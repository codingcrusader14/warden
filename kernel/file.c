#include "file.h"
#include "pipe.h"
#include "libk/includes/string.h"
#include "libk/includes/stdio.h"

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
  if (f->type == FILE_PIPE_WRITE) {
        pipe* p = f->private_data;
        p->write_ref_count++;
    } else if (f->type == FILE_PIPE_READ) {
        pipe* p = f->private_data;
        p->read_ref_count++;
    }
}

int32 file_close(file* f) {
    if (f->type == FILE_PIPE_WRITE) {
        pipe* p = f->private_data;
        lock(&p->spinlock);
        p->write_ref_count--;
        if (p->write_ref_count == 0)
            wait_queue_wakeup(&p->rx_wait);
        unlock(&p->spinlock);
    } else if (f->type == FILE_PIPE_READ) {
        pipe* p = f->private_data;
        lock(&p->spinlock);
        p->read_ref_count--;
        if (p->read_ref_count == 0)
            wait_queue_wakeup(&p->tx_wait);
        unlock(&p->spinlock);
    }

    f->refcount--;
    if (f->refcount == 0) {
        if (f->type == FILE_PIPE_WRITE || f->type == FILE_PIPE_READ) {
            pipe* p = f->private_data;
            if (p->write_ref_count == 0 && p->read_ref_count == 0)
                pipe_free(p);
        }
        memset(f, 0, sizeof(file));
    }
    return 0;
}

