#include <stdio.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdlib.h>

#include "common.h"
#include "gui.h"

TTF_Font *font = NULL;

typedef struct {
    char text[256];
    int w, h;
    SDL_Texture *texture;
    SDL_Renderer *renderer;
} CachedText;

#define TEXT_CACHE_MAX 64
CachedText text_cache[TEXT_CACHE_MAX];
int num_cached_text = 0;

void *exit_application(void *data);
void *reset_efx(void *data);
void *remove_efx(void *data);
void *open_add_efx_popup(void *data);
void *add_efx(void *data);
void *randomize_efx(void *data);

void draw_button(SDL_Renderer *renderer, Button *button);
void draw_slider(SDL_Renderer *renderer, Slider *slider);
void draw_text(SDL_Renderer *renderer, int x, int y, SDL_Color font_color, char *text);
void update_efx_cards(AppContext *ctx);
void draw_efx_card(SDL_Renderer *renderer, EfxCard *card);
SDL_Texture *get_cached_text(SDL_Renderer *renderer, int *w, int *h, SDL_Color font_color, char *text);
void create_add_efx_popup(AppContext *ctx);
void process_gui_events(SDL_Event *event, AppContext *ctx);
void process_popup_events(SDL_Event *event, AppContext *ctx);

int init_gui(AppContext *ctx, Image *image) {
    if(!TTF_Init()) {
        fprintf(stderr, "Failed to initialize SDL3_TTF: %s\n", SDL_GetError());
        return 1;
    }

    const char *base_path = SDL_GetBasePath();
    const char *font_name = "font.ttf";
    size_t font_path_len = strlen(base_path) + strlen(font_name) + 1;

    char *font_path = malloc(font_path_len);
    if(font_path == NULL) { return 1; }

    snprintf(font_path, font_path_len, "%s%s", base_path, font_name);
    font = TTF_OpenFont(font_path, 15);

    if(font == NULL) {
        free(font_path);
        return 1;
    }
    free(font_path);

    const int top_button_w = 120;
    int top_button_offset = 0;
    for(int i = 0; i < NUM_TOP_BUTTONS; i++) {
        ctx->gui.top_buttons[i] = (Button) {
            .x = MARGIN_LEFT + top_button_offset, .y = 3,
            .w = top_button_w, .h = 24
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

    for(int i = 0; i < NUM_EFX; i++) {
        EfxCard *card = &ctx->gui.efx_cards[i];

        card->pipeline = &ctx->efx;

        card->x = ctx->sdl.win_width - MARGIN_RIGHT + 5;
        card->w = MARGIN_RIGHT - 10;

        card->x_button.x = card->x + card->w - 25;
        card->x_button.w = 20;
        card->x_button.h = 20;
        card->x_button.text = "X";
        card->x_button.on_click = remove_efx;
    }

    ctx->gui.add_efx_button.x = ctx->sdl.win_width - MARGIN_RIGHT + 50;
    ctx->gui.add_efx_button.w = MARGIN_RIGHT - 100;
    ctx->gui.add_efx_button.h = 40;
    ctx->gui.add_efx_button.text = "ADD EFFECT";
    ctx->gui.add_efx_button.on_click = open_add_efx_popup;
    ctx->gui.add_efx_button.data = ctx;

    int efx_button_offset = 5;
    for(int i = 0; i < NUM_EFX; i++) {
        Button *efx_button = &ctx->gui.efx_buttons[i];
        efx_button->x = i % 2 == 0 ? 10 : 155;
        efx_button->y = efx_button_offset;
        efx_button->w = 135;
        efx_button->h = 30;
        efx_button->text = ctx->efx.efx_list[i].name;
        efx_button->on_click = add_efx;
        efx_button->data = &ctx->gui.efx_button_data[i];
        ctx->gui.efx_button_data[i].efx = &ctx->efx;
        ctx->gui.efx_button_data[i].id = (EfxID)i;
        efx_button_offset = i % 2 == 1? efx_button_offset + 40 : efx_button_offset;
    }

    return 0;
}

void cleanup_gui(AppContext *ctx) {
    if(font != NULL) {
        TTF_CloseFont(font);
    }
    TTF_Quit();

    for(int i = 0; i <= num_cached_text; i++) {
        SDL_DestroyTexture(text_cache[i].texture);
    }
    if(ctx->gui.add_efx_popup) {
        SDL_DestroyWindow(ctx->gui.add_efx_popup);
    }
}

void *exit_application(void *data) {
    AppState *state = (AppState *)data;
    state->running = false;
    return NULL;
}

void *remove_efx(void *data) {
    EfxCard *card = (EfxCard *)data;
    remove_from_pipeline(card->pipeline, card->id);
    return NULL;
}

void *open_add_efx_popup(void *data) {
    AppContext *ctx = (AppContext *)data;
    if(!ctx->gui.adding_efx) {
        if(ctx->gui.add_efx_popup == NULL) {
            create_add_efx_popup(ctx);
        } else {
            SDL_SetWindowPosition(
                    ctx->gui.add_efx_popup,
                    ctx->gui.add_efx_button.x,
                    ctx->gui.add_efx_button.y + ctx->gui.add_efx_button.h + 10
            );
            SDL_ShowWindow(ctx->gui.add_efx_popup);
        }
        ctx->gui.adding_efx = true;
    }
    return NULL;
}

void *add_efx(void *data) {
    EfxButtonData *efx_button_data = (EfxButtonData *)data;
    append_to_pipeline(efx_button_data->efx, efx_button_data->id);
    return NULL;
}

void *reset_efx(void *data) {
    EfxPipeline *efx = (EfxPipeline *)data;
    clear_pipeline(efx);
    return NULL;
}

void *randomize_efx(void *data) {
    (void)data;
    return NULL;
}

void *change_mode_3(void *data) {
    float *param = (float *)data;
    *param += 0.5f;
    if(*param > 1.0f) { *param = 0.0f; }
    return NULL;
}

void render_gui(AppContext *ctx, Image *image) {
    for(int i = 0; i < NUM_TOP_BUTTONS; i++) {
        draw_button(ctx->sdl.renderer, &ctx->gui.top_buttons[i]);
    }

    if(ctx->state.image_loaded) {
        int path_w, path_h;
        TTF_GetStringSize(font, image->path, 0, &path_w, &path_h);
        draw_text(ctx->sdl.renderer, ctx->sdl.win_width/2 - path_w/2,
                  ctx->sdl.win_height - MARGIN_Y/4 - path_h/2, WHITE, image->path);
    }

    update_efx_cards(ctx);

    if(ctx->gui.adding_efx) {
        SDL_SetWindowPosition(
                ctx->gui.add_efx_popup,
                ctx->gui.add_efx_button.x,
                ctx->gui.add_efx_button.y + ctx->gui.add_efx_button.h + 10
        );

        set_draw_color(ctx->gui.add_efx_popup_renderer, MID_GRAY);
        SDL_RenderClear(ctx->gui.add_efx_popup_renderer);

        for(int i = 0; i < NUM_EFX; i++) {
            draw_button(ctx->gui.add_efx_popup_renderer, &ctx->gui.efx_buttons[i]);
        }

        SDL_RenderPresent(ctx->gui.add_efx_popup_renderer);
    }
}

bool point_in_rect(SDL_Point point, int x, int y, int w, int h) {
    SDL_Rect rect = {.x = x, .y = y, .w = w, .h = h};
    return SDL_PointInRect(&point, &rect);
}

void handle_button_hover(SDL_Point mouse_pos, Button *button) {
    if(point_in_rect(mouse_pos, button->x, button->y, button->w, button->h)) {
        button->hovered = true;
    } else { button->hovered = false; }
}

void deal_gui_events(SDL_Event *event, AppContext *ctx) {
    Uint32 window_id = 0;
    switch(event->type) {
        case SDL_EVENT_MOUSE_MOTION: window_id = event->motion.windowID; break;
        case SDL_EVENT_MOUSE_BUTTON_DOWN: window_id = event->button.windowID; break;
        case SDL_EVENT_MOUSE_BUTTON_UP: window_id = event->button.windowID; break;
    }
    if(window_id == SDL_GetWindowID(ctx->gui.add_efx_popup) && ctx->gui.adding_efx) {
        process_popup_events(event, ctx);
    } else {
        process_gui_events(event, ctx);
    }
}

void process_gui_events(SDL_Event *event, AppContext *ctx) {
    switch(event->type) {
        case SDL_EVENT_MOUSE_MOTION: {
            SDL_Point mouse_pos = {(int)event->motion.x, (int)event->motion.y};
            
            for(int i = 0; i < NUM_TOP_BUTTONS; i++) {
                if(i == SAVE_IMAGE && !ctx->state.image_loaded) { continue; }
                handle_button_hover(mouse_pos, &ctx->gui.top_buttons[i]);
            }

            for(int i = 0; i < ctx->efx.count; i++) {
                EfxCard *card = &ctx->gui.efx_cards[i];
                bool card_hover_consumed = false;

                for(int j = 0; j < card->num_sliders; j++) {
                    Slider *slider = &card->sliders[j];
                    if(point_in_rect(mouse_pos, slider->x, slider->y, slider->w, slider->h)) {
                        slider->hovered = true;
                        card_hover_consumed = true;
                    } else { slider->hovered = false; }

                    if(slider->sliding) {
                        float *slider_val = (float *)slider->data;
                        *slider_val += event->motion.xrel * 0.003f;
                        *slider_val = clampf(*slider_val, 0.0f, 1.0f);
                        ctx->state.image_needs_update = true;
                    }
                }

                for(int j = 0; j < card->num_buttons; j++) {
                    handle_button_hover(mouse_pos, &card->buttons[j]);
                    if(card->buttons[j].hovered) { card_hover_consumed = true; }
                }

                handle_button_hover(mouse_pos, &card->x_button);
                if(card->x_button.hovered) { card_hover_consumed = true; };

                if(point_in_rect(mouse_pos, card->x, card->y, card->w, card->h) && !card_hover_consumed) {
                    card->hovered = true;
                } else { card->hovered = false; }

                if(card->dragging) {
                    card->y += event->motion.yrel;
                    float max_y = ctx->gui.add_efx_button.y - 10 - card->h;
                    card->y = clampf(card->y, MARGIN_Y/2, max_y);
                }
            }

            handle_button_hover(mouse_pos, &ctx->gui.add_efx_button); 
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            if(event->button.button == SDL_BUTTON_LEFT) {
                for(int i = 0; i < NUM_TOP_BUTTONS; i++) {
                    if(ctx->gui.top_buttons[i].hovered) {
                        ctx->gui.top_buttons[i].on_click(ctx->gui.top_buttons[i].data);
                        return;
                    }
                }
                if(ctx->gui.add_efx_button.hovered) {
                    ctx->gui.add_efx_button.on_click(ctx->gui.add_efx_button.data);
                    return;
                } 
                for(int i = 0; i < ctx->efx.count; i++) {
                    EfxCard *card = &ctx->gui.efx_cards[i];
                    if(card->x_button.hovered) {
                        card->x_button.on_click(card->x_button.data);
                        ctx->state.image_needs_update = true;
                        return;
                    }
                    for(int j = 0; j < card->num_sliders; j++) {
                        if(card->sliders[j].hovered) {
                            card->sliders[j].sliding = true;
                            return;
                        }
                    }
                    for(int j = 0; j < card->num_buttons; j++) {
                        if(card->buttons[j].hovered) {
                            card->buttons[j].on_click(card->buttons[j].data);
                            ctx->state.image_needs_update = true;
                            return;
                        }
                    }
                    if(card->hovered && ctx->efx.count > 1) {
                        card->dragging = true;
                    }
                }
            }
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_UP: {
            if(event->button.button == SDL_BUTTON_LEFT) {
                for(int i = 0; i < ctx->efx.count; i++) {
                    EfxCard *card = &ctx->gui.efx_cards[i];
                    for(int j = 0; j < card->num_sliders; j++) {
                        card->sliders[j].sliding = false;
                    }

                    if(card->dragging) {
                        card->dragging = false;
                        card->hovered = false;
                        EfxID dragged_id = card->id;
                        for(int j = 0; j < ctx->efx.count; j++) {
                           EfxCard *card_2 = &ctx->gui.efx_cards[j];
                           if(card_2->hovered && card_2->id != dragged_id) {
                                swap_in_pipeline(&ctx->efx, dragged_id, card_2->id);
                                ctx->state.image_needs_update = true;
                                return;
                           }
                        }
                    }
                }
            }
            break;
        }
    }
}

void process_popup_events(SDL_Event *event, AppContext *ctx) {
    switch(event->type) {
        case SDL_EVENT_MOUSE_MOTION: {
            SDL_Point mouse_pos = {(int)event->motion.x, (int)event->motion.y};
            for(int i = 0; i < NUM_EFX; i++) {
                handle_button_hover(mouse_pos, &ctx->gui.efx_buttons[i]);
            }
            break;
        }

        case SDL_EVENT_MOUSE_BUTTON_DOWN: {
            for(int i = 0; i < NUM_EFX; i++) {
                if(ctx->gui.efx_buttons[i].hovered) {
                    ctx->gui.efx_buttons[i].on_click(ctx->gui.efx_buttons[i].data);
                    ctx->state.image_needs_update = true;
                    ctx->gui.adding_efx = false;
                    SDL_HideWindow(ctx->gui.add_efx_popup);
                }
            }
            break;
        }
    }
}

void draw_text(SDL_Renderer *renderer, int x, int y, SDL_Color font_color, char *text) {
    int w, h;
    SDL_Texture *texture = get_cached_text(renderer, &w, &h, font_color, text);
    SDL_FRect rect = {x, y - 2, w, h};
    SDL_RenderTexture(renderer, texture, NULL, &rect);
}

SDL_Texture *get_cached_text(SDL_Renderer *renderer, int *w, int *h, SDL_Color font_color, char *text) {
    for(int i = 0; i < num_cached_text; i++) {
        if(strcmp(text, text_cache[i].text) == 0 && text_cache[i].renderer == renderer) {
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
        entry->renderer = renderer;
    }

    return texture;
}

void draw_button(SDL_Renderer *renderer, Button *button) {
    SDL_FRect border_rect = {button->x, button->y, button->w, button->h};
    SDL_FRect bg_rect = {button->x + 2, button->y + 2, button->w - 4, button->h - 4};

    if(button->hovered) { set_draw_color(renderer, ORANGE); }
    else { set_draw_color(renderer, BLACK); }
    SDL_RenderFillRect(renderer, &border_rect);

    if(button->hovered) { set_draw_color(renderer, MID_GRAY); }
    else { set_draw_color(renderer, DARK_GRAY); }
    SDL_RenderFillRect(renderer, &bg_rect);

    int text_w, text_h;
    TTF_GetStringSize(font, button->text, 0, &text_w, &text_h);
    int text_x = (button->x + button->w/2) - (text_w/2);
    int text_y = (button->y + button->h/2) - (text_h/2);
    draw_text(renderer, text_x, text_y, WHITE, button->text);
}

void draw_slider(SDL_Renderer *renderer, Slider *slider) {
    SDL_FRect border_rect = {slider->x, slider->y, slider->w, slider->h};
    SDL_FRect bg_rect = {slider->x + 2, slider->y + 2, slider->w - 4, slider->h - 4};
    SDL_FRect slider_rect = {bg_rect.x, bg_rect.y, bg_rect.w * *(float *)slider->data, bg_rect.h};

    if(slider->hovered) { set_draw_color(renderer, ORANGE); }
    else { set_draw_color(renderer, BLACK); }
    SDL_RenderFillRect(renderer, &border_rect);

    set_draw_color(renderer, DARK_GRAY);
    SDL_RenderFillRect(renderer, &bg_rect);

    set_draw_color(renderer, LIGHT_GRAY);
    SDL_RenderFillRect(renderer, &slider_rect);

    int text_w, text_h;
    TTF_GetStringSize(font, slider->text, 0, &text_w, &text_h);
    int text_x = (slider->x + slider->w/2) - (text_w/2);
    int text_y = (slider->y + slider->h/2) - (text_h/2);

    draw_text(renderer, text_x, text_y, WHITE, slider->text);
}

void update_efx_cards(AppContext *ctx) {
    int card_offset = 0;
    for(int i = 0; i < ctx->efx.count; i++) {
        EfxCard *card = &ctx->gui.efx_cards[i];
        Efx *efx = ctx->efx.pipeline[i];

        card->num_sliders = 0;
        card->num_buttons = 0;

        card->x = ctx->sdl.win_width - MARGIN_RIGHT + 5;
        if(!card->dragging) {
            card->y = MARGIN_Y/2 + card_offset;
        }
        card->id = efx->id;
        card->name = efx->name;

        int param_offset = 30;
        for(int j = 0; j < efx->num_params; j++) {
            if(efx->param_types[j] == 0) {
                card->sliders[card->num_sliders].x = card->x + 25;
                card->sliders[card->num_sliders].y = card->y + param_offset;
                card->sliders[card->num_sliders].w = card->w - 50;
                card->sliders[card->num_sliders].h = 20;
                card->sliders[card->num_sliders].text = efx->param_names[j];
                card->sliders[card->num_sliders].data = &efx->params[j];
                card->num_sliders += 1;
                param_offset += 30;
            } else {
                card->buttons[card->num_buttons].x = card->x + 50;
                card->buttons[card->num_buttons].y = card->y + param_offset;
                card->buttons[card->num_buttons].w  = card->w - 100;
                card->buttons[card->num_buttons].h = 30;
                card->buttons[card->num_buttons].text = efx->param_names[j];
                card->buttons[card->num_buttons].on_click = change_mode_3;
                card->buttons[card->num_buttons].data = &efx->params[j];
                card->num_buttons += 1;
                param_offset += 40;
            }
        }
        card->h = param_offset;

        card->x_button.x = card->x + card->w - 25;
        card->x_button.y = card->y + 5;
        card->x_button.data = card;

        draw_efx_card(ctx->sdl.renderer, card);

        card_offset += param_offset + 10;
    }
    ctx->gui.add_efx_button.x = ctx->sdl.win_width - MARGIN_RIGHT + 50;
    ctx->gui.add_efx_button.y = MARGIN_Y/2 + card_offset;
    draw_button(ctx->sdl.renderer, &ctx->gui.add_efx_button);
}

void draw_efx_card(SDL_Renderer *renderer, EfxCard *card) {
    SDL_FRect border_rect = {card->x, card->y, card->w, card->h};
    SDL_FRect bg_rect = {card->x + 2, card->y + 2, card->w - 4, card->h - 4};

    if(card->hovered) { set_draw_color(renderer, ORANGE); }
    else { set_draw_color(renderer, BLACK); }
    SDL_RenderFillRect(renderer, &border_rect);

    if(card->hovered) { set_draw_color(renderer, MID_GRAY); }
    else { set_draw_color(renderer, DARK_GRAY); }
    SDL_RenderFillRect(renderer, &bg_rect);

    int text_w, text_h;
    TTF_GetStringSize(font, card->name, 0, &text_w, &text_h);
    int text_x = (card->x + card->w/2) - (text_w/2);
    int text_y = card->y + 5;
    draw_text(renderer, text_x, text_y, WHITE, card->name);

    draw_button(renderer, &card->x_button);

    for(int i = 0; i < card->num_sliders; i++) {
        draw_slider(renderer, &card->sliders[i]);
    }
    for(int i = 0; i < card->num_buttons; i++) {
        draw_button(renderer, &card->buttons[i]);
    }
}

void create_add_efx_popup(AppContext *ctx) {
    ctx->gui.add_efx_popup = SDL_CreatePopupWindow(
            ctx->sdl.window,
            ctx->gui.add_efx_button.x,
            ctx->gui.add_efx_button.y + ctx->gui.add_efx_button.h + 10,
            300, 400,
            SDL_WINDOW_POPUP_MENU | SDL_WINDOW_NOT_FOCUSABLE);
    if(ctx->gui.add_efx_popup == NULL) {
        fprintf(stderr, "Failed to create the popup window: %s\n", SDL_GetError());
    }
    ctx->gui.add_efx_popup_renderer = SDL_CreateRenderer(ctx->gui.add_efx_popup, NULL);
    if(ctx->gui.add_efx_popup_renderer == NULL) {
        fprintf(stderr, "Failed to create the popup renderer: %s\n", SDL_GetError());
    }
}
