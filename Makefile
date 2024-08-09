CFLAGS=-Wall -Wextra -std=c99

.PHONY: clean

build:
		gcc sfl.c $(CFLAGS) -o sfl -g
run_sfl:
		./sfl
clean:
		rm -rf sfl
