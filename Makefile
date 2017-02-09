CC=gcc

all:
	$(CC) xppu.c `pkg-config --cflags --libs glib-2.0` -lX11 -o xppu
clean:
	rm -f xppu
