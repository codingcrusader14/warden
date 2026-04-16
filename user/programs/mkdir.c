#include "../user_libc.h"

void _start(int argc, char* argv[]) {
  if (argc < 2) {
    printf("usage: mkdir <directory>.\n");
    exit(1);
  }

  int rc = mkdir(argv[1]);
  if (rc < 0) {
    printf("mkdir failed to create directory.\n");
    exit(1);
  }

  exit(0);
} 