#include "user_syscall.h"

void _start() {
    sys_write(1, "hello from exec!\n", 17);
    sys_exit(0);
}