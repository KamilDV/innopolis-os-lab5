#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static volatile sig_atomic_t stopped;
static void stop(int signal_number) { (void)signal_number; stopped = 1; }

static int read_all(int fd, void *buffer, size_t length) {
    char *p = buffer;
    while (length) {
        ssize_t n = read(fd, p, length);
        if (n < 0 && errno == EINTR) { if (stopped) return -1; continue; }
        if (n <= 0) return -1;
        p += n; length -= (size_t)n;
    }
    return 0;
}

int main(int argc, char **argv) {
    char *end;
    long id = argc == 2 ? strtol(argv[1], &end, 10) : 0;
    if (argc != 2 || *end || id < 1 || id > 128) {
        fprintf(stderr, "Usage: %s subscriber-id (1..128)\n", argv[0]); return 1;
    }
    char path[64];
    snprintf(path, sizeof path, "/tmp/ex1/s%ld", id);
    signal(SIGINT, stop); signal(SIGTERM, stop);
    int fd;
    do { fd = open(path, O_RDONLY); } while (fd < 0 && errno == EINTR && !stopped);
    if (fd < 0) { if (!stopped) perror(path); return !stopped; }
    while (!stopped) {
        uint16_t length;
        if (read_all(fd, &length, sizeof length)) break;
        if (length > 1024) { fputs("Invalid message length\n", stderr); close(fd); return 1; }
        char message[1024];
        if (read_all(fd, message, length)) break;
        if (fwrite(message, 1, length, stdout) != length || fflush(stdout)) { perror("stdout"); close(fd); return 1; }
    }
    close(fd);
    return 0;
}
