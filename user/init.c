#include "user_syscall.h"

static void print(const char *s) {
  int len = 0;
  while (s[len]) len++;
  sys_write(1, s, len);
}

void _start() {
  int fds[2];

  /* Test 1: create pipe */
  if (sys_pipe(fds) < 0) {
    print("TEST 1: FAIL - pipe creation failed\n");
    sys_exit(1);
  }
  print("TEST 1: pipe created\n");

  /* Test 2: write then read in same process */
  char *msg = "hello";
  sys_write(fds[1], msg, 5);
  char buf[32];
  int n = sys_read(fds[0], buf, 5);
  if (n == 5 && buf[0] == 'h' && buf[4] == 'o') {
    print("TEST 2: same-process pipe works\n");
  } else {
    print("TEST 2: FAIL\n");
  }

  /* Test 3: pipe across fork */
  int pid = sys_fork();
  if (pid == 0) {
    /* child: close read end, write to pipe */
    sys_close(fds[0]);
    char *child_msg = "from child!";
    sys_write(fds[1], child_msg, 11);
    sys_close(fds[1]);
    sys_exit(0);
  }

  /* parent: close write end, read from pipe */
  sys_close(fds[1]);
  char rbuf[32];
  int bytes = sys_read(fds[0], rbuf, 11);
  if (bytes == 11 && rbuf[0] == 'f') {
    print("TEST 3: cross-process pipe works: ");
    sys_write(1, rbuf, bytes);
    print("\n");
  } else {
    print("TEST 3: FAIL\n");
  }

  /* Test 4: EOF detection - writer closed, reader gets 0 */
  int status;
  sys_wait(&status);
  int eof = sys_read(fds[0], rbuf, 1);
  if (eof == 0) {
    print("TEST 4: EOF detected correctly\n");
  } else {
    print("TEST 4: FAIL - expected 0 got something else\n");
  }

  sys_close(fds[0]);
  print("ALL PIPE TESTS DONE\n");
  sys_exit(0);
}
