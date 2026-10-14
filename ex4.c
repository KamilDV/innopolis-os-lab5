#include "prime.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static pthread_mutex_t global_lock = PTHREAD_MUTEX_INITIALIZER;
static int k = 0, c = 0, n;

static int get_number_to_check(void) {
    int ret = k;
    if (k != n) ++k;
    return ret;
}

static void increment_primes(void) { ++c; }

static void *check_primes(void *arg) {
    (void)arg;
    for (;;) {
        pthread_mutex_lock(&global_lock);
        int candidate = get_number_to_check();
        pthread_mutex_unlock(&global_lock);
        if (candidate == n) return NULL;
        if (is_prime(candidate)) {
            pthread_mutex_lock(&global_lock);
            increment_primes();
            pthread_mutex_unlock(&global_lock);
        }
    }
}

int main(int argc, char **argv) {
    int m;
    if (argc != 3 || parse_positive(argv[1], &n) || parse_positive(argv[2], &m)) {
        fprintf(stderr, "Usage: %s positive-n positive-m\n", argv[0]); return 1;
    }
    pthread_t *threads = calloc((size_t)m, sizeof *threads);
    if (!threads) { perror("calloc"); return 1; }
    int started = 0, failed = 0;
    for (int i = 0; i < m; ++i) {
        int rc = pthread_create(&threads[i], NULL, check_primes, NULL);
        if (rc) { fprintf(stderr, "pthread_create: %s\n", strerror(rc)); failed = 1; break; }
        ++started;
    }
    for (int i = 0; i < started; ++i) {
        int rc = pthread_join(threads[i], NULL);
        if (rc) { fprintf(stderr, "pthread_join: %s\n", strerror(rc)); failed = 1; }
    }
    if (!failed) printf("Primes in [0, %d): %d\n", n, c);
    pthread_mutex_destroy(&global_lock);
    free(threads);
    return failed;
}
