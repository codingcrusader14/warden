#include "pipe.h"
#include "libk/includes/stdlib.h"
#include "libk/includes/stdio.h"
#include "libk/includes/string.h"
#include "wait_queue.h"

pipe* pipe_alloc() {
  pipe* new_pipe = kmalloc(sizeof(pipe));

  if (!new_pipe)
    return NULL;
  
  memset(new_pipe, 0, sizeof(pipe));
  lock_init(&new_pipe->spinlock);
  wait_queue_init(&new_pipe->rx_wait);
  wait_queue_init(&new_pipe->tx_wait);
  new_pipe->read_ref_count = 1;
  new_pipe->write_ref_count = 1;
  return new_pipe;  
}

void pipe_free(pipe* p) {
  if (!p) 
    return;

  kfree(p);
}

static int32 pipe_read_spin(pipe* p) {
  while (p->size == 0) {
    if (p->write_ref_count == 0) return -1;
    wait_queue_sleep(&p->rx_wait, &p->spinlock);
  }

  return 0;
}

int64 pipe_read(file* p, void* buf, size_t len) {
  if (!p || !buf) return -1;

  char* cbuf = (char*) buf;
  pipe* pipe = p->private_data;
  lock(&pipe->spinlock); 
  for (size_t i = 0; i < len; ++i) {
    if (pipe_read_spin(pipe) == -1) {
      unlock(&pipe->spinlock);
      return i;
    }
    cbuf[i] = pipe->circ_buf[pipe->read % PIPE_BUF_SIZE];
    pipe->read++;
    pipe->size--;
  }

  wait_queue_wakeup(&pipe->tx_wait);
  unlock(&pipe->spinlock);
  return len;
}

int64 pipe_read_write(file* p, const void* buf, size_t len) {
  (void)p; (void)buf; (void)len; // compiler warnings
  return -1; // cant write from read end
}
int64 pipe_read_close(file* p) {
  if (!p) 
    return -1;

  pipe* pipe = p->private_data;
  lock(&pipe->spinlock);
  pipe->read_ref_count--;
  if (pipe->read_ref_count == 0) {
    wait_queue_wakeup(&pipe->tx_wait);
  }

  if (pipe->write_ref_count == 0 && pipe->read_ref_count == 0) {
    unlock(&pipe->spinlock);
    pipe_free(pipe);
    return 0;
  }
  unlock(&pipe->spinlock);
  return 0;
}

static int32 pipe_write_spin(pipe* p) {
  while (p->size == PIPE_BUF_SIZE) {
    if (p->read_ref_count == 0) return -1;
    wait_queue_sleep(&p->tx_wait, &p->spinlock);
  }

  return 0;
}

int64 pipe_write(file* p, const void* buf, size_t len) {
  if (!p || !buf) return -1;

  char* cbuf = (char*)buf;
  pipe* pipe = p->private_data;
  lock(&pipe->spinlock);
  for (size_t i = 0; i < len; ++i) {
    if (pipe_write_spin(pipe) == -1) {
      unlock(&pipe->spinlock);
      return -1;
    }
    pipe->circ_buf[pipe->write % PIPE_BUF_SIZE] = cbuf[i];
    pipe->write++;
    pipe->size++;
  }

  wait_queue_wakeup(&pipe->rx_wait);
  unlock(&pipe->spinlock);
  return len;
}

int64 pipe_write_read(file* p, void* buf, size_t len) {
  (void)p; (void)buf; (void)len; // compiler warnings
  return -1; // not possible to read from write end
}
int64 pipe_write_close(file* p) {
  if (!p) 
    return -1;
  
  pipe* pipe = p->private_data;
  lock(&pipe->spinlock);
  pipe->write_ref_count--;
  if (pipe->write_ref_count == 0) {
    wait_queue_wakeup(&pipe->rx_wait);
  }

  if (pipe->write_ref_count == 0 && pipe->read_ref_count == 0) {
    unlock(&pipe->spinlock);
    pipe_free(pipe);
    return 0;
  }

  unlock(&pipe->spinlock);
  return 0;
}

file_ops pipe_read_ops = {.write = pipe_read_write, .read = pipe_read, .close = pipe_read_close};
file_ops pipe_write_ops = {.write = pipe_write, .read = pipe_write_read, .close = pipe_write_close};
