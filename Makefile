CC = clang
CFLAGS = -g -fcolor-diagnostics -std=gnu11 `sdl2-config --cflags`
LDFLAGS = `sdl2-config --libs` -lm

ppm-efx: ppm-efx.o effects.o
	$(CC) ppm-efx.o effects.o -o ppm-efx $(LDFLAGS)

ppm-efx.o: ppm-efx.c effects.h
	$(CC) $(CFLAGS) -c ppm-efx.c -o ppm-efx.o

effects.o: effects.c effects.h
	$(CC) $(CFLAGS) -c effects.c -o effects.o

clean:
	rm -f ppm-efx ppm-efx.o effects.o
