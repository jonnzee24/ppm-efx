CC = clang
CFLAGS = -g -fcolor-diagnostics -std=gnu11 -Wall -Wextra -Werror -I./include `pkg-config --cflags sdl3 sdl3-ttf`
CFLAGS += -MMD -MP
LDFLAGS = `pkg-config --libs sdl3 sdl3-ttf` -lm

BUILD_DIR = build

SRCS = ppm-efx.c file_io.c effects.c gui.c tinyfiledialogs.c
OBJS = $(addprefix $(BUILD_DIR)/, $(SRCS:.c=.o))
DEPS = $(OBJS:.o=.d)

ppm-efx: $(OBJS)
	$(CC) $(OBJS) -o ppm-efx $(LDFLAGS)

$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

-include $(DEPS)

clean:
	rm -rf ppm-efx $(BUILD_DIR)

# MacOS build
MACOS_BUILD_DIR = macos-build
APP_BUNDLE = $(MACOS_BUILD_DIR)/ppm-efx.app
APP_CONTENTS = $(APP_BUNDLE)/Contents
APP_MACOS = $(APP_CONTENTS)/MacOS
APP_FRAMEWORKS = $(APP_CONTENTS)/Frameworks
APP_RESOURCES = $(APP_CONTENTS)/Resources

build-macos: clean ppm-efx
	rm -rf $(APP_BUNDLE)
	mkdir -p $(APP_MACOS) $(APP_CONTENTS) $(APP_FRAMEWORKS) $(APP_RESOURCES)
	cp ppm-efx $(APP_MACOS)/ppm-efx
	cp Info.plist $(APP_CONTENTS)/Info.plist
	cp HiburMono.ttf $(APP_RESOURCES)/HiburMono.ttf
	cp ppm-efx.icns $(APP_RESOURCES)/ppm-efx.icns

	dylibbundler --overwrite-dir --bundle-deps         \
		-x $(APP_MACOS)/ppm-efx                        \
		--dest-dir $(APP_FRAMEWORKS)                   \
		--install-path @executable_path/../Frameworks  \

	codesign --force --deep --sign - $(APP_BUNDLE)

.PHONY: clean build-macos
