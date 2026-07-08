#ifndef PPM_EFX_H
#define PPM_EFX_H

#include <stdint.h>
#include <SDL.h>

typedef struct {
    char *path;
    int width;
    int height;
    uint8_t *framebuffer;
    size_t framebuffer_size;
    uint8_t *original;
} Image;

typedef struct {
    int win_width;
    int win_height;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Rect texture_rect;
} SDL_Context;

typedef enum {
    MODE_NONE = 0,
    MODE_WARP = 1,
    MODE_MONO = 2,
    MODE_QUANTIZE = 3,
    MODE_DITHER = 4,
    MODE_SHIFT = 5,
    MODE_EXPOSURE = 6,
    MODE_CONTRAST = 7,
    MODE_SATURATION = 8,
    MODE_COLOR_BIAS = 9,
    MODE_PIXELATE = 10
} Mode;

int init_sdl(SDL_Context *sdl, const Image *image);
void render(SDL_Context *sdl, const Image *image);

#endif
