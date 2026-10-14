#include "prime.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CAPACITY 1024
static int queue[CAPACITY], head, tail, size, done, n, count;
static pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t not_empty = PTHREAD_COND_INITIALIZER;
static pthread_cond_t not_full = PTHREAD_COND_INITIALIZER;

static void produce(int value) { queue[tail] = value; tail = (tail + 1) % CAPACITY; ++size; }
static int consume(void) { int value = queue[head]; head = (head + 1) % CAPACITY; --size; return value; }
static void increment_primes(void) { ++count; }

static void *producer(void *arg) {
    (void)arg;
    for (int i = 2; i < n; ++i) {
        if (i == 2 || i == 3) {
            pthread_mutex_lock(&count_lock);
            increment_primes();
            pthread_mutex_unlock(&count_lock);
            continue;
        }
        if (i % 2 == 0 || i % 3 == 0) continue;
        pthread_mutex_lock(&queue_lock);
        while (size == CAPACITY) pthread_cond_wait(&not_full, &queue_lock);
        produce(i);
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&queue_lock);
    }
    pthread_mutex_lock(&queue_lock);
    done = 1;
    pthread_cond_broadcast(&not_empty);
    pthread_mutex_unlock(&queue_lock);
    return NULL;
}

static void *consumer(void *arg) {
    (void)arg;
    for (;;) {
        pthread_mutex_lock(&queue_lock);
        while (!size && !done) pthread_cond_wait(&not_empty, &queue_lock);
        if (!size && done) { pthread_mutex_unlock(&queue_lock); break; }
        int candidate = consume();
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&queue_lock);
        if (is_prime(candidate)) {
            pthread_mutex_lock(&count_lock);
            increment_primes();
            pthread_mutex_unlock(&count_lock);
        }
    }
    return NULL;
}

int main(int argc, char **argv) {
    int m;
    if (argc != 3 || parse_positive(argv[1], &n) || parse_positive(argv[2], &m)) {
        fprintf(stderr, "Usage: %s positive-n positive-m\n", argv[0]); return 1;
    }
    pthread_t *workers = calloc((size_t)m, sizeof *workers), source;
    if (!workers) { perror("calloc"); return 1; }
    int started = 0, failed = 0;
    for (int i = 0; i < m; ++i) {
        int rc = pthread_create(&workers[i], NULL, consumer, NULL);
        if (rc) { fprintf(stderr, "pthread_create: %s\n", strerror(rc)); failed = 1; break; }
        ++started;
    }
    if (!failed) {
        int rc = pthread_create(&source, NULL, producer, NULL);
        if (rc) { fprintf(stderr, "pthread_create: %s\n", strerror(rc)); failed = 1; }
        else pthread_join(source, NULL);
    }
    if (failed) {
        pthread_mutex_lock(&queue_lock); done = 1;
        pthread_cond_broadcast(&not_empty); pthread_mutex_unlock(&queue_lock);
    }
    for (int i = 0; i < started; ++i) pthread_join(workers[i], NULL);
    if (!failed) printf("Primes in [0, %d): %d\n", n, count);
    free(workers);
    pthread_cond_destroy(&not_empty); pthread_cond_destroy(&not_full);
    pthread_mutex_destroy(&queue_lock); pthread_mutex_destroy(&count_lock);
    return failed;
}
