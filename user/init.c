#include "user_libc.h"
#include "../kernel/types.h"

#define MAX_LINE_LENGTH 512
#define MAX_ARGS 32

static const char* built_ins[] =  {"exit", "cd"};

static int check_built_in(char* argv[]) {
  size_t built_in_length = sizeof(built_ins) / (sizeof(char*));
  for (size_t i = 0; i < built_in_length; ++i) {
    if (strcmp(argv[0],built_ins[i]) == 0) { // built in exists
      if (strcmp(argv[0], "exit") == 0) {
        exit(0);
      }
      else if (strcmp(argv[0], "cd") == 0) {
        if (argv[1]) {
          int rc = chdir(argv[1]);
          if (rc < 0) {
            printf("cd: no such file or directory.\n");
          }
        }
        else {
          chdir("/");
        }
        return 1;
      }
    }
  }
  return -1;
}

static void trim_line(char* line, int linelen) {
  int i = 0; 
  while (i < linelen && isspace(line[i])) {
    i++;
  }
 
  int j = linelen;
  while (j > i && isspace(line[j - 1])) {
    j--;
  }
  
  int newlen = j - i;
  memmove(line, line + i, newlen);
  line[newlen] = '\0';
}

static int parse_line(char* line, char* argv[]) {
  int argc = 0, i = 0;

  while (line[i] != '\0' && argc < MAX_ARGS - 1) {
    while (isspace(line[i])){
      i++;
    }
    if (line[i] == '\0') break;

    argv[argc++] = &line[i];

    while (line[i] != '\0' && !isspace(line[i])) {
      i++;
    }

    if (line[i] != '\0') {
      line[i] = '\0';
      i++;
    }
  }
  argv[argc] = NULL;
  return argc;
}

static void execute_command(char* argv[]) {
  int rc = check_built_in(argv);
  if (rc > 0) return;
  int pid = fork();

  if (pid == 0) {
    exec(argv[0], argv);

    char buf[256];
    size_t len = strlen(argv[0]);
    memcpy(buf, argv[0], len);
    memcpy(buf + len, ".elf", 4);  // try with .elf extension
    buf[len + 4] = '\0';
    exec(buf, argv);

    // try from root
    char rootbuf[256];
    rootbuf[0] = '/';
    memcpy(rootbuf + 1, buf, len + 5);
    exec(rootbuf, argv);

    printf("command not found: %s\n", argv[0]);
    exit(1);
  } 
  else if (pid > 0) 
  {
    wait(NULL);
  }
}

void _start() {
  printf("Welcome to Warden -- inspired by our favorite operating system ... Unix!\n");

  char line[MAX_LINE_LENGTH];
  char* argv[MAX_ARGS];
  int linelen;

  while (1) {
    printf("Warden> ");
    linelen = read(stdin, line, MAX_LINE_LENGTH - 1);
    if (linelen <= 0) continue;

    line[linelen - 1] = '\0';  // strip newline
    trim_line(line, linelen);
    int argc = parse_line(line, argv);
    if (argc == 0) continue;
    execute_command(argv);
  }
  exit(0);
}
