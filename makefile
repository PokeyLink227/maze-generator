CC = gcc
CFLAGS = -Wall -Wextra -pedantic-errors

maze: obj\maze.o obj\bmp.o src\color.h
	$(CC) $(CFLAGS) obj\maze.o obj\bmp.o -o maze

obj\maze.o: src\maze.c src\bmp.h
	$(CC) $(CFLAGS) src\maze.c -c -o obj\maze.o

obj\bmp.o: src\bmp.c src\bmp.h
	$(CC) $(CFLAGS) src\bmp.c -c -o obj\bmp.o

clean:
	del obj\* maze.exe
