#include "gui.h"

#define WHITE (SDL_Color){255, 255, 255, 255}
#define GRAY (SDL_Color){180, 180, 180, 255}
#define DARK_RED (SDL_Color){100, 0, 0, 255}
#define DARK_GREEN (SDL_Color){0, 100, 0, 255}

static TTF_Font *font = NULL;

int init_gui(void) {
    if(TTF_Init() != 0) {
        fprintf(stderr, "Failed to initialize SDL2_TTF%s\n", SDL_GetError());
        return 1;
    }
    font = TTF_OpenFont("Aldrich-Regular.ttf", 20);

    if(font == NULL) {
        fprintf(stderr, "Failed to open TTF font%s\n", SDL_GetError());
    }
    return 0;
}

void draw_text(SDL_Renderer *renderer, int x_pos, int y_pos,
               SDL_Color font_color, char *text) {

    int text_width = 0;
    int text_height = 0;

    SDL_Surface *surface = TTF_RenderText_Blended(font, text, font_color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    TTF_SizeText(font, text, &text_width, &text_height);
    SDL_Rect rect = {x_pos, y_pos, text_width, text_height};

    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void draw_hud(SDL_Renderer *renderer, int x_pos, int y_pos,
              EffectFlags effects, EffectParams params, int mode) {

    char buffer[64];
    int line_spacing = 30;
    int y_offset = line_spacing;
    SDL_Color color;

    #define DRAW_EFX_FLAG(name, flag, efx_mode) \
        snprintf(buffer, sizeof(buffer), "%s: %s", name, (flag) ? "on" : "off"); \
        color = (flag) ? DARK_GREEN : GRAY; \
        color = (efx_mode == mode) ? WHITE : color; \
        draw_text(renderer, x_pos, y_pos + y_offset, color, buffer); \
        y_offset += line_spacing;
    
    y_offset += line_spacing;
    draw_text(renderer, x_pos, y_offset, DARK_RED, "EFFECTS");
    y_offset += line_spacing;
    
    // The last parameter is its corresponding mode in ppm-efx.c
    // I use it to change the color of the text if its mode is on
    // For effects that dont have a mode i just put 100 lol
    DRAW_EFX_FLAG("[W] Warp", effects.warp, 2);
    DRAW_EFX_FLAG("[Q] Quantize", effects.quantize, 4);
    DRAW_EFX_FLAG("[M] Mono", effects.mono, 3);
    DRAW_EFX_FLAG("[I] Invert", effects.invert, 100);
    DRAW_EFX_FLAG("[D] Dithering", effects.dither, 1);
    DRAW_EFX_FLAG("[E] Exposure", effects.exposure, 6);
    DRAW_EFX_FLAG("[C] Contrast", effects.contrast, 7);
    DRAW_EFX_FLAG("[S] Saturation", effects.saturation, 9);
    DRAW_EFX_FLAG("[Z] Color Shift", effects.shift, 5);
    DRAW_EFX_FLAG("[X] Color Bias", effects.color, 8);
    DRAW_EFX_FLAG("[B] Blur", effects.blur, 100);

    #undef DRAW_EFX_FLAG

    #define DRAW_EFX_PARAM(name, value, efx_mode) \
    snprintf(buffer, sizeof(buffer), "%s: %.2f", name, (float)value); \
    color = (efx_mode == mode) ? WHITE : GRAY; \
    draw_text(renderer, x_pos, y_pos + y_offset, color, buffer); \
    y_offset += line_spacing;
    
    y_offset += line_spacing;
    draw_text(renderer, x_pos, y_offset, DARK_RED, "PARAMETERS");
    y_offset += line_spacing;
   
    DRAW_EFX_PARAM("Bit Depth", params.bit_depth, 4);
    DRAW_EFX_PARAM("Warp Mode", params.warp_mode, 2);
    DRAW_EFX_PARAM("Mono Do Threshold", params.mono_do_thresh, 3);
    DRAW_EFX_PARAM("Mono Threshold", params.mono_thresh, 3);
    DRAW_EFX_PARAM("Dither Bightness", params.dither_brightness, 1);
    DRAW_EFX_PARAM("Exposure Value", params.exposure_val, 6);
    DRAW_EFX_PARAM("Contrast Value", params.contrast_val, 7);
    DRAW_EFX_PARAM("Saturation Value", params.saturation_val, 9);
    DRAW_EFX_PARAM("Color Shift", params.color_shift, 5);
    DRAW_EFX_PARAM("Color Bias", params.color_bias, 8);

    #undef DRAW_EFX_PARAM
}

void cleanup_gui(void) {
    if(font != NULL) {
        TTF_CloseFont(font);
    }
    TTF_Quit();
}
