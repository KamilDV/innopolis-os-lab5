CC ?= cc
CFLAGS = -O2 -std=c11 -Wall -Wextra -Wpedantic -pthread
PROGRAMS = channel publisher subscriber ex2 ex3 ex4 ex5 ex6 ex7

all: $(PROGRAMS)

%: %.c
	$(CC) $(CFLAGS) $< -o $@

examples:
	$(CC) $(CFLAGS) example_pipe.c -o example_pipe
	$(CC) $(CFLAGS) example_threads.c -o example_threads
	$(CC) $(CFLAGS) example_race.c -o example_race
	$(CC) $(CFLAGS) example_join.c -o example_join
	$(CC) $(CFLAGS) example_mutex.c -o example_mutex

clean:
	rm -f $(PROGRAMS) example_pipe example_threads example_race example_join example_mutex

.PHONY: all examples clean
