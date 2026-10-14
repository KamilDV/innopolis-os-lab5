#include <pthread.h>
#include <stdio.h>

static volatile int counter;
static void *run(void *arg) {
    (void)arg;
    for (int i = 0; i < 1000000; ++i) counter = counter + 1;
    return NULL;
}

int main(void) {
    pthread_t a, b;
    if (pthread_create(&a, NULL, run, NULL) || pthread_create(&b, NULL, run, NULL)) return 1;
    pthread_join(a, NULL); pthread_join(b, NULL);
    printf("Expected 2000000, observed %d\n", counter);
    return 0;
}
