CC = clang
CFLAGS = -g -fcolor-diagnostics -std=gnu11 -Wall -Wextra -Werror -I./include -O3 `pkg-config --cflags sdl3 sdl3-ttf` 
LDFLAGS = `pkg-config --libs sdl3 sdl3-ttf` -lm 
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
	rm -f ppm-efx $(OBJS)

# MacOS build
APP_BUNDLE = ppm-efx.app
APP_CONTENTS = $(APP_BUNDLE)/Contents
APP_MACOS = $(APP_CONTENTS)/MacOS
APP_FRAMEWORKS = $(APP_CONTENTS)/Frameworks
APP_RESOURCES = $(APP_CONTENTS)/Resources

build-macos: clean ppm-efx
	rm -rf ppm-efx.app
	mkdir -p $(APP_MACOS) $(APP_CONTENTS) $(APP_FRAMEWORKS) $(APP_RESOURCES)
	cp ppm-efx $(APP_MACOS)/ppm-efx
	cp Info.plist $(APP_CONTENTS)/Info.plist
	cp Aldrich-Regular.ttf $(APP_RESOURCES)/Aldrich-Regular.ttf
	cp ppm-efx.icns $(APP_RESOURCES)/ppm-efx.icns
	dylibbundler --overwrite-dir --bundle-deps \
		-x $(APP_MACOS)/ppm-efx \
		--dest-dir $(APP_FRAMEWORKS) \
		--install-path @executable_path/../Frameworks
	codesign --force --deep --sign - $(APP_BUNDLE)

.PHONY: clean build-macos
