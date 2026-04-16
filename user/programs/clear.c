#include "../user_libc.h"

void _start(int argc, char* argv[]) {
  (void)argc;
  (void)argv;
  
  const char* clear = "\033[H\033[J";
  write(stdout, clear, 6);
  exit(0);
}