CFLAGS = -Wall -Wextra -Wnarrowing -Wfloat-equal -Wundef -Wshadow
CFLAGS += -Wunreachable-code -Wpointer-arith -Wcast-align
CFLAGS += -Wconversion -Wunreachable-code -Wstrict-prototypes
CFLAGS += -Wwrite-strings -Wswitch-default -Wswitch-enum -Winit-self -Wcast-qual


all: exp

exp: exp.c
	cc ${CFLAGS} -fopenmp exp.c -o exp

clean:
	rm -f exp
