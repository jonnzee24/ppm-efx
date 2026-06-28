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
    font = TTF_OpenFont("Comme-Regular.ttf", 25);

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
    int line_spacing = 40;
    int y_offset = line_spacing;
    SDL_Color color;

    #define DRAW_EFX_FLAG(name, flag) \
        snprintf(buffer, sizeof(buffer), "%s: %s", name, (flag) ? "on" : "off"); \
        color = (flag) ? DARK_GREEN : GRAY; \
        draw_text(renderer, x_pos, y_pos + y_offset, color, buffer); \
        y_offset += line_spacing;
    
    draw_text(renderer, x_pos, y_pos, DARK_RED, "EFFECTS");

    DRAW_EFX_FLAG("Warp", effects.warp);
    DRAW_EFX_FLAG("Invert", effects.invert);
    DRAW_EFX_FLAG("Mono", effects.mono);
    DRAW_EFX_FLAG("Quantize", effects.quantize);
    DRAW_EFX_FLAG("Dithering", effects.dither);
    DRAW_EFX_FLAG("Shift", effects.shift);
    DRAW_EFX_FLAG("Exposure", effects.exposure);
    DRAW_EFX_FLAG("Contrast", effects.contrast);
    DRAW_EFX_FLAG("Saturation", effects.saturation);
    DRAW_EFX_FLAG("Color", effects.color);
    DRAW_EFX_FLAG("Blur", effects.blur);

    #undef DRAW_EFX_FLAG
}

void cleanup_gui(void) {
    if(font != NULL) {
        TTF_CloseFont(font);
    }
    TTF_Quit();
}
