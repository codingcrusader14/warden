#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <time.h>

#define ITERATIONS 1000

// Use the POSIX monotonic clock to bypass Linux kernel register traps
static uint64_t get_ns(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) {
        return 0;
    }
    return (uint64_t)ts.tv_sec * 1000000000ULL + (uint64_t)ts.tv_nsec;
}

static void bench_syscall(void) {
    uint64_t start = get_ns();
    for (int i = 0; i < ITERATIONS; i++) {
        getpid();
    }
    uint64_t end = get_ns();
    printf("lat_syscall:    %llu ns\n", (end - start) / ITERATIONS);
}

static void bench_read(void) {
    int fd = open("/dev/zero", O_RDONLY);
    if (fd < 0) return;
    char c;
    uint64_t start = get_ns();
    for (int i = 0; i < ITERATIONS; i++) {
        read(fd, &c, 1);
    }
    uint64_t end = get_ns();
    close(fd);
    printf("lat_read:       %llu ns\n", (end - start) / ITERATIONS);
}

static void bench_bw_pipe(void) {
    int p[2];
    pipe(p);
    char buf[512];
    memset(buf, 'x', sizeof(buf));

    pid_t pid = fork();
    if (pid == 0) {
        close(p[1]);
        while (read(p[0], buf, sizeof(buf)) > 0);
        exit(0);
    }

    close(p[0]);
    uint64_t total_bytes = 512 * 1000; // Total data sent
    uint64_t start = get_ns();
    for (int i = 0; i < 1000; i++) {
        if (write(p[1], buf, sizeof(buf)) != 512) break;
    }
    uint64_t end = get_ns();
    close(p[1]);
    wait(NULL);

    uint64_t ns = end - start;
    if (ns == 0) ns = 1; 
    // Calculate MB/s: (Bytes / 1024 / 1024) / (ns / 1,000,000,000)
    uint64_t mbps = (total_bytes * 1000) / ns; 
    printf("bw_pipe:        %lu MB/s\n", (unsigned long)mbps);
}

static void bench_fs(void) {
    uint64_t start = get_ns();
    for (int i = 0; i < 100; i++) {
        int fd = open("tmpfs_file", O_CREAT | O_WRONLY, 0644);
        close(fd);
        unlink("tmpfs_file");
    }
    uint64_t end = get_ns();
    printf("lat_fs:         %lu ns\n", (unsigned long)((end - start) / 100));
}


static void bench_write(void) {
    int fd = open("bench.tmp", O_CREAT | O_WRONLY | O_TRUNC, 0644);
    if (fd < 0) return;
    char c = 'x';
    uint64_t start = get_ns();
    for (int i = 0; i < ITERATIONS; i++) {
        write(fd, &c, 1);
    }
    uint64_t end = get_ns();
    close(fd);
    unlink("bench.tmp");
    printf("lat_write:      %llu ns\n", (end - start) / ITERATIONS);
}

static void bench_open(void) {
    int temp = open("hello.txt", O_CREAT | O_RDWR, 0644);
    close(temp);
    uint64_t start = get_ns();
    for (int i = 0; i < ITERATIONS; i++) {
        int fd = open("hello.txt", O_RDONLY);
        close(fd);
    }
    uint64_t end = get_ns();
    unlink("hello.txt");
    printf("lat_open:       %llu ns\n", (end - start) / ITERATIONS);
}

static void bench_fork(void) {
    uint64_t start = get_ns();
    for (int i = 0; i < 100; i++) {
        pid_t pid = fork();
        if (pid == 0) exit(0);
        wait(NULL);
    }
    uint64_t end = get_ns();
    printf("lat_fork:       %llu ns\n", (end - start) / 100);
}

static void bench_pipe(void) {
    int p[2];
    if (pipe(p) < 0) return;
    char c = 'x';
    uint64_t start = get_ns();
    for (int i = 0; i < ITERATIONS; i++) {
        write(p[1], &c, 1);
        read(p[0], &c, 1);
    }
    uint64_t end = get_ns();
    close(p[0]);
    close(p[1]);
    printf("lat_pipe:       %llu ns\n", (end - start) / ITERATIONS);
}

static void bench_ctx(void) {
    int p1[2], p2[2];
    pipe(p1); pipe(p2);
    char c = 'x';
    pid_t pid = fork();
    if (pid == 0) {
        for (int i = 0; i < 100; i++) {
            read(p1[0], &c, 1);
            write(p2[1], &c, 1);
        }
        exit(0);
    }
    uint64_t start = get_ns();
    for (int i = 0; i < 100; i++) {
        write(p1[1], &c, 1);
        read(p2[0], &c, 1);
    }
    uint64_t end = get_ns();
    wait(NULL);
    printf("lat_ctx:        %llu ns\n", (end - start) / 100);
}

static void bench_exec(void) {
    uint64_t start = get_ns();
    for (int i = 0; i < 50; i++) {
        pid_t pid = fork();
        if (pid == 0) {
            char* args[] = {"/bin/true", NULL};
            execv("/bin/true", args);
            exit(0);
        }
        wait(NULL);
    }
    uint64_t end = get_ns();
    printf("lat_exec:       %llu ns\n", (end - start) / 50);
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        printf("usage: bench <test|all>\n");
        return 1;
    }

    if (strcmp(argv[1], "all") == 0) {
        printf("=== Linux Baseline (vDSO Clock) ===\n");
        bench_syscall();
        bench_read();
        bench_write();
        bench_open();
        bench_fork();
        bench_pipe();
        bench_ctx();
        bench_exec();
	bench_bw_pipe();
	bench_fs();
        printf("=== Done ===\n");
    } else {
        printf("Use 'all' to run the suite.\n");
    }
    
    return 0;
}
