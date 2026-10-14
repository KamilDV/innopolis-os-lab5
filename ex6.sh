#!/bin/sh
set -eu
cd "$(dirname "$0")"
${CC:-cc} -O2 -std=c11 -Wall -Wextra -pthread ex6.c -o ex6
: > ex6_res.txt
for m in 1 2 4 10 100; do
  echo "m=$m" >> ex6_res.txt
  /usr/bin/time -p ./ex6 10000000 "$m" >> ex6_res.txt 2>&1
done
