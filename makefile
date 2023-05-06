all: main

main: main.o renderer.o
	gcc -fsanitize=undefined -g $^ -o $@  -lSDL2_gfx `sdl2-config --libs` -lm

.c.o: 
	gcc -fsanitize=undefined -g -Wall -pedantic `sdl2-config --cflags` -c  $<

renderer.o: renderer.c renderer.h

main.o: shapes.h main.c renderer.h

clean:
	-rm renderer.o main.o main