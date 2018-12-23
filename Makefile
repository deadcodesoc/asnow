CC      = gcc
CFLAGS  = -Wall -O2
LD      = gcc
LDFLAGS =
LIBS    = -lm
OBJS    = asnow.o

.c.o:
	$(CC) -c $(CFLAGS) -o $*.o $<

all: asnow

asnow: $(OBJS) asnow.h
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

clean:
	rm -f *~ *.o asnow

asnow.man: asnow.6
	preconv $+ | nroff -man > $@
	

.PHONY: clean
