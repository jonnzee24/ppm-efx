#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <SDL3/SDL.h>

#define MARGIN_Y 60
#define MARGIN_LEFT 30
#define MARGIN_RIGHT 400

#define WHITE (SDL_Color){250, 250, 250, 255}
#define BLACK (SDL_Color){20, 20, 20, 255}
#define DARK_GRAY (SDL_Color){98, 98, 98, 255}
#define LIGHT_GRAY (SDL_Color){147, 147, 147, 255}
#define MID_GRAY (SDL_Color){124,124,124,255}
#define ORANGE (SDL_Color){194, 131, 96, 255}

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

typedef struct Image {
    char *path;
    int width;
    int height;

    uint8_t *framebuffer;
    size_t framebuffer_size;
    uint8_t *original;
    uint8_t *scratch;

    SDL_Texture *texture;
    SDL_FRect texture_rect;

    bool needs_reload;
} Image;

typedef struct SDL_Context {
    int win_width;
    int win_height;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Rect image_vp;
} SDLContext;

typedef struct UserParams {
    float scale;
    bool panning;
    float mx, my;
} UserParams;

#endif
