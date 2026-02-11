CC = gcc
CFLAGS = -Wall -Wextra -g -pthread

all: test_memarena

test_memarena: main.o memarena.o
	$(CC) $(CFLAGS) -o test_memarena main.o memarena.o

main.o: main.c memarena.h
	$(CC) $(CFLAGS) -c main.c

memarena.o: memarena.c memarena.h
	$(CC) $(CFLAGS) -c memarena.c

clean:
	rm -f *.o test_memarena