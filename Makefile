
# CC=gcc
CC=clang
OPTFLAGS=-ffast-math -fno-finite-math-only -O3 -march=native
CFLAGS=-fno-strict-aliasing -std=c11 -Wall -pipe -Isrc/ -D_BSD_SOURCE -g
LDFLAGS=-lm -lc -lpthread -lSDL -lGL
COLOR=srgb
include config.mk

.PHONY: clean all

all: eu

eu: Makefile src/*.c src/*.h
	cp -f color/$(COLOR).h src/colorout_custom.h
	$(CC) $(CFLAGS) $(OPTFLAGS) src/*.c $(LDFLAGS) -o eu

clean:
	rm -f eu
