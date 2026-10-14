#!/bin/sh
set -eu
cd "$(dirname "$0")"
${CC:-cc} -O2 -std=c11 -Wall -Wextra -pthread ex5.c -o ex5
: > ex5_res.txt
for m in 1 2 4 10 100; do
  echo "m=$m" >> ex5_res.txt
  /usr/bin/time -p ./ex5 10000000 "$m" >> ex5_res.txt 2>&1
done
