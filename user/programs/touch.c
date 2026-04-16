#include "../user_libc.h"

void _start(int argc, char* argv[]) {
  if (argc < 2) {
    printf("usage: touch <file>\n");
    exit(1);
  }

  int fd = open(argv[1], O_CREAT);
  if (fd < 0) {
    printf("touch: cannot create %s\n", argv[1]);
    exit(1);
  }

  close(fd);

  exit(0);
}