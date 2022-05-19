CC = gcc

CFLAGS = -Wall -Wextra -Wnarrowing -Wfloat-equal -Wundef -Wshadow
CFLAGS += -Wunreachable-code -Wpointer-arith -Wcast-align
CFLAGS += -Wwrite-strings -Wswitch-default -Wswitch-enum -Winit-self -Wcast-qual

all: integral

integral: integral.c
	$(CC) $(CFLAGS) -pthread -lm integral.c -o integral.exe

clean:
	rm -f *.exe