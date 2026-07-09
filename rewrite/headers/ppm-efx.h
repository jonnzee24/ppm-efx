#ifndef PPM_EFX_H
#define PPM_EFX_H

#include "common.h"

#define MARGIN_Y 100
#define MARGIN_X 400

int init_sdl(SDL_Context *sdl, const Image *image);
void update(SDL_Context *sdl, Image *image, const EffectFlags *efx, const EffectParams *params);
void render(SDL_Context *sdl, const Image *image, UserParams *usr);
void process_events(UserParams *usr, SDL_Context *sdl, AppState *state, EffectFlags *efx);

#endif
