#include "../user_libc.h"

void _start(int argc, char *argv[]) {
  if (argc < 2) {
    printf("usage: cat <file1> <file2> ...\n");
    exit(1);
  }

  for (int i = 1; i < argc; ++i) {
    int fd = open(argv[i], 0);
    if (fd < 0) {
      printf("could not open file.\n");
      exit(1);
    }

    int n;
    char buf[512];
    while ((n = read(fd, buf, sizeof(buf))) > 0) {
      write(stdout, buf, n);
    }
    close(fd);
  }

  exit(0);
}