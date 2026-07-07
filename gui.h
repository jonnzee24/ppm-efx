#ifndef GUI_H
#define GUI_H

#include "effects.h"
#include <SDL_ttf.h>

#define WHITE (SDL_Color){255, 255, 255, 255}
#define GRAY (SDL_Color){180, 180, 180, 255}
#define DARK_RED (SDL_Color){100, 0, 0, 255}
#define DARK_GREEN (SDL_Color){0, 100, 0, 255}

#define HUD_WIDTH 300

int init_gui(void);
void cleanup_gui(void);

void draw_text(SDL_Renderer *renderer,
               int x_pos,
               int y_pos,
               SDL_Color font_color,
               char *text);

int draw_hud(SDL_Renderer *renderer, 
              int x_pos, 
              int y_pos, 
              EffectFlags effects,
              EffectParams params,
              int mode);

#endif
