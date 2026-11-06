#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>
#include <SDL3/SDL.h>

typedef struct SDL_Context SDLContext;
typedef struct AppContext AppContext;

typedef struct Image {
    char *path;
    int width;
    int height;
    int channels;

    uint8_t *framebuffer;
    size_t framebuffer_size;
    uint8_t *original;
    int org_w, org_h;
    uint8_t *scratch;

    SDL_Texture *texture;
    SDL_FRect texture_rect;

    bool loaded;
    bool needs_reload;
} Image;

typedef struct ImageScaleData {
    Image *image;
    float multiplier;
} ImageScaleData;

int create_image_texture(SDLContext *sdl, Image *image);
void recenter_image(AppContext *ctx, Image *image);
void *scale_image_callback(void *data); // button callback (data = ImageScaleData*)

#endif
