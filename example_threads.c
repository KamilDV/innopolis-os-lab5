#include <pthread.h>
#include <stdio.h>
#include <string.h>

static void *run(void *arg) {
    (void)arg;
    printf("Thread id: %lu\n", (unsigned long)pthread_self());
    return NULL;
}

int main(void) {
    pthread_t a, b;
    int rc = pthread_create(&a, NULL, run, NULL);
    if (rc) { fprintf(stderr, "pthread_create: %s\n", strerror(rc)); return 1; }
    rc = pthread_create(&b, NULL, run, NULL);
    if (rc) { fprintf(stderr, "pthread_create: %s\n", strerror(rc)); pthread_join(a, NULL); return 1; }
    pthread_join(a, NULL); pthread_join(b, NULL);
    return 0;
}
