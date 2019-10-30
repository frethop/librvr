CC=gcc
CFLAGS=-I. -I..
DEPS = ../librvr.a

all: first powertest1

%o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

first: first.o $(DEPS)
	$(CC) -o first first.o -L.. -lrvr

powertest1: powertest1.o $(DEPS)
	$(CC) -o powertest1 powertest1.o -L.. -lrvr

