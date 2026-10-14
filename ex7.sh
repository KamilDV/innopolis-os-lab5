#!/bin/sh
set -eu
cd "$(dirname "$0")"
${CC:-cc} -O2 -std=c11 -Wall -Wextra -pthread ex7.c -o ex7
: > ex7_res.txt
for count in 10000 100000 1000000; do
  for m in 1 2 4 10; do
    echo "numbers=$count threads=$m" >> ex7_res.txt
    awk -v count="$count" 'NR <= 20 { a[NR]=$1 } END { for (i=1; i<=count; ++i) print a[(i-1)%NR+1] }' ex7_sample.txt > ex7_input.tmp
    /usr/bin/time -p ./ex7 ex7_input.tmp "$m" >> ex7_res.txt 2>&1
  done
done
rm -f ex7_input.tmp
