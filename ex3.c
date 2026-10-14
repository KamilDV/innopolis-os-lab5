#include "prime.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct { int a, b, count; } PrimeRequest;

static void *prime_counter(void *arg) {
    PrimeRequest *request = arg;
    for (int i = request->a; i < request->b; ++i)
        request->count += is_prime(i);
    return NULL;
}

int main(int argc, char **argv) {
    int n, m;
    if (argc != 3 || parse_positive(argv[1], &n) || parse_positive(argv[2], &m)) {
        fprintf(stderr, "Usage: %s positive-n positive-m\n", argv[0]); return 1;
    }
    pthread_t *threads = calloc((size_t)m, sizeof *threads);
    PrimeRequest *requests = calloc((size_t)m, sizeof *requests);
    if (!threads || !requests) { perror("calloc"); free(threads); free(requests); return 1; }
    int started = 0, failed = 0;
    for (int i = 0; i < m; ++i) {
        requests[i].a = (int)((long long)n * i / m);
        requests[i].b = (int)((long long)n * (i + 1) / m);
        int rc = pthread_create(&threads[i], NULL, prime_counter, &requests[i]);
        if (rc) { fprintf(stderr, "pthread_create: %s\n", strerror(rc)); failed = 1; break; }
        ++started;
    }
    int total = 0;
    for (int i = 0; i < started; ++i) {
        int rc = pthread_join(threads[i], NULL);
        if (rc) { fprintf(stderr, "pthread_join: %s\n", strerror(rc)); failed = 1; }
        else total += requests[i].count;
    }
    if (!failed) printf("Primes in [0, %d): %d\n", n, total);
    free(requests); free(threads);
    return failed;
}
