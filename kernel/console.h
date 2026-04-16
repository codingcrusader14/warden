#ifndef CONSOLE_H
#define CONSOLE_H

#include "types.h"
#include "file.h"

#define DELETE 0x7F
#define BACKSPACE 0x08

extern file_ops console_ops;

int64 console_read(file* f, void* buf, size_t len);
int64 console_write(file* f, const void* buf, size_t len);
int64 console_close(file* f);

#endif
