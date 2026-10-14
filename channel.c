#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

static int transfer(int input, int output) {
    char buf[1024];
    for (;;) {
        ssize_t n = read(input, buf, sizeof buf);
        if (n == 0) return 0;
        if (n < 0) {
            if (errno == EINTR) continue;
            return -1;
        }
        for (ssize_t done = 0; done < n;) {
            ssize_t written = write(output, buf + done, (size_t)(n - done));
            if (written < 0 && errno == EINTR) continue;
            if (written <= 0) return -1;
            done += written;
        }
    }
}

int main(void) {
    int fd[2];
    if (pipe(fd) < 0) { perror("pipe"); return 1; }
    pid_t child = fork();
    if (child < 0) { perror("fork"); return 1; }
    if (child == 0) {
        close(fd[1]);
        int ok = transfer(fd[0], STDOUT_FILENO);
        close(fd[0]);
        if (ok < 0) perror("subscriber");
        _exit(ok < 0);
    }
    close(fd[0]);
    int ok = 0;
    char *line = NULL;
    size_t capacity = 0;
    ssize_t length;
    while ((length = getline(&line, &capacity, stdin)) >= 0) {
        if (length > 1024) { fputs("Message exceeds 1024 bytes\n", stderr); ok = -1; continue; }
        for (ssize_t done = 0; done < length;) {
            ssize_t written = write(fd[1], line + done, (size_t)(length - done));
            if (written < 0 && errno == EINTR) continue;
            if (written <= 0) { ok = -1; break; }
            done += written;
        }
        if (ok < 0) break;
    }
    free(line);
    close(fd[1]);
    int status;
    while (waitpid(child, &status, 0) < 0 && errno == EINTR) {}
    if (ok < 0) perror("publisher");
    return ok < 0 || !WIFEXITED(status) || WEXITSTATUS(status);
}
