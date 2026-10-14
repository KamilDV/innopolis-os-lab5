#!/bin/sh
set -eu
cd "$(dirname "$0")"
${CC:-cc} -O2 -std=c11 -Wall -Wextra -pthread ex2.c -o ex2
./ex2 "${1:-5}"
