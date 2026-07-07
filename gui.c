#include "gui.h"

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

int draw_hud(SDL_Renderer *renderer, int x_pos, int y_pos,
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
    
    draw_text(renderer, x_pos, y_offset, DARK_RED, "EFFECTS");
    y_offset += line_spacing;
    
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
    DRAW_EFX_FLAG("[P] Pixelate", effects.pixelate, 10);

    #undef DRAW_EFX_FLAG

    #define DRAW_EFX_PARAM(fmt_string, value, efx_mode) \
        snprintf(buffer, sizeof(buffer), fmt_string, value); \
        color = (efx_mode == mode) ? WHITE : GRAY; \
        draw_text(renderer, x_pos, y_pos + y_offset, color, buffer); \
        y_offset += line_spacing;
    
    y_offset += line_spacing;
    draw_text(renderer, x_pos, y_offset, DARK_RED, "PARAMETERS");
    y_offset += line_spacing;
   
    DRAW_EFX_PARAM("Bit Depth: %d", params.bit_depth, 4);
    DRAW_EFX_PARAM("Warp Mode: %d", params.warp_mode, 2);
    DRAW_EFX_PARAM("Sine wave lenght: %d", (int)params.sine_length, 2);
    DRAW_EFX_PARAM("Sine wave amplitude: %d", (int)params.sine_amp, 2);
    DRAW_EFX_PARAM("Mono Threshold: %s", params.mono_do_thresh == 0.0f ? "off" : "on", 3);
    DRAW_EFX_PARAM("Threshold Value: %d", params.mono_thresh, 3);
    DRAW_EFX_PARAM("Dither Brightness: %.2f", params.dither_brightness, 1);
    DRAW_EFX_PARAM("Exposure Value: %.2f", params.exposure_val, 6);
    DRAW_EFX_PARAM("Contrast Value: %d", params.contrast_val, 7);
    DRAW_EFX_PARAM("Saturation Value: %.2f", params.saturation_val, 9);
    DRAW_EFX_PARAM("Color Shift: %.2f", params.color_shift, 5);
    DRAW_EFX_PARAM("Color Bias: %c", (params.color_bias == 0.0f ? 'R' : params.color_bias == 1.0f ? 'G' : 'B'), 8);
    DRAW_EFX_PARAM("Pixel Size: %d", params.pixel_size, 10);

    #undef DRAW_EFX_PARAM

    return y_offset;
}

void cleanup_gui(void) {
    if(font != NULL) {
        TTF_CloseFont(font);
    }
    TTF_Quit();
}
