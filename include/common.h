#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <SDL3/SDL.h>

#include "effects.h"
#include "file_io.h"
#include "gui.h"

#define MARGIN_Y 60
#define MARGIN_LEFT 30
#define MARGIN_RIGHT 400

#define WHITE (SDL_Color){250, 250, 250, 255}
#define BLACK (SDL_Color){20, 20, 20, 255}
#define DARK_GRAY (SDL_Color){98, 98, 98, 255}
#define DARKER_GRAY (SDL_Color){60, 60, 60, 255}
#define LIGHT_GRAY (SDL_Color){147, 147, 147, 255}
#define MID_GRAY (SDL_Color){124,124,124,255}
#define ORANGE (SDL_Color){194, 131, 96, 255}

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

typedef struct AppState {
    bool running;
    bool image_needs_update;
} AppState;

typedef struct AppContext {
    AppState state;
    SDLContext sdl;
    GuiContext gui;
    UserParams usr;
    EfxPipeline efx;
} AppContext;

int resize_image(Image *image, int new_w, int new_h);

#endif
