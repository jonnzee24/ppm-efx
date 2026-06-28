#ifndef GUI_H
#define GUI_H

#include "effects.h"
#include <SDL2/SDL_ttf.h>

int init_gui(void);
void cleanup_gui(void);

void draw_text(SDL_Renderer *renderer,
               int x_pos,
               int y_pos,
               SDL_Color font_color,
               char *text);

void draw_hud(SDL_Renderer *renderer, 
              int x_pos, 
              int y_pos, 
              EffectFlags effects,
              EffectParams params,
              int mode);

#endif
