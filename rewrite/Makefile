CC = clang
CFLAGS = -g -fcolor-diagnostics -std=gnu11 -Wall -Wextra -Werror `pkg-config --cflags sdl2 SDL2_ttf`
LDFLAGS = `pkg-config --libs sdl2 SDL2_ttf` -lm

ppm-efx: ppm-efx.o file_io.o effects.o gui.o
	$(CC) ppm-efx.o file_io.o effects.o gui.o -o ppm-efx $(LDFLAGS)

ppm-efx.o: ppm-efx.c headers/effects.h
	$(CC) $(CFLAGS) -c ppm-efx.c -o ppm-efx.o

file_io.o: file_io.c headers/file_io.h
	$(CC) $(CFLAGS) -c file_io.c -o file_io.o

effects.o: effects.c headers/effects.h
	$(CC) $(CFLAGS) -c effects.c -o effects.o

gui.o: gui.c headers/gui.h
	$(CC) $(CFLAGS) -c gui.c -o gui.o

clean:
	rm -f ppm-efx ppm-efx.o file_io.o effects.o gui.o
