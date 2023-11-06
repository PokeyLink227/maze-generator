CC = gcc
CFLAGS = -Wall -Wextra -pedantic-errors

maze: maze.o bmp.o
maze.o: maze.c bmp.h
bmp.o: bmp.c bmp.h

clean:
	del *.o maze.exe
