CC = gcc
CFLAGS = -O2 -Wall

all: xbm_dump xbm_print

xbm_dump: xbm_dump.o
	$(CC) $(CFLAGS) xbm_dump.c -o xbm_dump

xbm_print: xbm_print.o
	$(CC) $(CFLAGS) xbm_print.c -o xbm_print

.PHONY: clean
clean:
	rm -rf *o xbm_dump xbm_print