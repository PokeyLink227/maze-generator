CC = gcc
CFLAGS = -Wall -Wextra -pedantic-errors

maze: obj\maze.o obj\bmp.o obj\terminal_tools.o
	$(CC) $(CFLAGS) obj\maze.o obj\bmp.o obj\terminal_tools.o -o maze

obj\maze.o: src\maze.c src\bmp.h src\terminal_tools.h
	$(CC) $(CFLAGS) src\maze.c -c -o obj\maze.o

obj\bmp.o: src\bmp.c src\bmp.h
	$(CC) $(CFLAGS) src\bmp.c -c -o obj\bmp.o

obj\terminal_tools.o: src\terminal_tools.c src\terminal_tools.h
	$(CC) $(CFLAGS) src\terminal_tools.c -c -o obj\terminal_tools.o

clean:
	del obj\* maze.exe /Q
