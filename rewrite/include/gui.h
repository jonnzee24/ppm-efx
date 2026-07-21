#ifndef GUI_H
#define GUI_H

#include <SDL3/SDL.h>
#include <stdbool.h>

typedef struct AppContext AppContext;

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

typedef struct GuiContext {
    Button top_buttons[NUM_TOP_BUTTONS];
} GuiContext;

int init_gui(AppContext *ctx, Image *image);
void cleanup_gui(void);
void render_gui(AppContext *ctx, Image *image);
void process_gui_events(SDL_Event *event, AppContext *ctx);

#endif
