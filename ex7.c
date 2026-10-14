#define _POSIX_C_SOURCE 200809L
#include "prime.h"
#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint64_t *numbers, result = 1;
static size_t length, next;
static int overflow;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

static uint64_t gcd(uint64_t a, uint64_t b) {
    while (b) { uint64_t t = a % b; a = b; b = t; }
    return a;
}

static void *worker(void *arg) {
    (void)arg;
    for (;;) {
        pthread_mutex_lock(&lock);
        if (next == length || overflow) { pthread_mutex_unlock(&lock); return NULL; }
        uint64_t value = numbers[next++];
        uint64_t reduced = result / gcd(result, value);
        if (reduced > UINT64_MAX / value) overflow = 1;
        else result = reduced * value;
        pthread_mutex_unlock(&lock);
    }
}

int main(int argc, char **argv) {
    int m;
    if (argc != 3 || parse_positive(argv[2], &m)) {
        fprintf(stderr, "Usage: %s input-file positive-threads\n", argv[0]); return 1;
    }
    FILE *file = fopen(argv[1], "r");
    if (!file) { perror(argv[1]); return 1; }
    size_t capacity = 16;
    numbers = malloc(capacity * sizeof *numbers);
    if (!numbers) { perror("malloc"); fclose(file); return 1; }
    for (;;) {
        unsigned long long value;
        int rc = fscanf(file, "%llu", &value);
        if (rc == EOF) break;
        if (rc != 1 || value == 0) { fputs("Input must contain positive integers\n", stderr); fclose(file); free(numbers); return 1; }
        if (length == capacity) {
            capacity *= 2;
            uint64_t *resized = realloc(numbers, capacity * sizeof *numbers);
            if (!resized) { perror("realloc"); fclose(file); free(numbers); return 1; }
            numbers = resized;
        }
        numbers[length++] = value;
    }
    fclose(file);
    if ((size_t)m >= length) { fputs("Thread count must be smaller than number count\n", stderr); free(numbers); return 1; }
    pthread_t *threads = calloc((size_t)m, sizeof *threads);
    if (!threads) { perror("calloc"); free(numbers); return 1; }
    int started = 0, failed = 0;
    for (int i = 0; i < m; ++i) {
        int rc = pthread_create(&threads[i], NULL, worker, NULL);
        if (rc) { fprintf(stderr, "pthread_create: %s\n", strerror(rc)); failed = 1; break; }
        ++started;
    }
    for (int i = 0; i < started; ++i) pthread_join(threads[i], NULL);
    if (overflow) fputs("LCM exceeds uint64_t\n", stderr);
    else if (!failed) printf("LCM: %llu\n", (unsigned long long)result);
    pthread_mutex_destroy(&lock); free(threads); free(numbers);
    return failed || overflow;
}
