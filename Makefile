CC = clang
CFLAGS = -g -fcolor-diagnostics -std=gnu11 -Wall -Wextra -Werror -I./include -MMD -MP -O2 `pkg-config --cflags sdl2 SDL2_ttf` 
LDFLAGS = `pkg-config --libs sdl2 SDL2_ttf` -lm 
OBJS = ppm-efx.o file_io.o effects.o gui.o tinyfiledialogs.o

ppm-efx: $(OBJS)
	$(CC) $(OBJS) -o ppm-efx $(LDFLAGS)

ppm-efx.o: ppm-efx.c
	$(CC) $(CFLAGS) -c ppm-efx.c -o ppm-efx.o

file_io.o: file_io.c
	$(CC) $(CFLAGS) -c file_io.c -o file_io.o

effects.o: effects.c
	$(CC) $(CFLAGS) -c effects.c -o effects.o

gui.o: gui.c
	$(CC) $(CFLAGS) -c gui.c -o gui.o

tinyfiledialogs.o: tinyfiledialogs.c
	$(CC) $(CFLAGS) -c tinyfiledialogs.c -o tinyfiledialogs.o

clean:
	rm -f ppm-efx $(OBJS) *.d

-include $(OBJS:.o=.d)
