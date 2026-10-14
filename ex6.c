#define _POSIX_C_SOURCE 200809L
#include "prime.h"
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#define CAPACITY 1024
static int queue[CAPACITY], head, tail, n, count;
static pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t count_lock = PTHREAD_MUTEX_INITIALIZER;
static sem_t *items, *slots;

static int wait_sem(sem_t *sem) {
    while (sem_wait(sem) < 0) if (errno != EINTR) return -1;
    return 0;
}

static void produce(int value) { queue[tail] = value; tail = (tail + 1) % CAPACITY; }
static int consume(void) { int value = queue[head]; head = (head + 1) % CAPACITY; return value; }
static void increment_primes(void) { ++count; }

static void *producer(void *arg) {
    int consumers = *(int *)arg;
    for (int i = 2; i < n; ++i) {
        if (i == 2 || i == 3) {
            pthread_mutex_lock(&count_lock); increment_primes(); pthread_mutex_unlock(&count_lock);
            continue;
        }
        if (i % 2 == 0 || i % 3 == 0) continue;
        if (wait_sem(slots)) return (void *)1;
        pthread_mutex_lock(&queue_lock); produce(i); pthread_mutex_unlock(&queue_lock);
        sem_post(items);
    }
    for (int i = 0; i < consumers; ++i) {
        if (wait_sem(slots)) return (void *)1;
        pthread_mutex_lock(&queue_lock); produce(-1); pthread_mutex_unlock(&queue_lock);
        sem_post(items);
    }
    return NULL;
}

static void *consumer(void *arg) {
    (void)arg;
    for (;;) {
        if (wait_sem(items)) return (void *)1;
        pthread_mutex_lock(&queue_lock); int value = consume(); pthread_mutex_unlock(&queue_lock);
        sem_post(slots);
        if (value == -1) return NULL;
        if (is_prime(value)) {
            pthread_mutex_lock(&count_lock); increment_primes(); pthread_mutex_unlock(&count_lock);
        }
    }
}

static sem_t *make_sem(const char *suffix, unsigned value) {
    char name[80]; snprintf(name, sizeof name, "/lab5_%ld_%s", (long)getpid(), suffix);
    sem_t *sem = sem_open(name, O_CREAT | O_EXCL, 0600, value);
    if (sem != SEM_FAILED) sem_unlink(name);
    return sem;
}

int main(int argc, char **argv) {
    int m;
    if (argc != 3 || parse_positive(argv[1], &n) || parse_positive(argv[2], &m)) {
        fprintf(stderr, "Usage: %s positive-n positive-m\n", argv[0]); return 1;
    }
    items = make_sem("items", 0); slots = make_sem("slots", CAPACITY);
    if (items == SEM_FAILED || slots == SEM_FAILED) { perror("sem_open"); return 1; }
    pthread_t *workers = calloc((size_t)m, sizeof *workers), source;
    if (!workers) { perror("calloc"); return 1; }
    int started = 0, failed = 0;
    for (int i = 0; i < m; ++i) {
        int rc = pthread_create(&workers[i], NULL, consumer, NULL);
        if (rc) { fprintf(stderr, "pthread_create: %s\n", strerror(rc)); failed = 1; break; }
        ++started;
    }
    if (!failed) {
        int rc = pthread_create(&source, NULL, producer, &m);
        if (rc) { fprintf(stderr, "pthread_create: %s\n", strerror(rc)); failed = 1; }
        else { void *result; pthread_join(source, &result); if (result) failed = 1; }
    }
    if (failed) {
        for (int i = 0; i < started; ++i) {
            wait_sem(slots);
            pthread_mutex_lock(&queue_lock); produce(-1); pthread_mutex_unlock(&queue_lock);
            sem_post(items);
        }
    }
    for (int i = 0; i < started; ++i) pthread_join(workers[i], NULL);
    if (!failed) printf("Primes in [0, %d): %d\n", n, count);
    free(workers); sem_close(items); sem_close(slots);
    pthread_mutex_destroy(&queue_lock); pthread_mutex_destroy(&count_lock);
    return failed;
}
