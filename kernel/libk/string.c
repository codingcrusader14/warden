#include "includes/string.h"

size_t strlen(const char *str) {
  size_t len = 0;
  while (str[len]) {
    len++;
  }
  return len;
}

void *memcpy(void *restrict dst, const void *restrict src, size_t n) {
  unsigned char *dst_copy = (unsigned char *)dst;
  const unsigned char *src_copy = (const unsigned char *)src;
  for (size_t i = 0; i < n; ++i) {
    dst_copy[i] = src_copy[i];
  }
  return dst;
}

void *memmove(void *dst, const void *src, size_t n) {
  unsigned char *dst_copy = (unsigned char *)dst;
  const unsigned char *src_copy = (const unsigned char *)src;
  if (dst_copy > src_copy && dst_copy < src_copy + n) { // overlap
    for (size_t i = n; i != 0; i--) {
      dst_copy[i - 1] = src_copy[i - 1];
    }
  } else { // non-overlap
    for (size_t i = 0; i < n; ++i) {
      dst_copy[i] = src_copy[i];
    }
  }
  return dst;
}

void *memset(void *bufptr, int v, size_t len) {
  unsigned char value = (unsigned char)v;
  unsigned char *buf = (unsigned char *)bufptr;
  for (size_t i = 0; i < len; ++i) {
    buf[i] = value;
  }
  return bufptr;
}

int memcmp(const void *s1, const void *s2, size_t len) {
  const unsigned char *str_1 = (const unsigned char *)s1;
  const unsigned char *str_2 = (const unsigned char *)s2;
  for (size_t i = 0; i < len; ++i) {
    if (str_1[i] != str_2[i]) {
      return str_1[i] - str_2[i];
    }
  }

  return 0;
}
