#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>

#include "common.h"
#include "file_io.h"
#include "gui.h"

TTF_Font *font_15 = NULL;
bool debug_info = true;

typedef struct {
    char text[64];
    int w, h;
    SDL_Texture *texture;
} CachedText;

#define TEXT_CACHE_MAX 32
CachedText text_cache[TEXT_CACHE_MAX];
int num_cached_text = 0;

void *exit_application(void *data);
void *toggle_efx(void *data);
void *change_warp_mode(void *data);
void *change_mode_3(void *data);

void draw_button(SDL_Renderer *renderer, Button *button);
void draw_slider(SDL_Renderer *renderer, Slider *slider);
void draw_static_text(SDL_Renderer *renderer, int x, int y, SDL_Color font_color, char *text);
void draw_dynamic_text(SDL_Renderer *renderer, int x, int y, SDL_Color font_color, char *text);
SDL_Texture *get_cached_text(SDL_Renderer *renderer, int *w, int *h, SDL_Color font_color, char *text);
void draw_debug_info(AppContext *ctx, Image *image);

int init_gui(AppContext *ctx, Image *image) {
    if(!TTF_Init()) {
        fprintf(stderr, "Failed to initialize SDL3_TTF: %s\n", SDL_GetError());
        return 1;
    }

    const char *base_path = SDL_GetBasePath();
    const char *font_name = "Aldrich-Regular.ttf";
    size_t font_path_len = strlen(base_path) + strlen(font_name) + 1;

    char *font_path = malloc(font_path_len);
    if(font_path == NULL) { return 1; }

    snprintf(font_path, font_path_len, "%s%s", base_path, font_name);
    font_15 = TTF_OpenFont(font_path, 15);

    if(font_15 == NULL) {
        fprintf(stderr, "Failed to load the font: %s\n", SDL_GetError());
        free(font_path);
        return 1;
    }
    free(font_path);

    // Top bar buttons
    #define TOP_X (MARGIN_X / 2)
    #define TOP_Y 3
    #define TOP_W 100
    #define TOP_H 24
    #define TOP_X_OFFSET (TOP_W + 10)

    ctx->gui.buttons[LOAD_IMAGE] = (Button){ .button_type = GENERIC, .x = TOP_X, .y = TOP_Y, .w = TOP_W, .h = TOP_H, 
                                             .text = "Load Image", .on_click = load_image, .data = image};

    ctx->gui.buttons[SAVE_IMAGE] = (Button){ .button_type = GENERIC, .x = TOP_X + TOP_X_OFFSET, .y = TOP_Y, .w = TOP_W, .h = TOP_H,
                                             .text = "Save Image", .on_click = save_image, .data = image};

    ctx->gui.buttons[EXIT]       = (Button){ .button_type = GENERIC, .x = TOP_X + 2 * TOP_X_OFFSET, .y = TOP_Y, .w = TOP_W, .h = TOP_H,
                                             .text = "Quit", .on_click = exit_application, .data = &ctx->state.running};

    // Left side effect buttons
    #define LEFT_X 5
    #define LEFT_Y (MARGIN_Y / 2)
    #define LEFT_W ((MARGIN_X / 2) - 10)
    #define LEFT_H 40
    #define LEFT_Y_OFFSET (LEFT_H + 10)

    ctx->gui.buttons[SATURATION]  = (Button){ .button_type = EFX_BTN, .x = LEFT_X, .y = LEFT_Y, .w = LEFT_W, .h = LEFT_H,
                                              .text = "Saturation", .on_click = toggle_efx, .data = &ctx->efx.saturation};

    ctx->gui.buttons[CONTRAST]    = (Button){ .button_type = EFX_BTN, .x = LEFT_X, .y = LEFT_Y + LEFT_Y_OFFSET, .w = LEFT_W, .h = LEFT_H,
                                              .text = "Contrast", .on_click = toggle_efx, .data = &ctx->efx.contrast};

    ctx->gui.buttons[EXPOSURE]    = (Button){ .button_type = EFX_BTN, .x = LEFT_X, .y = LEFT_Y + 2 * LEFT_Y_OFFSET, .w = LEFT_W, .h = LEFT_H,
                                              .text = "Exposure", .on_click = toggle_efx, .data = &ctx->efx.exposure};

    ctx->gui.buttons[INVERT]      = (Button){ .button_type = EFX_BTN, .x = LEFT_X, .y = LEFT_Y + 3 * LEFT_Y_OFFSET, .w = LEFT_W, .h = LEFT_H,
                                              .text = "Invert", .on_click = toggle_efx, .data = &ctx->efx.invert};

    ctx->gui.buttons[MONO]        = (Button){ .button_type = EFX_BTN, .x = LEFT_X, .y = LEFT_Y + 4 * LEFT_Y_OFFSET, .w = LEFT_W, .h = LEFT_H,
                                              .text = "Monochrome", .on_click = toggle_efx, .data = &ctx->efx.mono};

    ctx->gui.buttons[THRESHOLD]   = (Button){ .button_type = EFX_BTN, .x = LEFT_X, .y = LEFT_Y + 5 * LEFT_Y_OFFSET, .w = LEFT_W, .h = LEFT_H,
                                              .text = "Threshold", .on_click = toggle_efx, .data = &ctx->efx.threshold};

    ctx->gui.buttons[QUANTIZE]    = (Button){ .button_type = EFX_BTN, .x = LEFT_X, .y = LEFT_Y + 6 * LEFT_Y_OFFSET, .w = LEFT_W, .h = LEFT_H,
                                              .text = "Quantize", .on_click = toggle_efx, .data = &ctx->efx.quantize};

    ctx->gui.buttons[COLOR_BIAS]  = (Button){ .button_type = EFX_BTN, .x = LEFT_X, .y = LEFT_Y + 7 * LEFT_Y_OFFSET, .w = LEFT_W, .h = LEFT_H,
                                              .text = "Color Bias", .on_click = toggle_efx, .data = &ctx->efx.color_bias};

    ctx->gui.buttons[COLOR_SHIFT] = (Button){ .button_type = EFX_BTN, .x = LEFT_X, .y = LEFT_Y + 8 * LEFT_Y_OFFSET, .w = LEFT_W, .h = LEFT_H,
                                              .text = "Color Shift", .on_click = toggle_efx, .data = &ctx->efx.color_shift};

    ctx->gui.buttons[WARP]        = (Button){ .button_type = EFX_BTN, .x = LEFT_X, .y = LEFT_Y + 9 * LEFT_Y_OFFSET, .w = LEFT_W, .h = LEFT_H,
                                              .text = "Warp", .on_click = toggle_efx, .data = &ctx->efx.warp};

    ctx->gui.buttons[PIXELATE]    = (Button){ .button_type = EFX_BTN, .x = LEFT_X, .y = LEFT_Y + 10 * LEFT_Y_OFFSET, .w = LEFT_W, .h = LEFT_H,
                                              .text = "Pixelate", .on_click = toggle_efx, .data = &ctx->efx.pixelate};

    ctx->gui.buttons[DITHER]      = (Button){ .button_type = EFX_BTN, .x = LEFT_X, .y = LEFT_Y + 11 * LEFT_Y_OFFSET, .w = LEFT_W, .h = LEFT_H,
                                              .text = "Dither", .on_click = toggle_efx, .data = &ctx->efx.dither};
    
    // Right side effect parameter sliders
    #define SLIDER_X (ctx->sdl.win_width - (MARGIN_X / 2) + 5)
    #define SLIDER_Y (MARGIN_Y / 2)
    #define SLIDER_W ((MARGIN_X / 2) - 10)
    #define SLIDER_H 30
    #define SLIDER_Y_OFFSET (SLIDER_H + 10) 

    ctx->gui.sliders[SATURATION_VAL] = (Slider){.x = SLIDER_X, .y = SLIDER_Y, .w = SLIDER_W, .h = SLIDER_H,
                                                .text = "Saturation", .data = &ctx->params.saturation_val};

    ctx->gui.sliders[CONTRAST_VAL] = (Slider){.x = SLIDER_X, .y = SLIDER_Y + SLIDER_Y_OFFSET, .w = SLIDER_W, .h = SLIDER_H,
                                              .text = "Contrast", .data = &ctx->params.contrast_val};

    ctx->gui.sliders[EXPOSURE_VAL] = (Slider){.x = SLIDER_X, .y = SLIDER_Y + 2 * SLIDER_Y_OFFSET, .w = SLIDER_W, .h = SLIDER_H,
                                              .text = "Exposure", .data = &ctx->params.exposure_val};

    ctx->gui.sliders[THRESHOLD_VAL] = (Slider){.x = SLIDER_X, .y = SLIDER_Y + 3 * SLIDER_Y_OFFSET, .w = SLIDER_W, .h = SLIDER_H,
                                               .text = "Threshold", .data = &ctx->params.threshold_val};

    ctx->gui.sliders[BIT_DEPTH] = (Slider){.x = SLIDER_X, .y = SLIDER_Y + 4 * SLIDER_Y_OFFSET, .w = SLIDER_W, .h = SLIDER_H,
                                           .text = "Bit depth", .data = &ctx->params.bit_depth};

    ctx->gui.sliders[COLOR_SHIFT_VAL] = (Slider){.x = SLIDER_X, .y = SLIDER_Y + 5 * SLIDER_Y_OFFSET, .w = SLIDER_W, .h = SLIDER_H,
                                                 .text = "Color Shift", .data = &ctx->params.color_shift_val};

    ctx->gui.sliders[PIXEL_SIZE] = (Slider){.x = SLIDER_X, .y = SLIDER_Y + 6 * SLIDER_Y_OFFSET, .w = SLIDER_W, .h = SLIDER_H,
                                            .text = "Pixel size", .data = &ctx->params.pixel_size};

    ctx->gui.sliders[DITHER_BRIGHTNESS] = (Slider){.x = SLIDER_X, .y = SLIDER_Y + 7 * SLIDER_Y_OFFSET, .w = SLIDER_W, .h = SLIDER_H,
                                                   .text = "Dither brightness", .data = &ctx->params.dither_brightness};

    ctx->gui.sliders[SINE_LENGTH] = (Slider){.x = SLIDER_X, .y = SLIDER_Y + 8 * SLIDER_Y_OFFSET, .w = SLIDER_W, .h = SLIDER_H,
                                             .text = "Sine length", .data = &ctx->params.sine_length};

    ctx->gui.sliders[SINE_AMP] = (Slider){.x = SLIDER_X, .y = SLIDER_Y + 9 * SLIDER_Y_OFFSET, .w = SLIDER_W, .h = SLIDER_H,
                                          .text = "Sine amplitude", .data = &ctx->params.sine_amp};

    // Mode change buttons
    ctx->gui.buttons[WARP_MODE] = (Button){.button_type = GENERIC, .x = SLIDER_X, .y = SLIDER_Y + 10 * SLIDER_Y_OFFSET, .w = SLIDER_W, .h = LEFT_H,
                                           .text = "Warp mode", .on_click = change_warp_mode, .data = &ctx->params.warp_mode};

    ctx->gui.buttons[THRESHOLD_MODE] = (Button){.button_type = GENERIC, .x = SLIDER_X, .y = SLIDER_Y + 11 * SLIDER_Y_OFFSET + 10,
                                                .w = SLIDER_W, .h = LEFT_H,
                                                .text = "Threshold mode", .on_click = change_mode_3, .data = &ctx->params.threshold_mode};

    ctx->gui.buttons[COLOR_BIAS_MODE] = (Button){.button_type = GENERIC, .x = SLIDER_X, .y = SLIDER_Y + 12 * SLIDER_Y_OFFSET + 20, 
                                                 .w = SLIDER_W, .h = LEFT_H,
                                                 .text = "Color bias mode", .on_click = change_mode_3, .data = &ctx->params.color_bias};

    ctx->gui.buttons[DITHER_MODE] = (Button){.button_type = GENERIC, .x = SLIDER_X, .y = SLIDER_Y + 13 * SLIDER_Y_OFFSET + 30, 
                                                 .w = SLIDER_W, .h = LEFT_H,
                                                 .text = "Dither mode", .on_click = change_mode_3, .data = &ctx->params.dither_mode};

    return 0;
}

void cleanup_gui(void) {
    if(font_15 != NULL) {
        TTF_CloseFont(font_15);
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

void *toggle_efx(void *data) {
    bool *flag = (bool *)data;
    *flag = !*flag;
    return NULL;
}

void *change_warp_mode(void *data) {
    int *mode = (int *)data;
    if(*mode < 3) {
        *mode += 1;
    } else {
        *mode = 0;
    }
    return NULL;
}

void *change_mode_3(void *data) {
    int *mode = (int *)data;
    if(*mode < 2) {
        *mode += 1;
    } else {
        *mode = 0;
    }
    return NULL;
}

void update_gui(AppContext *ctx) {
    SDL_GetWindowSize(ctx->sdl.window, &ctx->sdl.win_width, &ctx->sdl.win_height);

    // Left side effect buttons
    // i is 7 to skip the buttons that arent the right side effect buttons
    for(int i = 7; i < NUM_BUTTONS; i++) {
        ctx->gui.buttons[i].x = LEFT_X;
    }

    ctx->gui.buttons[WARP_MODE].x = SLIDER_X;
    ctx->gui.buttons[THRESHOLD_MODE].x = SLIDER_X;
    ctx->gui.buttons[COLOR_BIAS_MODE].x = SLIDER_X;
    ctx->gui.buttons[DITHER_MODE].x = SLIDER_X;

    //Right side effect parameter sliders
    for(int i = 0; i < NUM_SLIDERS; i++) {
        ctx->gui.sliders[i].x = SLIDER_X;
    }
}

void render_gui(AppContext *ctx, Image *image) {
    for(int i = 0; i < NUM_BUTTONS; i++) {
        draw_button(ctx->sdl.renderer, &ctx->gui.buttons[i]);
    }
    
    for(int i = 0; i < NUM_SLIDERS; i++) {
        draw_slider(ctx->sdl.renderer, &ctx->gui.sliders[i]);
    }
    
    if(debug_info) { draw_debug_info(ctx, image); }
}

void process_gui_events(SDL_Event *event, AppContext *ctx) {
    switch(event->type) {
        case SDL_EVENT_MOUSE_MOTION: {
            SDL_Point mouse_pos = {ctx->usr.mx, ctx->usr.my};

            for(int i = 0; i < NUM_BUTTONS; i++) {
                if(i == SAVE_IMAGE && !ctx->state.image_loaded) { continue; }

                SDL_Rect button_rect = {
                    ctx->gui.buttons[i].x,
                    ctx->gui.buttons[i].y,
                    ctx->gui.buttons[i].w,
                    ctx->gui.buttons[i].h
                };

                if(SDL_PointInRect(&mouse_pos, &button_rect)) {
                    ctx->gui.buttons[i].hovered = true;
                } 
                else {
                    ctx->gui.buttons[i].hovered = false;
                }
            }

            for(int i = 0; i < NUM_SLIDERS; i++) {
                SDL_Rect slider_rect = {
                    ctx->gui.sliders[i].x,
                    ctx->gui.sliders[i].y,
                    ctx->gui.sliders[i].w,
                    ctx->gui.sliders[i].h
                };

                if(SDL_PointInRect(&mouse_pos, &slider_rect)) {
                    ctx->gui.sliders[i].hovered = true;
                } else {
                    ctx->gui.sliders[i].hovered = false;
                }

                if(ctx->gui.sliders[i].sliding) {
                    float *slider_val = (float *)ctx->gui.sliders[i].data;
                    *slider_val += event->motion.xrel * 0.005;
                    *slider_val = clampf(*slider_val, 0.0f, 1.0f);

                    ctx->state.needs_update = true;
                }
            }

            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if(event->button.button == SDL_BUTTON_LEFT) {
                for(int i = 0; i < NUM_BUTTONS; i++) {
                    if(ctx->gui.buttons[i].hovered) {
                        ctx->gui.buttons[i].clicked = true;
                        ctx->state.needs_update = true;
                        ctx->gui.buttons[i].on_click(ctx->gui.buttons[i].data);
                    }
                }

                for(int i = 0; i < NUM_SLIDERS; i++) {
                    if(ctx->gui.sliders[i].hovered) {
                        ctx->gui.sliders[i].sliding = true;
                    }
                }
            }
            break;
        case SDL_EVENT_MOUSE_BUTTON_UP:
            if(event->button.button == SDL_BUTTON_LEFT) {
                for(int i = 0; i < NUM_SLIDERS; i++) {
                    if(ctx->gui.sliders[i].sliding) {
                        ctx->gui.sliders[i].sliding = false;
                    }
                }
            }
            break;
    }
}

void draw_static_text(SDL_Renderer *renderer, int x, int y, SDL_Color font_color, char *text) {
    int w, h;
    SDL_Texture *texture = get_cached_text(renderer, &w, &h, font_color, text);
    SDL_FRect rect = {x, y, w, h};
    SDL_RenderTexture(renderer, texture, NULL, &rect);
}

void draw_dynamic_text(SDL_Renderer *renderer, int x, int y, SDL_Color font_color, char *text) {
    SDL_Surface *surface = TTF_RenderText_Blended(font_15, text, 0, font_color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);

    int text_w = surface->w;
    int text_h = surface->h;

    SDL_FRect rect = {x, y, text_w, text_h};
    SDL_RenderTexture(renderer, texture, NULL, &rect);

    SDL_DestroySurface(surface);
    SDL_DestroyTexture(texture);
}

SDL_Texture *get_cached_text(SDL_Renderer *renderer, int *w, int *h, SDL_Color font_color, char *text) {
    for(int i = 0; i < num_cached_text; i++) {
        if(strcmp(text, text_cache[i].text) == 0) {
            *w = text_cache[i].w;
            *h = text_cache[i].h;
            return text_cache[i].texture;
        }
    }

    SDL_Surface *surface = TTF_RenderText_Blended(font_15, text, 0, font_color);
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
    
    if(button->button_type == EFX_BTN && *(bool *)button->data) {
        if(button->hovered) {
            SDL_SetRenderDrawColor(renderer, 0, 120, 0, 255);
        }
        else {
            SDL_SetRenderDrawColor(renderer, 0, 80, 0, 255);
        }
    }
    else if(button->hovered) {
        SDL_SetRenderDrawColor(renderer, 80, 80, 80, 255);
    }
    else {
        SDL_SetRenderDrawColor(renderer, 60, 60, 60, 255);
    }

    SDL_RenderFillRect(renderer, &button_rect);

    int text_w, text_h;
    TTF_GetStringSize(font_15, button->text, 0, &text_w, &text_h);
    draw_static_text(renderer, (button->x + button->w / 2) - (text_w / 2), (button->y + button->h / 2) - (text_h / 2), WHITE, button->text);
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
    TTF_GetStringSize(font_15, slider->text, 0, &text_w, &text_h);
    draw_static_text(renderer, (slider->x + slider->w / 2) - (text_w / 2), (slider->y + slider->h / 2) - (text_h / 2), WHITE, slider->text);
}


void draw_debug_info(AppContext *ctx, Image *image) {
    char buffer[32];
    int x = (MARGIN_X / 2) + 10;
    int y = 50;
    int y_offset = 30;

    #define DRAW_DEBUG_INFO(fmt_str, value)                                \
            do {                                                           \
                snprintf(buffer, 32, fmt_str, value);                      \
                draw_dynamic_text(ctx->sdl.renderer, x, y, WHITE, buffer); \
                y += y_offset;                                             \
            } while(0)                                                     \

    DRAW_DEBUG_INFO("Scale: %.2f", ctx->usr.scale);

    DRAW_DEBUG_INFO("Original Width: %d",  image->width);
    DRAW_DEBUG_INFO("Original Height: %d", image->height);

    DRAW_DEBUG_INFO("Current Width: %.0f", image->texture_rect.w);
    DRAW_DEBUG_INFO("Current Height: %.0f", image->texture_rect.h);

    DRAW_DEBUG_INFO("X Position: %.0f", image->texture_rect.x);
    DRAW_DEBUG_INFO("Y Position: %.0f", image->texture_rect.y);

    DRAW_DEBUG_INFO("Mouse X: %.0f", ctx->usr.mx);
    DRAW_DEBUG_INFO("Mouse Y: %.0f", ctx->usr.my);
}
