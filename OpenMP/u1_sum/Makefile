CC = clang

CFLAGS = -Wall -Wextra -Wnarrowing -Wfloat-equal -Wundef -Wshadow
CFLAGS += -Wunreachable-code -Wpointer-arith -Wcast-align
CFLAGS += -Wconversion -Wunreachable-code -Wstrict-prototypes
CFLAGS += -Wwrite-strings -Wswitch-default -Wswitch-enum -Winit-self -Wcast-qual


all: sum

harmonic: sum.c
	${CC} ${CFLAGS} -fopenmp sum.c -o sum

clean:
	rm -f sum
