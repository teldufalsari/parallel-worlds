CFLAGS = -Wall -Wextra -Wnarrowing -Wfloat-equal -Wundef -Wshadow
CFLAGS += -Wunreachable-code -Wpointer-arith -Wcast-align
CFLAGS += -Wconversion -Wunreachable-code -Wstrict-prototypes
CFLAGS += -Wwrite-strings -Wswitch-default -Wswitch-enum -Winit-self -Wcast-qual


all: primes

primes: primes.c
	cc ${CFLAGS} -lm -fopenmp primes.c -o primes

clean:
	rm -f primes
