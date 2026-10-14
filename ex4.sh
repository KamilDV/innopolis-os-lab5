#!/bin/sh
set -eu
cd "$(dirname "$0")"
${CC:-cc} -O2 -std=c11 -Wall -Wextra -pthread ex4.c -o ex4
: > ex4_res.txt
for m in 1 2 4 10 100; do
  echo "m=$m" >> ex4_res.txt
  /usr/bin/time -p ./ex4 10000000 "$m" >> ex4_res.txt 2>&1
done
