PREFIX = /usr/local
CC = gcc
CFLAGS = `pkg-config --cflags glib-2.0`
LDFLAGS = `pkg-config --libs glib-2.0` -lX11

.PHONY: all
all:
	$(CC) -c $(CFLAGS) xppu.c 
	$(CC) xppu.o $(LDFLAGS) -o xppu

.PHONY: install
install: xppu
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $< $(DESTDIR)$(PREFIX)/bin/xppu

.PHONY: uninstall
uninstall:
	rm -f $(DESTDIR)$(PREFIX)/bin/xppu

.PHONY: clean
clean:
	rm -f *.o xppu
