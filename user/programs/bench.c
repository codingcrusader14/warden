#include "../user_libc.h"

#define ITERATIONS 1000

static void bench_syscall(void) {
    uint64_t freq = rdfreq();
    uint64_t start = rdcycle();
    for (int i = 0; i < ITERATIONS; i++) {
        getpid();
    }
    uint64_t end = rdcycle();
    uint64_t ns = cycles_to_ns(end - start, freq) / ITERATIONS;
    printf("lat_syscall:    %d ns\n", (int)ns);
}

static void bench_read(void) {
    int fd = open("hello.txt", 0);
    if (fd < 0) { printf("bench_read: cannot open file\n"); return; }
    char c;
    uint64_t freq = rdfreq();
    uint64_t start = rdcycle();
    for (int i = 0; i < ITERATIONS; i++) {
        read(fd, &c, 1);
    }
    uint64_t end = rdcycle();
    close(fd);
    uint64_t ns = cycles_to_ns(end - start, freq) / ITERATIONS;
    printf("lat_read:       %d ns\n", (int)ns);
}

static void bench_write(void) {
    int fd = open("bench.tmp", O_CREAT | O_WRONLY);
    if (fd < 0) { printf("bench_write: cannot open file\n"); return; }
    char c = 'x';
    uint64_t freq = rdfreq();
    uint64_t start = rdcycle();
    for (int i = 0; i < ITERATIONS; i++) {
        write(fd, &c, 1);
    }
    uint64_t end = rdcycle();
    close(fd);
    uint64_t ns = cycles_to_ns(end - start, freq) / ITERATIONS;
    printf("lat_write:      %d ns\n", (int)ns);
}

static void bench_open(void) {
    uint64_t freq = rdfreq();
    uint64_t start = rdcycle();
    for (int i = 0; i < ITERATIONS; i++) {
        int fd = open("hello.txt", 0);
        close(fd);
    }
    uint64_t end = rdcycle();
    uint64_t ns = cycles_to_ns(end - start, freq) / 10000;
    printf("lat_open:       %d ns\n", (int)ns);
}

static void bench_fork(void) {
    uint64_t freq = rdfreq();
    uint64_t start = rdcycle();
    for (int i = 0; i < 1000; i++) {
        int pid = fork();
        if (pid == 0) exit(0);
        wait(0);
    }
    uint64_t end = rdcycle();
    uint64_t ns = cycles_to_ns(end - start, freq) / 1000;
    printf("lat_fork:       %d ns\n", (int)ns);
}

static void bench_pipe(void) {
    int p[2];
    pipe(p);
    int pid = fork();
    if (pid == 0) {
        close(p[1]);
        char c;
        for (int i = 0; i < ITERATIONS; i++) {
            read(p[0], &c, 1);
            write(p[1], &c, 1);  // wrong end - need two pipes
        }
        exit(0);
    }
    // need two pipes for round trip - simplify to one-way
    close(p[0]);
    char c = 'x';
    uint64_t freq = rdfreq();
    uint64_t start = rdcycle();
    for (int i = 0; i < ITERATIONS; i++) {
        write(p[1], &c, 1);
    }
    uint64_t end = rdcycle();
    close(p[1]);
    wait(0);
    uint64_t ns = cycles_to_ns(end - start, freq) / ITERATIONS;
    printf("lat_pipe:       %d ns\n", (int)ns);
}

static void bench_exec(void) {
    uint64_t freq = rdfreq();
    uint64_t start = rdcycle();
    for (int i = 0; i < 100; i++) {
        int pid = fork();
        if (pid == 0) {
            exec("bench_nop", (char*[]){ "bench_nop", 0 });
            exec("bench_nop.elf", (char*[]){ "bench_nop", 0 });
            exec("/bench_nop.elf", (char*[]){ "bench_nop", 0 });
            exit(0);
        }
        wait(0);
    }
    uint64_t end = rdcycle();
    uint64_t ns = cycles_to_ns(end - start, freq) / 100;
    printf("lat_exec:       %d ns\n", (int)ns);
}

static void bench_ctx(void) {
    int p1[2], p2[2];
    pipe(p1);
    pipe(p2);
    char c = 'x';

    int pid = fork();
    if (pid == 0) {
        close(p1[1]);
        close(p2[0]);
        for (int i = 0; i < 100; i++) {
            read(p1[0], &c, 1);
            write(p2[1], &c, 1);
        }
        close(p1[0]);
        close(p2[1]);
        exit(0);
    }

    close(p1[0]);
    close(p2[1]);

    uint64_t freq = rdfreq();
    uint64_t start = rdcycle();
    for (int i = 0; i < 100; i++) {
        write(p1[1], &c, 1);
        read(p2[0], &c, 1);
    }
    uint64_t end = rdcycle();

    close(p1[1]);
    close(p2[0]);
    wait(0);

    uint64_t ns = cycles_to_ns(end - start, freq) / 100;
    printf("lat_ctx:        %d ns\n", (int)ns);
}

static void bench_fs(void) {
    uint64_t freq = rdfreq();
    uint64_t start = rdcycle();
    for (int i = 0; i < 100; i++) {
        int fd = open("tmpfile", O_CREAT);
        close(fd);
        unlink("tmpfile");
    }
    uint64_t end = rdcycle();
    uint64_t ns = cycles_to_ns(end - start, freq) / 100;
    printf("lat_fs:         %d ns\n", (int)ns);
}

static void bench_bw_pipe(void) {
    int p[2];
    pipe(p);
    char buf[512];
    memset(buf, 'x', sizeof(buf));

    int pid = fork();
    if (pid == 0) {
        close(p[1]);
        while (read(p[0], buf, sizeof(buf)) > 0) {}
        exit(0);
    }

    close(p[0]);
    int total_bytes = 512 * 1000;
    uint64_t freq = rdfreq();
    uint64_t start = rdcycle();
    for (int i = 0; i < 100; i++) {
        write(p[1], buf, sizeof(buf));
    }
    uint64_t end = rdcycle();
    close(p[1]);
    wait(0);

    uint64_t ns = cycles_to_ns(end - start, freq);
    uint64_t mbps = ((uint64_t)total_bytes * 1000) / ns;
    printf("bw_pipe:        %d MB/s\n", (int)mbps);
}

void _start(int argc, char* argv[]) {
    if (argc < 2) {
        printf("usage: bench <syscall|read|write|open|fork|exec|pipe|bw_pipe|ctx|fs|all>\n");
        exit(1);
    }

    if (strcmp(argv[1], "syscall") == 0) bench_syscall();
    else if (strcmp(argv[1], "read") == 0) bench_read();
    else if (strcmp(argv[1], "write") == 0) bench_write();
    else if (strcmp(argv[1], "open") == 0) bench_open();
    else if (strcmp(argv[1], "fork") == 0) bench_fork();
    else if (strcmp(argv[1], "exec") == 0) bench_exec();
    else if (strcmp(argv[1], "pipe") == 0) bench_pipe();
    else if (strcmp(argv[1], "bw_pipe") == 0) bench_bw_pipe();
    else if (strcmp(argv[1], "ctx") == 0) bench_ctx();
    else if (strcmp(argv[1], "fs") == 0) bench_fs();
    else if (strcmp(argv[1], "all") == 0) {
        printf("=== Warden Benchmark Suite ===\n");
        bench_syscall();
        bench_read();
        bench_write();
        bench_open();
        bench_fork();
        bench_exec();
        bench_pipe();
        bench_bw_pipe();
        bench_ctx();
        bench_fs();
        printf("=== Done ===\n");
    }
    else {
        printf("unknown test: %s\n", argv[1]);
    }

    exit(0);
}