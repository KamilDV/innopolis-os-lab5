#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "Usage: %s message\n", argv[0]); return 1; }
    int to_child[2], to_parent[2], ready[2];
    if (pipe(to_child) || pipe(to_parent) || pipe(ready)) { perror("pipe"); return 1; }
    pid_t child = fork();
    if (child < 0) { perror("fork"); return 1; }
    if (!child) {
        close(to_child[1]); close(to_parent[0]); close(ready[0]);
        char marker = 'R';
        write(ready[1], &marker, 1); close(ready[1]);
        char message[1024];
        ssize_t size = read(to_child[0], message, sizeof message - 1);
        if (size < 0) _exit(1);
        message[size] = 0;
        printf("Child: %s\n", message); fflush(stdout);
        size_t length = strlen(message);
        write(to_parent[1], &length, sizeof length);
        close(to_child[0]); close(to_parent[1]); _exit(0);
    }
    close(to_child[0]); close(to_parent[1]); close(ready[1]);
    char marker;
    if (read(ready[0], &marker, 1) != 1) return 1;
    close(ready[0]);
    size_t length = strlen(argv[1]);
    if (length >= 1024 || write(to_child[1], argv[1], length) != (ssize_t)length) return 1;
    close(to_child[1]);
    size_t received;
    if (read(to_parent[0], &received, sizeof received) != sizeof received) return 1;
    printf("Parent: length = %zu\n", received);
    close(to_parent[0]); waitpid(child, NULL, 0);
    return 0;
}
