#include "../user_libc.h"

void _start(int argc, char* argv[]) {
  (void)argc;
  (void)argv;
  const char *path = ".";
  int fd = open(path, 0);

  if (fd < 0) {
    printf("ls cannot open directory.\n");
    exit(1);
  }

  dirent entries[MAX_DIRECT_ENTRIES];
  int count = getdents(fd, entries, sizeof(entries));
  close(fd);

  for (int i = 0; i < count; ++i) {
    if (entries[i].attr & 0x10) {
      printf("%.11s  <DIR>\n", entries[i].name);
    } else {
      printf("%.11s  %d\n", entries[i].name, entries[i].size);
    }
  }

  exit(0);
}