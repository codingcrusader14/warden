#ifndef PIPE_H
#define PIPE_H

#include "types.h"
#include "spinlock.h"
#include "wait_queue.h"
#include "file.h"

extern file_ops pipe_read_ops;
extern file_ops pipe_write_ops;

#define PIPE_BUF_SIZE 512

typedef struct {
  char circ_buf[PIPE_BUF_SIZE];
  uint32 read; // where to start next read
  uint32 write; // where to store next write
  uint32 size; // bytes currently in buffer
  lock_t spinlock;
  wait_queue rx_wait;
  wait_queue tx_wait;
  uint32 read_ref_count;
  uint32 write_ref_count;
} pipe;

pipe* pipe_alloc();
void pipe_free(pipe* p);

int64 pipe_read(file* p, void* buf, size_t len);
int64 pipe_read_write(file* p, const void* buf, size_t len);
int64 pipe_read_close(file* p);

int64 pipe_write(file* p, const void* buf, size_t len);
int64 pipe_write_read(file* p, void* buf, size_t len);
int64 pipe_write_close(file* p);


#endif
