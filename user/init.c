#include "user_libc.h"
#include "../kernel/types.h"

#define MAX_LINE_LENGTH 512
#define MAX_ARGS 32
#define PIPE_SYMBOL "|"

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

static int parse_pipeline(char* line, char* argv[], char** commands[], int max_cmds) {
  int argc = parse_line(line, argv);
  int ncmds = 0;

  commands[ncmds++] = &argv[0];

  for (int i = 0; i < argc; i++) {
    if (strcmp(argv[i], PIPE_SYMBOL) == 0) {
      argv[i] = NULL;
      if (ncmds < max_cmds)
        commands[ncmds++] = &argv[i + 1];
    }
  }

  return ncmds;
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

static void execute_pipeline(char** commands[], int ncmds) {
  if (ncmds == 1) {
    execute_command(commands[0]);
    return;
  }

  int p[2];
  pipe(p);

  int pid1 = fork();
  if (pid1 == 0) {
    close(p[0]);
    dup2(p[1], stdout);
    close(p[1]);
    exec(commands[0][0], commands[0]);

    char buf[256];
    size_t len = strlen(commands[0][0]);
    memcpy(buf, commands[0][0], len);
    memcpy(buf + len, ".elf", 4);
    buf[len + 4] = '\0';
    exec(buf, commands[0]);

    char rootbuf[256];
    rootbuf[0] = '/';
    memcpy(rootbuf + 1, buf, len + 5);
    exec(rootbuf, commands[0]);

    printf("command not found: %s\n", commands[0][0]);
    exit(1);
  }

  int pid2 = fork();
  if (pid2 == 0) {
    close(p[1]);
    dup2(p[0], stdin);
    close(p[0]);
    exec(commands[1][0], commands[1]);

    char buf[256];
    size_t len = strlen(commands[1][0]);
    memcpy(buf, commands[1][0], len);
    memcpy(buf + len, ".elf", 4);
    buf[len + 4] = '\0';
    exec(buf, commands[1]);

    char rootbuf[256];
    rootbuf[0] = '/';
    memcpy(rootbuf + 1, buf, len + 5);
    exec(rootbuf, commands[1]);

    printf("command not found: %s\n", commands[1][0]);
    exit(1);
  }
  close(p[0]);
  close(p[1]);
  wait(NULL);
  wait(NULL);
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
    if (line[0] == '\0') continue;

    char** commands[MAX_ARGS];
    int ncmds = parse_pipeline(line, argv, commands, MAX_ARGS);
    execute_pipeline(commands, ncmds);
  }
  exit(0);
}
