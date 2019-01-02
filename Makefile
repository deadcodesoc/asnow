VERSION = 0.1

CC      = cc
CFLAGS  = -std=c99 -Wall -O2 -DVERSION=\"$(VERSION)\"
LD      = cc
LDFLAGS =
LIBS    = -lm
OBJS    = asnow.o flake.o frame.o line.o circle.o io.o

.c.o:
	$(CC) -c $(CFLAGS) -o $*.o $<

all: asnow

asnow: $(OBJS)
	$(LD) $(LDFLAGS) -o $@ $(OBJS) $(LIBS)

asnow-static: $(OBJS)
	$(LD) $(LDFLAGS) -static -o $@ $(OBJS) $(LIBS)

install: asnow
	mkdir -p $(DESTDIR)/bin $(DESTDIR)/share/man/man6
	install -m 755 asnow $(DESTDIR)/bin
	install -m 644 asnow.6 $(DESTDIR)/share/man/man6

clean:
	rm -f *~ *.o asnow

asnow.man: asnow.6
	preconv $+ | nroff -man > $@
	
asnow.o: asnow.c asnow.h stamp.h
frame.o: frame.c frame.h
flake.o: flake.c flake.h
line.o: line.c frame.h
circle.o: circle.c frame.h
io.o: io.c io.h

$(OBJS): Makefile

.PHONY: clean
