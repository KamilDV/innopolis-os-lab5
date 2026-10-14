#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#define DIR "/tmp/ex1"
#define MAX_SUBSCRIBERS 128

static volatile sig_atomic_t stopped;
static void stop(int signal_number) { (void)signal_number; stopped = 1; }

static int write_all(int fd, const void *data, size_t length) {
    const char *p = data;
    while (length) {
        ssize_t n = write(fd, p, length);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return -1;
        p += n; length -= (size_t)n;
    }
    return 0;
}

static int read_all(int fd, void *data, size_t length) {
    char *p = data;
    while (length) {
        ssize_t n = read(fd, p, length);
        if (n < 0 && errno == EINTR) continue;
        if (n <= 0) return -1;
        p += n; length -= (size_t)n;
    }
    return 0;
}

static void child_loop(int input, int id, int ready) {
    char path[64];
    snprintf(path, sizeof path, DIR "/s%d", id);
    int fifo = open(path, O_WRONLY);
    if (fifo < 0) { perror(path); _exit(1); }
    char marker = 'R';
    if (write_all(ready, &marker, 1)) _exit(1);
    close(ready);
    for (;;) {
        uint16_t length;
        if (read_all(input, &length, sizeof length)) break;
        if (length > 1024) break;
        char message[1024];
        if (read_all(input, message, length)) break;
        if (write_all(fifo, &length, sizeof length) || write_all(fifo, message, length)) break;
    }
    close(fifo); close(input); _exit(0);
}

int main(int argc, char **argv) {
    char *end;
    long n = argc == 2 ? strtol(argv[1], &end, 10) : 0;
    if (argc != 2 || *end || n < 1 || n > MAX_SUBSCRIBERS) {
        fprintf(stderr, "Usage: %s subscribers (1..%d)\n", argv[0], MAX_SUBSCRIBERS);
        return 1;
    }
    if (mkdir(DIR, 0700) < 0 && errno != EEXIST) { perror("mkdir"); return 1; }
    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, stop);
    signal(SIGTERM, stop);
    int channels[MAX_SUBSCRIBERS] = {0};
    pid_t children[MAX_SUBSCRIBERS] = {0};
    int ready[2];
    if (pipe(ready) < 0) { perror("pipe"); return 1; }
    int started = 0, failed = 0;
    for (int i = 0; i < n; ++i) {
        char path[64];
        snprintf(path, sizeof path, DIR "/s%d", i + 1);
        if (mkfifo(path, 0600) < 0 && errno != EEXIST) { perror("mkfifo"); failed = 1; break; }
        int fd[2];
        if (pipe(fd) < 0) { perror("pipe"); failed = 1; break; }
        pid_t child = fork();
        if (child < 0) { perror("fork"); close(fd[0]); close(fd[1]); failed = 1; break; }
        if (child == 0) {
            close(ready[0]); close(fd[1]);
            for (int j = 0; j < started; ++j) close(channels[j]);
            child_loop(fd[0], i + 1, ready[1]);
        }
        close(fd[0]);
        channels[i] = fd[1]; children[i] = child; ++started;
    }
    close(ready[1]);
    if (!failed) {
        for (int i = 0; i < n && !stopped; ++i) {
            char marker;
            if (read_all(ready[0], &marker, 1)) { failed = 1; break; }
        }
    }
    close(ready[0]);
    if (!failed && !stopped) {
        char *line = NULL;
        size_t capacity = 0;
        ssize_t length;
        while (!stopped && (length = getline(&line, &capacity, stdin)) >= 0) {
            if (length > 1024) { fputs("Message exceeds 1024 bytes\n", stderr); failed = 1; continue; }
            uint16_t size = (uint16_t)length;
            for (int i = 0; i < n; ++i) {
                if (write_all(channels[i], &size, sizeof size) ||
                    write_all(channels[i], line, size)) { perror("publish"); failed = 1; stopped = 1; break; }
            }
        }
        free(line);
    }
    for (int i = 0; i < started; ++i) close(channels[i]);
    for (int i = 0; i < started; ++i) {
        if (stopped || failed) kill(children[i], SIGTERM);
        while (waitpid(children[i], NULL, 0) < 0 && errno == EINTR) {}
    }
    for (int i = 0; i < n; ++i) {
        char path[64]; snprintf(path, sizeof path, DIR "/s%d", i + 1); unlink(path);
    }
    rmdir(DIR);
    return failed;
}
