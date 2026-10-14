#ifndef PRIME_H
#define PRIME_H
#include <stdbool.h>
#include <errno.h>
#include <limits.h>
#include <stdlib.h>

static inline bool is_prime(int value) {
    if (value < 2) return false;
    if (value == 2) return true;
    if (value % 2 == 0) return false;
    for (int divisor = 3; divisor <= value / divisor; divisor += 2)
        if (value % divisor == 0) return false;
    return true;
}

static inline int parse_positive(const char *arg, int *value) {
    char *end;
    errno = 0;
    long parsed = strtol(arg, &end, 10);
    if (errno || end == arg || *end || parsed < 1 || parsed > INT_MAX) return -1;
    *value = (int)parsed;
    return 0;
}
#endif
