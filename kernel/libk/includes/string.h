#ifndef LIBK_STRING_H
#define LIBK_STRING_H

#include <stddef.h>

size_t strlen(const char*);
void* memcpy(void* , const void* , size_t);
void* memmove(void*, const void*, size_t);
void* memset(void*, int, size_t);
int memcmp(const void*, const void*, size_t);
int strcmp(const char* s1, const char* s2);

#endif 
