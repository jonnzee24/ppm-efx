#ifndef GUI_H
#define GUI_H

#include <SDL.h>

#include "common.h"

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


void draw_debug_info(SDL_Context *sdl,
                     const Image *image,
                     const UserParams *usr,
                     float scale);

#endif
