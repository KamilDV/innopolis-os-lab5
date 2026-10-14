# Operating Systems Lab 5: IPC and threads

Solutions to the preliminary examples and exercises 1–7 in the supplied lab.
Exercises 5–7 are marked optional in the slides and are included here.

Build with `make all examples` using a POSIX C compiler. `make clean` removes
binaries. Each exercise also has its own executable shell script. The prime
benchmark scripts compile their program, run `n=10000000` with `m=1,2,4,10,100`,
and record wall, user and system time in `exN_res.txt`. The checked-in result
files contain actual single-run measurements on macOS. Rerunning a script
replaces that exercise's results.

Exercise 1: `channel.c` transfers stdin to stdout through one unnamed pipe;
`publisher.c` and `subscriber.c` broadcast each input line through one FIFO
per subscriber. Start `./publisher N` and `./subscriber ID` for all IDs 1..N,
or use `./ex1.sh N` to open separate terminal windows on supported systems.
All subscribers must connect before the publisher reads stdin. Lines are
limited to 1024 bytes, including the newline. Use Ctrl+C to stop a long-lived
session. See `ex1.txt` for the IPC explanation.

Exercise 2: `./ex2.sh 5` creates five threads and joins each before creating
the next; a mutex orders each creation notice before its thread's output.

Exercises 3–6: run `./ex3 n m` etc. Exercise 3 partitions the range, exercise
4 reserves candidates under a mutex, exercise 5 uses a bounded queue with
condition variables, and exercise 6 uses semaphores for the same queue.
The slide for exercise 6 refers to exercise 3's condition variables, though
exercise 3 has none; `ex6.c` therefore adapts exercise 5. For `n=10000000`,
the expected count is 664579.

Exercise 7: `./ex7 ex7_sample.txt 4` computes the LCM of positive integers
from a file. Its worker threads reserve one number at a time under a mutex
and update the shared LCM; overflow of `uint64_t` is reported as an error.
`ex7.sh` varies thread count and number count and records timings. The sample
contains integers 2–20; the benchmark repeats them to construct larger files.

The `example_*` files cover the preliminary pipe, thread creation, race,
join and mutex prompts, with their explanations in `examples.txt`.
