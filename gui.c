#include "gui.h"

static TTF_Font *font = NULL;

int init_gui(void) {
    if(TTF_Init() != 0) {
        fprintf(stderr, "Failed to initialize SDL2_TTF%s\n", SDL_GetError());
        return 1;
    }
    font = TTF_OpenFont("Comme-Regular.ttf", 30);
    if(font == NULL) {
        fprintf(stderr, "Failed to open TTF font%s\n", SDL_GetError());
    }
    return 0;
}

void draw_text(SDL_Renderer *renderer, int x_pos, int y_pos, int width, int height, SDL_Color font_color, char *text) {
    SDL_Surface *surface = TTF_RenderText_Solid(font, text, font_color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect rect = {x_pos, y_pos, width, height};

    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void draw_hud(SDL_Renderer *renderer, int x_pos, int y_pos, 
              int width, int height, SDL_Color font_color,
              EffectFlags effects, EffectParams params) {

    #define DRAW_EFX_FLAG(name, flag)
}

void cleanup_gui(void) {
    if(font != NULL) {
        TTF_CloseFont(font);
    }
    TTF_Quit();
}
