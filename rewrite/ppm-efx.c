#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <SDL.h>

#include "headers/ppm-efx.h"
#include "headers/file_io.h"
#include "headers/effects.h"

int main(void) {
    Image image = {0};
    if(load_image(&image) != 0) {
        perror("Failed to load the image");
        goto exit;
    }

    SDL_Context sdl = {0};
    if(init_sdl(&sdl, &image) != 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());  
        goto exit;
    }

    bool running = true;
    while(running) {
        render(&sdl, &image);
        
        SDL_Event event;
        while(SDL_PollEvent(&event) != 0) {
            switch(event.type) {
                case SDL_QUIT:
                    running = false;
            }
        }
        SDL_Delay(20);
    }


exit:
    if(image.path)        free(image.path);
    if(image.framebuffer) free(image.framebuffer);
    if(image.original)    free(image.original);

    if(sdl.window)   SDL_DestroyWindow(sdl.window);
    if(sdl.renderer) SDL_DestroyRenderer(sdl.renderer);
    if(sdl.texture)  SDL_DestroyTexture(sdl.texture);
    SDL_PumpEvents();
    SDL_Quit();

    return 0;
}

int init_sdl(SDL_Context *sdl, const Image *image) {
    if(SDL_Init(SDL_INIT_VIDEO) != 0) { return 1; }

    sdl->win_width = image->width;
    sdl->win_height = image->height > 1000 ? 1000 : image->height;
    
    sdl->window = SDL_CreateWindow("PPM EFX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, sdl->win_width, sdl->win_height, SDL_WINDOW_RESIZABLE);
    if(sdl->window == NULL) { return 1; }

    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
    if(sdl->renderer == NULL) { return 1; }

    sdl->texture = SDL_CreateTexture(sdl->renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, image->width, image->height);
    sdl->texture_rect = (SDL_Rect){0, 0, image->width, image->height};
    if(sdl->texture == NULL) { return 1; }

    return 0;
}

void render(SDL_Context *sdl, const Image *image) {
    SDL_UpdateTexture(sdl->texture, NULL, image->framebuffer, image->width * 3);
    SDL_RenderClear(sdl->renderer);
    
    SDL_GetWindowSize(sdl->window, &sdl->win_width, &sdl->win_height);
    int image_vp_width = sdl->win_width;
    int image_vp_height = sdl->win_height;
    SDL_Rect image_vp = { 0, 0, image_vp_width, image_vp_height};
    
    // Set the scale
    float scale_x = (float)image_vp_width / image->width;
    float scale_y = (float)image_vp_height / image->height;
    float scale = (scale_x < scale_y) ? scale_x : scale_y;

    // Draw the image
    SDL_RenderSetViewport(sdl->renderer, &image_vp);

    sdl->texture_rect.w = (int)image->width  * scale;
    sdl->texture_rect.h = (int)image->height * scale;
    sdl->texture_rect.x = ((sdl->win_width / 2) - sdl->texture_rect.w / 2);
    sdl->texture_rect.y = ((sdl->win_height / 2) - sdl->texture_rect.h / 2);

    SDL_RenderCopy(sdl->renderer, sdl->texture, NULL, &sdl->texture_rect);
    
    // Reset viewport and render everything
    SDL_RenderSetViewport(sdl->renderer, NULL);
    SDL_RenderPresent(sdl->renderer);
}
