#ifndef GUI_H
#define GUI_H

#include <SDL.h>
#include <stdbool.h>

#define MARGIN_Y 60
#define MARGIN_X 400

typedef struct AppContext AppContext;
typedef struct Image Image;

typedef struct {
    enum {
        TOP_BAR,
        EFX_BTN
    } button_type;
    int x, y, w, h;
    char *text;
    bool hovered;
    bool clicked;
    void *(*on_click)(void *data);
    void *data;
} Button;

typedef enum {
    LOAD_IMAGE,
    SAVE_IMAGE,
    EXIT,

    SATURATION,
    CONTRAST,
    EXPOSURE,
    INVERT,
    MONO,
    QUANTIZE,
    COLOR_BIAS,
    COLOR_SHIFT,
    WARP,
    PIXELATE,
    DITHER,

    NUM_BUTTONS
} ButtonIDs;

typedef struct {
    Button buttons[NUM_BUTTONS];
} GuiState;

int init_gui(AppContext *ctx, Image *image);
void cleanup_gui(void);
void update_gui(AppContext *ctx);

// Button callback functions
void *button_dummy(void *data);
void *exit_application(void *data);
void *toggle_efx(void *data);

void process_gui_events(SDL_Event *event, AppContext *ctx);

void render_gui(AppContext *ctx, Image *image);

void draw_button(SDL_Renderer *renderer, Button *button);

void draw_text(SDL_Renderer *renderer,
               int x_pos,
               int y_pos,
               SDL_Color font_color,
               char *text);

void draw_debug_info(AppContext *ctx, Image *image);

#endif
