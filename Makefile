CC = clang
CFLAGS = -g -fcolor-diagnostics -std=gnu11 `pkg-config --cflags sdl2 SDL2_ttf`
LDFLAGS = `pkg-config --libs sdl2 SDL2_ttf` -lm

ppm-efx: ppm-efx.o effects.o gui.o
	$(CC) ppm-efx.o effects.o gui.o -o ppm-efx $(LDFLAGS)

ppm-efx.o: ppm-efx.c effects.h
	$(CC) $(CFLAGS) -c ppm-efx.c -o ppm-efx.o

effects.o: effects.c effects.h
	$(CC) $(CFLAGS) -c effects.c -o effects.o
gui.o: gui.c gui.h
	$(CC) $(CFLAGS) -c gui.c -o gui.o

clean:
	rm -f ppm-efx ppm-efx.o effects.o gui.o
