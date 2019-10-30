CC=gcc
CFLAGS=-I.
AR=ar
DEPS = rvrio.h rvrcmd.h rvrdefs.h
OBJ = rvrio.o miscellaneous.o power.o io.o

librvr.a: $(OBJ)
	$(AR) rcs librvr.a $(OBJ)

%o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)

