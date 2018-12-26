VERSION = 0.1

CC      = cc
CFLAGS  = -Wall -O2 -DVERSION=\"$(VERSION)\"
LD      = cc
LDFLAGS =
LIBS    = -lm
OBJS    = asnow.o flake.o frame.o line.o

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
	
asnow.o: asnow.c asnow.h stamp.h
frame.o: frame.c frame.h
flake.o: flake.c flake.h

$(OBJS): Makefile

.PHONY: clean
