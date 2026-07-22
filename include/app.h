#ifndef APP_H
#define APP_H

#include "common.h"
#include "effects.h"
#include "gui.h"

typedef struct AppState {
    bool running;
    bool image_loaded;
} AppState;

struct AppContext {
    AppState state;
    SDLContext sdl;
    GuiContext gui;
    UserParams usr;
    EfxPipeline efx;
};

#endif
