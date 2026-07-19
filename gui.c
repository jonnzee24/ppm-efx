#include <stdio.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>

#include "common.h"
#include "file_io.h"
#include "gui.h"

TTF_Font *font_15 = NULL;

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
void *reset_efx(void *data);
void *randomize_efx(void *data);
void *change_warp_mode(void *data);
void *change_mode_3(void *data);

void draw_button(SDL_Renderer *renderer, Button *button);
void draw_slider(SDL_Renderer *renderer, Slider *slider);
void draw_static_text(SDL_Renderer *renderer, int x, int y, SDL_Color font_color, char *text);
void draw_dynamic_text(SDL_Renderer *renderer, int x, int y, SDL_Color font_color, char *text);
SDL_Texture *get_cached_text(SDL_Renderer *renderer, int *w, int *h, SDL_Color font_color, char *text);

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

    ctx->gui.buttons[RESET] = (Button){ .button_type = GENERIC, .x = TOP_X + TOP_X_OFFSET, .y = TOP_Y, .w = TOP_W, .h = TOP_H,
                                             .text = "Reset", .on_click = reset_efx, .data = ctx};

    ctx->gui.buttons[RANDOMIZE] = (Button){ .button_type = GENERIC, .x = TOP_X + TOP_X_OFFSET * 2, .y = TOP_Y, .w = TOP_W, .h = TOP_H,
                                             .text = "Randomize", .on_click = randomize_efx, .data = ctx};

    ctx->gui.buttons[SAVE_IMAGE] = (Button){ .button_type = GENERIC, .x = ctx->sdl.win_width - TOP_W - TOP_X_OFFSET - MARGIN_X/2,
                                            .y = TOP_Y, .w = TOP_W, .h = TOP_H,
                                             .text = "Save Image", .on_click = save_image, .data = image};

    ctx->gui.buttons[EXIT]       = (Button){ .button_type = GENERIC, .x = ctx->sdl.win_width - TOP_W - MARGIN_X/2, .y = TOP_Y, .w = TOP_W, .h = TOP_H,
                                             .text = "Quit", .on_click = exit_application, .data = &ctx->state.running};

    // Left side effect buttons
    int y_offset = 0;

    #define INIT_EFX_BTN(id, string, efxm)                                       \
            do {                                                                 \
                ctx->gui.buttons[id] = (Button){ .button_type = EFX_BTN,         \
                                                 .x = 5,                         \
                                                 .y = (MARGIN_Y / 2) + y_offset, \
                                                 .w = (MARGIN_X / 2) -  10,      \
                                                 .h = 40,                        \
                                                 .text = string,                 \
                                                 .on_click = toggle_efx,         \
                                                 .data = efxm};                  \
                y_offset += 50;                                                  \
            } while(0)                                                           \
    
    INIT_EFX_BTN(SATURATION, "Saturation", &ctx->efx.saturation);
    INIT_EFX_BTN(CONTRAST, "Contrast", &ctx->efx.contrast);
    INIT_EFX_BTN(EXPOSURE, "Exposure", &ctx->efx.exposure);
    INIT_EFX_BTN(INVERT, "Invert", &ctx->efx.invert);
    INIT_EFX_BTN(MONO, "Monochrome", &ctx->efx.mono);
    INIT_EFX_BTN(THRESHOLD, "Threshold", &ctx->efx.threshold);
    INIT_EFX_BTN(QUANTIZE, "Quantize", &ctx->efx.quantize);
    INIT_EFX_BTN(COLOR_BIAS, "Color Bias", &ctx->efx.color_bias);
    INIT_EFX_BTN(COLOR_SHIFT, "Color Shift", &ctx->efx.color_shift);
    INIT_EFX_BTN(WARP, "Warp", &ctx->efx.warp);
    INIT_EFX_BTN(PIXELATE, "Pixelate", &ctx->efx.pixelate);
    INIT_EFX_BTN(DITHER, "Dither", &ctx->efx.dither);
    INIT_EFX_BTN(BLUR, "Blur", &ctx->efx.blur);
    
    #undef INIT_EFX_BTN
    
    // Right side effect parameter sliders
    #define SLIDER_X (ctx->sdl.win_width - (MARGIN_X / 2) + 5)

    #define INIT_EFX_SLIDER(id, string, parameter)                               \
            do {                                                                 \
                ctx->gui.sliders[id] = (Slider){.x = SLIDER_X,                   \
                                                .y = (MARGIN_Y / 2) + y_offset,  \
                                                .w = (MARGIN_X / 2) - 10,        \
                                                .h = 30,                         \
                                                .text = string,                  \
                                                .data = &ctx->params.parameter}; \
                y_offset += 40;                                                  \
            } while(0)                                                           \
    
    y_offset = 0;

    INIT_EFX_SLIDER(SATURATION_VAL, "Saturation", saturation_val);
    INIT_EFX_SLIDER(CONTRAST_VAL, "Contrast", contrast_val);
    INIT_EFX_SLIDER(EXPOSURE_VAL, "Exposure", exposure_val);
    INIT_EFX_SLIDER(INVERT_X, "Invert Split", invert_x);
    INIT_EFX_SLIDER(THRESHOLD_VAL, "Threshold", threshold_val);
    INIT_EFX_SLIDER(BIT_DEPTH, "Bit Depth", bit_depth);
    INIT_EFX_SLIDER(COLOR_SHIFT_VAL, "Color Shift", color_shift_val);
    INIT_EFX_SLIDER(PIXEL_SIZE, "Pixel Size", pixel_size);
    INIT_EFX_SLIDER(SINE_LENGTH, "Sine Lenght", sine_length);
    INIT_EFX_SLIDER(SINE_AMP, "Sine Amplitude", sine_amp);
    INIT_EFX_SLIDER(BLUR_SIZE, "Blur Size", blur_size);

    #undef INIT_EFX_SLIDER

    // Mode change buttons
    #define INIT_MODE_BTN(id, string, callback, parameter)                        \
            do {                                                                  \
                ctx->gui.buttons[id] = (Button){ .button_type = GENERIC,          \
                                                 .x = SLIDER_X,                   \
                                                 .y = (MARGIN_Y / 2) + y_offset,  \
                                                 .w = (MARGIN_X / 2) -  10,       \
                                                 .h = 40,                         \
                                                 .text = string,                  \
                                                 .on_click = callback,            \
                                                 .data = &ctx->params.parameter}; \
                y_offset += 50;                                                   \
            } while(0)                                                            \

    INIT_MODE_BTN(WARP_MODE, "Warp Mode", change_warp_mode, warp_mode);
    INIT_MODE_BTN(THRESHOLD_MODE, "Threshold Mode", change_mode_3, threshold_mode);
    INIT_MODE_BTN(COLOR_BIAS_MODE, "Color Bias Mode", change_mode_3, color_bias);
    INIT_MODE_BTN(DITHER_MODE, "Dither Mode", change_mode_3, dither_mode);

    #undef INIT_MODE_BTN

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

void *reset_efx(void *data) {
    AppContext *ctx = (AppContext *)data;
    ctx->efx = (EffectFlags){0};
    ctx->params.warp_mode = MIRROR_X;
    ctx->params.dither_mode = 0;
    ctx->params.color_bias = R;
    ctx->params.threshold_mode = 0;
    ctx->params.sine_length = 0.5f;
    ctx->params.sine_amp = 0.5f;
    ctx->params.pixel_size = 0.5f;
    ctx->params.threshold_val = 0.5f;
    ctx->params.bit_depth = 0.5f;
    ctx->params.exposure_val = 0.5f;
    ctx->params.contrast_val = 0.5f;
    ctx->params.saturation_val = 0.5f;
    ctx->params.invert_x = 0.5f;
    ctx->params.color_shift_val = 0.5f;
    ctx->params.blur_size = 0.5f;
    return NULL;
}

void *randomize_efx(void *data) {
    AppContext *ctx = (AppContext *)data;
    size_t num_efx = sizeof(ctx->efx)/sizeof(ctx->efx.warp);
    bool *efx_ptr = (bool *)&ctx->efx;
    for(size_t i = 0; i < num_efx; i++) {
        efx_ptr[i] = (rand() % 2);
    }
    #define RAND_FLOAT (float)rand()/(float)RAND_MAX

    ctx->params.warp_mode = rand() % 4;
    ctx->params.threshold_mode = rand() % 3;
    ctx->params.dither_mode = rand() % 3;
    ctx->params.color_bias = rand() % 3;

    ctx->params.sine_length = RAND_FLOAT; 
    ctx->params.sine_amp = RAND_FLOAT;
    ctx->params.pixel_size = RAND_FLOAT;
    ctx->params.threshold_val = RAND_FLOAT;
    ctx->params.bit_depth = RAND_FLOAT;
    ctx->params.exposure_val = RAND_FLOAT;
    ctx->params.contrast_val = RAND_FLOAT;
    ctx->params.saturation_val = RAND_FLOAT;
    ctx->params.invert_x = RAND_FLOAT;
    ctx->params.color_shift_val = RAND_FLOAT;
    ctx->params.blur_size = RAND_FLOAT;

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

    ctx->gui.buttons[SAVE_IMAGE].x = ctx->sdl.win_width - TOP_W - TOP_X_OFFSET - MARGIN_X/2;
    ctx->gui.buttons[EXIT].x = ctx->sdl.win_width - MARGIN_X/2 - TOP_W;

    ctx->gui.buttons[WARP_MODE].x = SLIDER_X;
    ctx->gui.buttons[THRESHOLD_MODE].x = SLIDER_X;
    ctx->gui.buttons[COLOR_BIAS_MODE].x = SLIDER_X;
    ctx->gui.buttons[DITHER_MODE].x = SLIDER_X;

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

    if(ctx->state.image_loaded) {
        int path_w, path_h;
        TTF_GetStringSize(font_15, image->path, 0, &path_w, &path_h);
        draw_dynamic_text(ctx->sdl.renderer, ctx->sdl.win_width/2 - path_w/2, MARGIN_Y/4 - path_h/2, WHITE, image->path);
    }
}

void process_gui_events(SDL_Event *event, AppContext *ctx, Image *image) {
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

                    image->needs_update = true;
                }
            }

            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            if(event->button.button == SDL_BUTTON_LEFT) {
                for(int i = 0; i < NUM_BUTTONS; i++) {
                    if(ctx->gui.buttons[i].hovered) {
                        ctx->gui.buttons[i].clicked = true;
                        ctx->gui.buttons[i].on_click(ctx->gui.buttons[i].data);

                        image->needs_update = true;
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
    int y = (MARGIN_Y / 2) + 10;
    int y_offset = 30;

    #define DRAW_DEBUG_INFO(fmt_str, value)                                \
            do {                                                           \
                snprintf(buffer, 32, fmt_str, value);                      \
                draw_dynamic_text(ctx->sdl.renderer, x, y, WHITE, buffer); \
                y += y_offset;                                             \
            } while(0)                                                     \


    DRAW_DEBUG_INFO("DEBUG INFO%s", "");

    DRAW_DEBUG_INFO("Scale: %.2f", ctx->usr.scale);

    DRAW_DEBUG_INFO("Original Width: %d",  image->width);
    DRAW_DEBUG_INFO("Original Height: %d", image->height);

    DRAW_DEBUG_INFO("Image X: %.0f", image->texture_rect.x);
    DRAW_DEBUG_INFO("Image Y: %.0f", image->texture_rect.y);
    DRAW_DEBUG_INFO("Image W: %.0f", image->texture_rect.w);
    DRAW_DEBUG_INFO("Image H: %.0f", image->texture_rect.h);

    DRAW_DEBUG_INFO("Image VP X: %d", ctx->sdl.image_vp.x);
    DRAW_DEBUG_INFO("Image VP Y: %d", ctx->sdl.image_vp.y);
    DRAW_DEBUG_INFO("Image VP W: %d", ctx->sdl.image_vp.w);
    DRAW_DEBUG_INFO("Image VP H: %d", ctx->sdl.image_vp.h);

    DRAW_DEBUG_INFO("Mouse X: %0.f", ctx->usr.mx);
    DRAW_DEBUG_INFO("Mouse Y: %0.f", ctx->usr.my);
}
