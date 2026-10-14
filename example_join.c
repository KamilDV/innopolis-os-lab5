#include <pthread.h>
#include <stdio.h>

static int counter;
static void *run(void *arg) {
    (void)arg;
    for (int i = 0; i < 1000000; ++i) ++counter;
    return NULL;
}

int main(void) {
    pthread_t thread;
    for (int i = 0; i < 2; ++i) {
        if (pthread_create(&thread, NULL, run, NULL)) return 1;
        if (pthread_join(thread, NULL)) return 1;
    }
    printf("Counter: %d\n", counter);
    return 0;
}
