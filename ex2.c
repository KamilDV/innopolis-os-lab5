#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Thread {
    pthread_t id;
    int i;
    char message[256];
} Thread;

static pthread_mutex_t print_lock = PTHREAD_MUTEX_INITIALIZER;

static void *run(void *arg) {
    Thread *thread = arg;
    pthread_mutex_lock(&print_lock);
    printf("Thread %d (id %lu): %s\n", thread->i + 1,
           (unsigned long)thread->id, thread->message);
    pthread_mutex_unlock(&print_lock);
    return NULL;
}

int main(int argc, char **argv) {
    if (argc != 2) { fprintf(stderr, "Usage: %s n\n", argv[0]); return 1; }
    char *end;
    long n = strtol(argv[1], &end, 10);
    if (*end || n < 1 || n > 100000) { fputs("n must be 1..100000\n", stderr); return 1; }
    Thread *threads = calloc((size_t)n, sizeof *threads);
    if (!threads) { perror("calloc"); return 1; }
    for (long i = 0; i < n; ++i) {
        threads[i].i = (int)i;
        snprintf(threads[i].message, sizeof threads[i].message,
                 "Hello from thread %ld", i);
        pthread_mutex_lock(&print_lock);
        int rc = pthread_create(&threads[i].id, NULL, run, &threads[i]);
        if (rc) { pthread_mutex_unlock(&print_lock); fprintf(stderr, "pthread_create: %s\n", strerror(rc)); free(threads); return 1; }
        printf("Thread %ld is created\n", i + 1);
        pthread_mutex_unlock(&print_lock);
        rc = pthread_join(threads[i].id, NULL);
        if (rc) { fprintf(stderr, "pthread_join: %s\n", strerror(rc)); free(threads); return 1; }
        printf("Thread %ld exits\n", i + 1);
    }
    free(threads);
    pthread_mutex_destroy(&print_lock);
    return 0;
}
