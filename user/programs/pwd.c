#include "../user_libc.h"

void _start(int argc, char* argv[]) {
  (void)argc;
  (void)argv;
  char buf[256];

  if (getcwd(buf, sizeof(buf)) != 0) {
    printf("%s\n", buf);
  } 
  else {
    printf("Cannot get current working directory.\n");
    exit(1);
  }

  exit(0);
}