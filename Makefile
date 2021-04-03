CC = gcc
CFLAGS = -std=c99 -pedantic -Wall -Wextra
LDFLAGS = -lm -lmingw32 -lSDL2main -lSDL2

all: chip8.exe

%.o: %.c chip8.h instructions.h
	$(CC) $(CFLAGS) -c -o $@ $<

chip8.exe: main.o chip8.o
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)