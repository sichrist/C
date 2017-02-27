### .........  MAKEFILE .........................####
###                                              ####
### .......generic Makefile .....................####

### clang compiler ###

CC=clang
CFLAGS=-Wall -pedantic -Werror -g   



### Linker bib ###
LDFLAGS= -lpthread -lrt -lncurses

### C files einlesen ###

SOURCES = $(wildcard *.c)

### Namen der .o files aus den .c files generieren

OBJECTS = $(SOURCES:.c=.o)

### Name zum ausführen generieren

NAME = $(SOURCES:.c=)



all: Snake
	make clean

Snake: Snake.o
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $@.o

$(OBJECTS) : %.o : %.c
	$(CC) $(CFLAGS) -c $<

	

# CleanUp .......................................#



clean:
	rm -f *.o
