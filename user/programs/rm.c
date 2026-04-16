#include "../user_libc.h"


void _start(int argc, char* argv[]) {
  if (argc < 2) {
    printf("usage: rm <file1> <file2> ...\n");
    exit(1);
  }

  for (int i = 1; i < argc; ++i) {
    int rc = unlink(argv[i]);
    if (rc < 0) {
      printf("failed to remove file\n");
      exit(1);
    }
  }

  exit(0);
}