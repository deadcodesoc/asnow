CC      = gcc
CFLAGS  = -Wall -O2
LD      = gcc
LDFLAGS =
LIBS    = -lm
OBJS    = asnow.o frame.o

.c.o:
	$(CC) -c $(CFLAGS) -o $*.o $<

all: asnow

asnow: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

asnow-static: $(OBJS)
	$(LD) $(LDFLAGS) -static -o $@ $(OBJS) $(LIBS)

clean:
	rm -f *~ *.o asnow

asnow.man: asnow.6
	preconv $+ | nroff -man > $@
	
asnow.o: asnow.c asnow.h stamp.h Makefile

.PHONY: clean
