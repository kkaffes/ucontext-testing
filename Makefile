CC = gcc
CFLAGS = -Wall -O3

LDLIBS = -pthread

all: symmetric convoluted size benchmark

symmetric: symmetric.c
	$(CC) -o symmetric $(LDLIBS) $(CFLAGS) symmetric.c

convoluted: convoluted.c
	$(CC) -o convoluted $(LDLIBS) $(CFLAGS) convoluted.c

size: size.c
	$(CC) -o size $(LDLIBS) $(CFLAGS) size.c

benchmark: benchmark.c
	$(CC) -o benchmark $(LDLIBS) $(CFLAGS) benchmark.c

clean:
	rm symmetric convoluted size
