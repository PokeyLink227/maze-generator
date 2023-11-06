flags = -Wall -Wextra -pedantic-errors

maze: maze.o bmp.o
	@gcc -o maze maze.o bmp.o $(flags)

maze.o: maze.c bmp.h
	@gcc -c maze.c $(flags)

bmp.o: bmp.c bmp.h
	@gcc -c bmp.c $(flags)
