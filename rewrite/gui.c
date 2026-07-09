#include <SDL_ttf.h>

#include "common.h"
#include "gui.h"

static TTF_Font *font = NULL;

int init_gui(void) {
    if(TTF_Init() != 0) {
        fprintf(stderr, "Failed to initialize SDL2_TTF%s\n", SDL_GetError());
        return 1;
    }

    font = TTF_OpenFont("Aldrich-Regular.ttf", 15);
    if(font == NULL) {
        fprintf(stderr, "Failed to open TTF font%s\n", SDL_GetError());
    }

    return 0;
}

void cleanup_gui(void) {
    if(font != NULL) {
        TTF_CloseFont(font);
    }
    TTF_Quit();
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

void draw_debug_info(SDL_Context *sdl, const Image *image, const UserParams *usr, const float scale) {
    char buffer[32];
    snprintf(buffer, 32, "Scale: %.2f", scale);
    draw_text(sdl->renderer, 10, 10, WHITE, buffer);
    snprintf(buffer, 32, "User Scale: %.2f", usr->scale);
    draw_text(sdl->renderer, 10, 40, WHITE, buffer);

    snprintf(buffer, 32, "Original Width: %d", image->width);
    draw_text(sdl->renderer, 10, 70, WHITE, buffer);
    snprintf(buffer, 32, "Original Height: %d", image->height);
    draw_text(sdl->renderer, 10, 100, WHITE, buffer);

    snprintf(buffer, 32, "Current Width: %d", sdl->texture_rect.w);
    draw_text(sdl->renderer, 10, 130, WHITE, buffer);
    snprintf(buffer, 32, "Current Height: %d", sdl->texture_rect.h);
    draw_text(sdl->renderer, 10, 160, WHITE, buffer);

    snprintf(buffer, 32, "X Position: %d", sdl->texture_rect.x);
    draw_text(sdl->renderer, 10, 190, WHITE, buffer);
    snprintf(buffer, 32, "Y Position: %d", sdl->texture_rect.y);
    draw_text(sdl->renderer, 10, 220, WHITE, buffer);

    snprintf(buffer, 32, "User X Offset: %d", usr->x_offset);
    draw_text(sdl->renderer, 10, 250, WHITE, buffer);
    snprintf(buffer, 32, "User Y Offset: %d", usr->y_offset);
    draw_text(sdl->renderer, 10, 280, WHITE, buffer);

    snprintf(buffer, 32, "Mouse X: %d", usr->mx);
    draw_text(sdl->renderer, 10, 310, WHITE, buffer);
    snprintf(buffer, 32, "Mouse Y: %d", usr->my);
    draw_text(sdl->renderer, 10, 340, WHITE, buffer);
}
