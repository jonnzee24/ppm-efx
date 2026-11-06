#ifndef UTILS_H
#define UTILS_H

#include <SDL3/SDL.h>

typedef struct SDL_Renderer SDL_Renderer;

static inline void set_draw_color(SDL_Renderer *renderer, SDL_Color c) {
    SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, c.a);
}

static inline int clamp(int value, int min, int max) {
    const int t = value < min ? min : value;
    return t > max ? max : t;
}

static inline float clampf(float value, float min, float max) {
    const float t = value < min ? min : value;
    return t > max ? max : t;
}

#endif
