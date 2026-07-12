#ifndef PPM_EFX_H
#define PPM_EFX_H

#include "common.h"
#include "gui.h"

int init_sdl(AppContext *ctx);

int create_image_texture(SDL_Context *sdl, Image *image);
void update(AppContext *ctx, Image *image);
void render(AppContext *ctx, Image *image);

void process_events(AppContext *ctx);

#endif
