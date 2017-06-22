CC=gcc
CFLAGS=-Wall -std=c99 -pedantic -Werror -g -lX11
LDFLAGS= -lpthread -lrt

all : clipboard


clipboard : clipboard.c
	$(CC) $(CFLAGS) $(DEBUG) $^ -o $@

clean:
	rm -f *.o clipboard

.PHONY: clipboard
