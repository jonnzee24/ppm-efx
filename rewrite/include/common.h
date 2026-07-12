#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <SDL.h>

#include "gui.h"

#define WHITE (SDL_Color){255, 255, 255, 255}

static inline int clamp(int value, int min, int max) {
    const int t = value < min ? min : value;
    return t > max ? max : t;
}

static inline float clampf(float value, float min, float max) {
    const float t = value < min ? min : value;
    return t > max ? max : t;
}

typedef struct AppState {
    bool running;
    bool needs_update;
} AppState;

typedef struct Image {
    char *path;
    int width;
    int height;
    uint8_t *framebuffer;
    size_t framebuffer_size;
    uint8_t *original;
    bool loaded;
} Image;

typedef struct SDL_Context {
    int win_width;
    int win_height;
    SDL_Window *window;
    SDL_Renderer *renderer;
    SDL_Texture *texture;
    SDL_Rect texture_rect;
    SDL_Rect image_vp;
} SDL_Context;

typedef struct UserParams {
    int x_offset;
    int y_offset;
    float scale;
    bool panning;
    int mx;
    int my;
} UserParams;

typedef struct EffectFlags {
    bool warp;
    bool pixelate;
    bool dither;
    bool invert;
    bool mono;
    bool quantize;
    bool exposure;
    bool contrast;
    bool saturation;
    bool color_bias;
    bool color_shift;
} EffectFlags;

typedef struct EffectParams {
    enum {
        MIRROR_X,
        MIRROR_Y,
        UPSIDE_DOWN,
        SINE
    } warp_mode;

    float sine_length;
    float sine_amp;
    float dither_brightness;
    int pixel_size;
    bool mono_do_thresh;
    int mono_thresh;
    int bit_depth;
    float exposure_val;
    int contrast_val;
    float saturation_val;

    enum {
        R, G, B
    } color_bias;

    float color_shift;
} EffectParams;

typedef struct AppContext {
    AppState state;
    GuiState gui;
    SDL_Context sdl;
    UserParams usr;
    EffectFlags efx;
    EffectParams params;
} AppContext;

#endif
