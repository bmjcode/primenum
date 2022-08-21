CC ?= gcc
CFLAGS ?= -O3 -fPIC -Wall -Werror
CFLAGS += -std=c99
LDFLAGS ?=
LDFLAGS += -lm libprimenum.a

DEFAULT = all
all: libprimenum.a pfactor primes

libprimenum.a: list.o primenum.o
	ar cru $@ $+

pfactor: pfactor.o
	$(CC) -o $@ $+ $(LDFLAGS)

primes: primes.o
	$(CC) -o $@ $+ $(LDFLAGS)

%.o: %.c
	$(CC) -c $(CFLAGS) -o $@ $<

clean:
	rm -f libprimenum.a pfactor primes *.exe *.o
