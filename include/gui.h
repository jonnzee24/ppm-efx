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
        GENERIC,
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

    WARP_MODE,
    THRESHOLD_MODE,
    COLOR_BIAS_MODE,
    DITHER_MODE,

    SATURATION,
    CONTRAST,
    EXPOSURE,
    INVERT,
    MONO,
    THRESHOLD,
    QUANTIZE,
    COLOR_BIAS,
    COLOR_SHIFT,
    WARP,
    PIXELATE,
    DITHER,

    NUM_BUTTONS
} ButtonIDs;

typedef struct {
    int x, y, w, h;
    char *text;
    bool hovered;
    bool sliding;
    void *data;
} Slider;

typedef enum {
    SATURATION_VAL,
    CONTRAST_VAL,
    EXPOSURE_VAL,
    THRESHOLD_VAL,
    BIT_DEPTH,
    COLOR_SHIFT_VAL,
    PIXEL_SIZE,
    DITHER_BRIGHTNESS,
    SINE_LENGTH,
    SINE_AMP,

    NUM_SLIDERS
} SliderIDs;

typedef struct {
    Button buttons[NUM_BUTTONS];
    Slider sliders[NUM_SLIDERS];
} GuiState;

int init_gui(AppContext *ctx, Image *image);
void cleanup_gui(void);
void update_gui(AppContext *ctx);
void render_gui(AppContext *ctx, Image *image);
void process_gui_events(SDL_Event *event, AppContext *ctx);

#endif
