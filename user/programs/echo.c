#include "../user_libc.h"

void _start(int argc, char* argv[]) {
  if (argc < 2) {
    exit(0);
  }

  for (int i = 1; i < argc; ++i) {
    printf("%s ", argv[i]);
  }
  printf("\n");
  exit(0);
}