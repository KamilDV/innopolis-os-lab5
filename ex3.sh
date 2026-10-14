#!/bin/sh
set -eu
cd "$(dirname "$0")"
${CC:-cc} -O2 -std=c11 -Wall -Wextra -pthread ex3.c -o ex3
: > ex3_res.txt
for m in 1 2 4 10 100; do
  echo "m=$m" >> ex3_res.txt
  /usr/bin/time -p ./ex3 10000000 "$m" >> ex3_res.txt 2>&1
done
