#!/bin/sh
set -eu
cd "$(dirname "$0")"
${CC:-cc} -O2 -std=c11 -Wall -Wextra example_pipe.c -o example_pipe
for i in 1 2 3 4 5 6 7 8 9 10; do ./example_pipe "${1:-hello}"; done
