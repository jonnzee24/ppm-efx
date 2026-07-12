#include <SDL_ttf.h>

#include "common.h"
#include "file_io.h"
#include "gui.h"

static TTF_Font *font_15 = NULL;

int init_gui(AppContext *ctx, Image *image) {
    if(TTF_Init() != 0) {
        fprintf(stderr, "Failed to initialize SDL2_TTF%s\n", SDL_GetError());
        return 1;
    }

    font_15 = TTF_OpenFont("Aldrich-Regular.ttf", 15);
    if(font_15 == NULL) {
        fprintf(stderr, "Failed to open TTF font%s\n", SDL_GetError());
    }

    // Top bar buttons
    ctx->gui.buttons[LOAD_IMAGE] = (Button){ .button_type = TOP_BAR, .x = 0, .y = 0, .w = 100, .h = 30, 
                                             .text = "Load Image", .on_click = load_image, .data = image};

    ctx->gui.buttons[SAVE_IMAGE] = (Button){ .button_type = TOP_BAR, .x = 100, .y = 0, .w = 100, .h = 30,
                                             .text = "Save Image", .on_click = button_dummy, .data = "button inactive"};

    ctx->gui.buttons[EXIT]       = (Button){ .button_type = TOP_BAR, .x = 200, .y = 0, .w = 100, .h = 30,
                                             .text = "Exit", .on_click = exit_application, .data = &ctx->state.running};

    // Right side effect buttons
    int right_x = (ctx->sdl.win_width - MARGIN_X / 2) + 5;
    int right_y = MARGIN_Y / 2;
    int right_w = (MARGIN_X / 2) - 10;
    int right_h = 40;
    int y_offset = 50;

    ctx->gui.buttons[SATURATION]  = (Button){ .button_type = EFX_BTN, .x = right_x, .y = right_y, .w = right_w, .h = right_h,
                                              .text = "Saturation", .on_click = toggle_efx, .data = &ctx->efx.saturation};

    ctx->gui.buttons[CONTRAST]    = (Button){ .button_type = EFX_BTN, .x = right_x, .y = right_y + y_offset, .w = right_w, .h = right_h,
                                              .text = "Contrast", .on_click = toggle_efx, .data = &ctx->efx.contrast};

    ctx->gui.buttons[EXPOSURE]    = (Button){ .button_type = EFX_BTN, .x = right_x, .y = right_y + 2 * y_offset, .w = right_w, .h = right_h,
                                              .text = "Exposure", .on_click = toggle_efx, .data = &ctx->efx.exposure};

    ctx->gui.buttons[INVERT]      = (Button){ .button_type = EFX_BTN, .x = right_x, .y = right_y + 3 * y_offset, .w = right_w, .h = right_h,
                                              .text = "Invert", .on_click = toggle_efx, .data = &ctx->efx.invert};

    ctx->gui.buttons[MONO]        = (Button){ .button_type = EFX_BTN, .x = right_x, .y = right_y + 4 * y_offset, .w = right_w, .h = right_h,
                                              .text = "Monochrome", .on_click = toggle_efx, .data = &ctx->efx.mono};

    ctx->gui.buttons[QUANTIZE]    = (Button){ .button_type = EFX_BTN, .x = right_x, .y = right_y + 5 * y_offset, .w = right_w, .h = right_h,
                                              .text = "Quantize", .on_click = toggle_efx, .data = &ctx->efx.quantize};

    ctx->gui.buttons[COLOR_BIAS]  = (Button){ .button_type = EFX_BTN, .x = right_x, .y = right_y + 6 * y_offset, .w = right_w, .h = right_h,
                                              .text = "Color Bias", .on_click = toggle_efx, .data = &ctx->efx.color_bias};

    ctx->gui.buttons[COLOR_SHIFT] = (Button){ .button_type = EFX_BTN, .x = right_x, .y = right_y + 7 * y_offset, .w = right_w, .h = right_h,
                                              .text = "Color Shift", .on_click = toggle_efx, .data = &ctx->efx.color_shift};

    ctx->gui.buttons[WARP]        = (Button){ .button_type = EFX_BTN, .x = right_x, .y = right_y + 8 * y_offset, .w = right_w, .h = right_h,
                                              .text = "Warp", .on_click = toggle_efx, .data = &ctx->efx.warp};

    ctx->gui.buttons[PIXELATE]    = (Button){ .button_type = EFX_BTN, .x = right_x, .y = right_y + 9 * y_offset, .w = right_w, .h = right_h,
                                              .text = "Pixelate", .on_click = toggle_efx, .data = &ctx->efx.pixelate};

    ctx->gui.buttons[DITHER]      = (Button){ .button_type = EFX_BTN, .x = right_x, .y = right_y + 10 * y_offset, .w = right_w, .h = right_h,
                                              .text = "Dither", .on_click = toggle_efx, .data = &ctx->efx.dither};

    return 0;
}

void cleanup_gui(void) {
    if(font_15 != NULL) {
        TTF_CloseFont(font_15);
    }
    TTF_Quit();
}

void update_gui(AppContext *ctx) {
    SDL_GetWindowSize(ctx->sdl.window, &ctx->sdl.win_width, &ctx->sdl.win_height);

    // Right side effect buttons
    int right_x = (ctx->sdl.win_width - MARGIN_X / 2) + 5;

    // i is 3 to skip the buttons that arent the right side effect buttons
    for(int i = 3; i < NUM_BUTTONS; i++) {
        ctx->gui.buttons[i].x = right_x;
    }
}

void *button_dummy(void *data) {
    char *text = (char *)data;
    printf("%s\n", text);
    return NULL;
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

void render_gui(AppContext *ctx, Image *image) {
    for(int i = 0; i < NUM_BUTTONS; i++) {
        draw_button(ctx->sdl.renderer, &ctx->gui.buttons[i]);
    }
    draw_debug_info(ctx, image);
}

void process_gui_events(SDL_Event *event, AppContext *ctx) {
    switch(event->type) {
        case SDL_MOUSEMOTION: {
            SDL_Point mouse_pos = {ctx->usr.mx, ctx->usr.my};

            for(int i = 0; i < NUM_BUTTONS; i++) {
                SDL_Rect button_rect = {
                    ctx->gui.buttons[i].x,
                    ctx->gui.buttons[i].y,
                    ctx->gui.buttons[i].w,
                    ctx->gui.buttons[i].h
                };

                if(SDL_PointInRect(&mouse_pos, &button_rect)) {
                    ctx->gui.buttons[i].hovered = true;
                } else {
                    ctx->gui.buttons[i].hovered = false;
                }
            }
            break;
        }

        case SDL_MOUSEBUTTONDOWN:
            if(event->button.button == SDL_BUTTON_LEFT) {
                for(int i = 0; i < NUM_BUTTONS; i++) {
                    if(ctx->gui.buttons[i].hovered) {
                        ctx->gui.buttons[i].clicked = true;
                        ctx->state.needs_update = true;
                        ctx->gui.buttons[i].on_click(ctx->gui.buttons[i].data);
                    }
                }
            }
            break;
    }
}

void draw_button(SDL_Renderer *renderer, Button *button) {
    SDL_Rect border_rect = {button->x, button->y, button->w, button->h};
    SDL_Rect button_rect = {button->x + 3, button->y + 3, button->w - 6, button->h - 6};

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
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
    TTF_SizeText(font_15, button->text, &text_w, &text_h);
    draw_text(renderer, (button->x + button->w / 2) - (text_w / 2), (button->y + button->h / 2) - (text_h / 2), WHITE, button->text);
}

void draw_text(SDL_Renderer *renderer, int x_pos, int y_pos,
               SDL_Color font_color, char *text) {

    int text_width = 0;
    int text_height = 0;

    SDL_Surface *surface = TTF_RenderText_Blended(font_15, text, font_color);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    
    TTF_SizeText(font_15, text, &text_width, &text_height);
    SDL_Rect rect = {x_pos, y_pos, text_width, text_height};

    SDL_RenderCopy(renderer, texture, NULL, &rect);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void draw_debug_info(AppContext *ctx, Image *image) {
    char buffer[32];
    int x = 10;
    int y = 50;
    int y_offset = 30;

    #define DRAW_DEBUG_INFO(fmt_str, value)                        \
            do {                                                   \
                snprintf(buffer, 32, fmt_str, value);              \
                draw_text(ctx->sdl.renderer, x, y, WHITE, buffer); \
                y += y_offset;                                     \
            } while(0)                                             \

    DRAW_DEBUG_INFO("Scale: %.2f", ctx->usr.scale);

    DRAW_DEBUG_INFO("Original Width: %d",  image->width);
    DRAW_DEBUG_INFO("Original Height: %d", image->height);

    DRAW_DEBUG_INFO("Current Width: %d", ctx->sdl.texture_rect.w);
    DRAW_DEBUG_INFO("Current Height: %d", ctx->sdl.texture_rect.h);

    DRAW_DEBUG_INFO("X Position: %d", ctx->sdl.texture_rect.x);
    DRAW_DEBUG_INFO("Y Position: %d", ctx->sdl.texture_rect.y);

    DRAW_DEBUG_INFO("User X Offset: %d", ctx->usr.x_offset);
    DRAW_DEBUG_INFO("User Y Offset: %d", ctx->usr.y_offset);

    DRAW_DEBUG_INFO("Mouse X: %d", ctx->usr.mx);
    DRAW_DEBUG_INFO("Mouse Y: %d", ctx->usr.my);
}
