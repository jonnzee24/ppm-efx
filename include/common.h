#ifndef COMMON_H
#define COMMON_H

#include <stdbool.h>
#include <stdint.h>
#include <SDL3/SDL.h>

#include "gui.h"

#define MARGIN_Y 60
#define MARGIN_X 400

#define WHITE (SDL_Color){255, 255, 255, 255}
#define GREY (SDL_Color){120, 120, 120, 255}

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
    bool image_loaded;
} AppState;

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
} SDL_Context;

typedef struct UserParams {
    float x_offset;
    float y_offset;
    float scale;
    bool panning;
    float mx, my;
} UserParams;

typedef struct EffectFlags {
    bool warp;
    bool pixelate;
    bool dither;
    bool invert;
    bool mono;
    bool threshold;
    bool quantize;
    bool exposure;
    bool contrast;
    bool saturation;
    bool color_bias;
    bool color_shift;
    bool blur;
} EffectFlags;

typedef struct EffectParams {
    enum {
        MIRROR_X,
        MIRROR_Y,
        UPSIDE_DOWN,
        SINE
    } warp_mode;
    int threshold_mode;
    int dither_mode;
        enum {
        R, G, B
    } color_bias;

    float sine_length;
    float sine_amp;
    float pixel_size;
    float threshold_val;
    float bit_depth;
    float exposure_val;
    float contrast_val;
    float saturation_val;
    float color_shift_val;
    float blur_size;
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
