#include <pthread.h>
#include <stdio.h>

static int counter;
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
static void *unsafe(void *arg) {
    (void)arg;
    for (int i = 0; i < 1000; ++i) ++counter;
    return NULL;
}
static void *safe(void *arg) {
    (void)arg;
    for (int i = 0; i < 1000; ++i) {
        pthread_mutex_lock(&lock); ++counter; pthread_mutex_unlock(&lock);
    }
    return NULL;
}
static int test(void *(*worker)(void *)) {
    pthread_t threads[1000];
    for (int i = 0; i < 1000; ++i)
        if (pthread_create(&threads[i], NULL, worker, NULL)) return 1;
    for (int i = 0; i < 1000; ++i) pthread_join(threads[i], NULL);
    return 0;
}
int main(void) {
    if (test(unsafe)) return 1;
    printf("Without mutex: %d\n", counter);
    counter = 0;
    if (test(safe)) return 1;
    printf("With mutex: %d\n", counter);
    pthread_mutex_destroy(&lock);
    return 0;
}
