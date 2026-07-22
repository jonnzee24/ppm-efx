#ifndef GUI_H
#define GUI_H

#include <SDL3/SDL.h>
#include <stdbool.h>

#include "effects.h"

typedef struct AppContext AppContext;
typedef struct Image Image;

typedef struct Button {
    int x, y, w, h;
    char *text;
    bool clicked;
    bool hovered;
    void *(*on_click)(void *data);
    void *data;
} Button;

typedef enum ButtonIDs {
    LOAD_IMAGE,
    SAVE_IMAGE,
    RESET,
    RANDOMIZE,
    EXIT,
    NUM_TOP_BUTTONS
} TopBtnIDs;

typedef struct Slider {
    int x, y, w, h;
    char *text;
    bool hovered;
    bool sliding;
    void *data;
} Slider;

typedef struct EfxCard {
    EfxPipeline *pipeline;
    EfxID id;
    float x, y, w, h;
    char *name;
    int num_sliders;
    Slider sliders[4];
    int num_buttons;
    Button buttons[4];
    Button x_button;
    bool hovered;
    bool dragging;
} EfxCard;

typedef struct EfxButtonData {
    EfxPipeline *efx;
    EfxID id;
} EfxButtonData;

typedef struct GuiContext {
    Button top_buttons[NUM_TOP_BUTTONS];
    EfxCard efx_cards[NUM_EFX];
    Button add_efx_button;
    Button efx_buttons[NUM_EFX];
    EfxButtonData efx_button_data[NUM_EFX];
    SDL_Window *add_efx_popup;
    SDL_Renderer *add_efx_popup_renderer;
    bool adding_efx;
} GuiContext;

int init_gui(AppContext *ctx, Image *image);
void cleanup_gui(AppContext *ctx);
void render_gui(AppContext *ctx, Image *image);
void deal_gui_events(SDL_Event *event, AppContext *ctx);

#endif
