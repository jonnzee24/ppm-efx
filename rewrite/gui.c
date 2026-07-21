#include <stdio.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>

#include "common.h"
#include "file_io.h"
#include "gui.h"
#include "app.h"

TTF_Font *font = NULL;

typedef struct {
    char text[64];
    int w, h;
    SDL_Texture *texture;
} CachedText;

#define TEXT_CACHE_MAX 32
CachedText text_cache[TEXT_CACHE_MAX];
int num_cached_text = 0;

void *exit_application(void *data);
void *reset_efx(void *data);
void *randomize_efx(void *data);

void draw_button(SDL_Renderer *renderer, Button *button);
void draw_slider(SDL_Renderer *renderer, Slider *slider);
void draw_text(SDL_Renderer *renderer, int x, int y, SDL_Color font_color, char *text);
SDL_Texture *get_cached_text(SDL_Renderer *renderer, int *w, int *h, SDL_Color font_color, char *text);

int init_gui(AppContext *ctx, Image *image) {
    if(!TTF_Init()) {
        fprintf(stderr, "Failed to initialize SDL3_TTF: %s\n", SDL_GetError());
        return 1;
    }

    const char *base_path = SDL_GetBasePath();
    const char *font_name = "HiburMono.ttf";
    size_t font_path_len = strlen(base_path) + strlen(font_name) + 1;

    char *font_path = malloc(font_path_len);
    if(font_path == NULL) { return 1; }

    snprintf(font_path, font_path_len, "%s%s", base_path, font_name);
    font = TTF_OpenFont(font_path, 15);

    if(font == NULL) {
        fprintf(stderr, "Failed to load the font: %s\n", SDL_GetError());
        free(font_path);
        return 1;
    }
    free(font_path);

    const int top_button_w = 120;
    int top_button_offset = 0;
    for(int i = 0; i < NUM_TOP_BUTTONS; i++) {
        ctx->gui.top_buttons[i] = (Button) {
            .x = MARGIN_LEFT + top_button_offset, .y = 3,
            .w = top_button_w, .h = 24,
            .clicked = false,
            .hovered = false
        };

        top_button_offset += top_button_w + 10;
    }

    ctx->gui.top_buttons[LOAD_IMAGE].text = "Load Image";
    ctx->gui.top_buttons[LOAD_IMAGE].on_click = load_image;
    ctx->gui.top_buttons[LOAD_IMAGE].data = image;

    ctx->gui.top_buttons[SAVE_IMAGE].text = "Save Image";
    ctx->gui.top_buttons[SAVE_IMAGE].on_click = save_image;
    ctx->gui.top_buttons[SAVE_IMAGE].data = image;

    ctx->gui.top_buttons[SAVE_IMAGE].text = "Save Image";
    ctx->gui.top_buttons[SAVE_IMAGE].on_click = save_image;
    ctx->gui.top_buttons[SAVE_IMAGE].data = image;

    ctx->gui.top_buttons[RESET].text = "Reset";
    ctx->gui.top_buttons[RESET].on_click = reset_efx;
    ctx->gui.top_buttons[RESET].data = &ctx->efx;

    ctx->gui.top_buttons[RANDOMIZE].text = "Randomize";
    ctx->gui.top_buttons[RANDOMIZE].on_click = randomize_efx;
    ctx->gui.top_buttons[RANDOMIZE].data = &ctx->efx;

    ctx->gui.top_buttons[EXIT].text = "Exit";
    ctx->gui.top_buttons[EXIT].on_click = exit_application;
    ctx->gui.top_buttons[EXIT].data = &ctx->state;

    return 0;
}

void cleanup_gui(void) {
    if(font != NULL) {
        TTF_CloseFont(font);
    }
    TTF_Quit();

    for(int i = 0; i <= num_cached_text; i++) {
        SDL_DestroyTexture(text_cache[i].texture);
    }
}

void *exit_application(void *data) {
    AppState *state = (AppState *)data;
    state->running = false;
    return NULL;
}

void *reset_efx(void *data) {
    (void)data;
    return NULL;
}

void *randomize_efx(void *data) {
    (void)data;
    return NULL;
}

void render_gui(AppContext *ctx, Image *image) {
    for(int i = 0; i < NUM_TOP_BUTTONS; i++) {
        draw_button(ctx->sdl.renderer, &ctx->gui.top_buttons[i]);
    }

    if(ctx->state.image_loaded) {
        int path_w, path_h;
        TTF_GetStringSize(font, image->path, 0, &path_w, &path_h);
        draw_text(ctx->sdl.renderer, ctx->sdl.win_width/2 - path_w/2, MARGIN_Y/2 - path_h*1.2, WHITE, image->path);
    }
}

void process_gui_events(SDL_Event *event, AppContext *ctx) {
    switch(event->type) {
        case SDL_EVENT_MOUSE_MOTION: {
            SDL_Point mouse_pos = {ctx->usr.mx, ctx->usr.my};
            
            for(int i = 0; i < NUM_TOP_BUTTONS; i++) {
                if(i == SAVE_IMAGE && !ctx->state.image_loaded) { continue; }

                SDL_Rect top_button_rect = {
                    .x = ctx->gui.top_buttons[i].x,
                    .y = ctx->gui.top_buttons[i].y,
                    .w = ctx->gui.top_buttons[i].w,
                    .h = ctx->gui.top_buttons[i].h,
                };
                
                if(SDL_PointInRect(&mouse_pos, &top_button_rect)) {
                    ctx->gui.top_buttons[i].hovered = true;
                } else { ctx->gui.top_buttons[i].hovered = false; }
            }
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            if(event->button.button == SDL_BUTTON_LEFT) {
                for(int i = 0; i < NUM_TOP_BUTTONS; i++) {
                    if(ctx->gui.top_buttons[i].hovered) {
                        ctx->gui.top_buttons[i].on_click(ctx->gui.top_buttons[i].data);
                    }
                }
            }
        }
        break;
    }
}

void draw_text(SDL_Renderer *renderer, int x, int y, SDL_Color font_color, char *text) {
    int w, h;
    SDL_Texture *texture = get_cached_text(renderer, &w, &h, font_color, text);
    SDL_FRect rect = {x, y, w, h};
    SDL_RenderTexture(renderer, texture, NULL, &rect);
}

SDL_Texture *get_cached_text(SDL_Renderer *renderer, int *w, int *h, SDL_Color font_color, char *text) {
    for(int i = 0; i < num_cached_text; i++) {
        if(strcmp(text, text_cache[i].text) == 0) {
            *w = text_cache[i].w;
            *h = text_cache[i].h;
            return text_cache[i].texture;
        }
    }

    SDL_Surface *surface = TTF_RenderText_Blended(font, text, 0, font_color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    int text_w = surface->w;
    int text_h = surface->h;

    SDL_DestroySurface(surface);

    if(num_cached_text < TEXT_CACHE_MAX) {
        CachedText *entry = &text_cache[num_cached_text++];
        strcpy(entry->text, text);
        entry->w = text_w;
        entry->h = text_h;
        entry->texture = texture;
    }

    return texture;
}

void draw_button(SDL_Renderer *renderer, Button *button) {
    SDL_FRect border_rect = {button->x, button->y, button->w, button->h};
    SDL_FRect button_rect = {button->x + 3, button->y + 3, button->w - 6, button->h - 6};

    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderFillRect(renderer, &border_rect);
    

    if(button->hovered) {
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    }

    SDL_RenderFillRect(renderer, &button_rect);

    int text_w, text_h;
    TTF_GetStringSize(font, button->text, 0, &text_w, &text_h);
    int text_x = (button->x + button->w/2) - (text_w/2);
    int text_y = (button->y + button->h/2) - (text_h/2) - 3;
    draw_text(renderer, text_x, text_y, WHITE, button->text);
}

void draw_slider(SDL_Renderer *renderer, Slider *slider) {
    float *slider_val = (float *)slider->data;

    SDL_FRect border_rect = {slider->x, slider->y, slider->w, slider->h};
    SDL_FRect bg_rect = {slider->x + 3, slider->y + 3, slider->w - 6, slider->h - 6};
    SDL_FRect slider_rect = {bg_rect.x, bg_rect.y, bg_rect.w * *slider_val, bg_rect.h};

    SDL_SetRenderDrawColor(renderer, 20, 20, 20, 255);
    SDL_RenderFillRect(renderer, &border_rect);

    SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    SDL_RenderFillRect(renderer, &bg_rect);

    SDL_SetRenderDrawColor(renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(renderer, &slider_rect);

    int text_w, text_h;
    TTF_GetStringSize(font, slider->text, 0, &text_w, &text_h);
    int text_x = (slider->x + slider->w/2) - (text_w/2);
    int text_y = (slider->y + slider->h/2) - (text_h/2) - 3;

    draw_text(renderer, text_x, text_y, WHITE, slider->text);
}
