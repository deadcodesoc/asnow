all: asnow

asnow: asnow.o
	$(CC) $(LDFLAGS) -o $@ $+ -lm

clean:
	rm -f *~ *.o asnow

.PHONY: clean
