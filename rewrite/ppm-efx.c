#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <SDL.h>

#include "common.h"
#include "ppm-efx.h"
#include "file_io.h"
#include "effects.h"
#include "gui.h"

int main(int argc, char **argv) {
    (void)argv;
    if(argc != 1) {
        printf("PPM EFX doesnt take any arguments!\n");
        return 1;
    }
    
    Image image = {0};
    Output output = {0};

    SDL_Context sdl = {0};

    EffectFlags efx = {0};
    EffectParams params = {
        .warp_mode = 1,
        .sine_length = 120.0f,
        .sine_amp = 20.0f,
        .dither_brightness = 0.8f,
        .pixel_size = 4,
        .mono_do_thresh = false,
        .mono_thresh = 140,
        .bit_depth = 4,
        .exposure_val = 1.5f,
        .contrast_val = 30,
        .saturation_val = 1.4f,
        .color_bias = 0,
        .color_shift = 0.4f
    };

    AppState state = {
        .running = true,
        .needs_update = true,
        .needs_render = true,
        .do_output = false
    };

    UserParams usr = {
        .x_offset = 0,
        .y_offset = 0,
        .scale = 1.0f,
        .panning = false,
        .mx = 0,
        .my = 0
    };
    
    if(load_image(&image) != 0) {
        perror("Failed to load the image");
        goto exit;
    }

    if(init_sdl(&sdl, &image) != 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());  
        goto exit;
    }

    while(state.running) {
        SDL_GetWindowSize(sdl.window, &sdl.win_width, &sdl.win_height);
        SDL_GetMouseState(&usr.mx, &usr.my);

        if(state.needs_update) {
            update(&sdl, &image, &efx, &params);
            state.needs_update = false;
        }

        if(state.needs_render) {
            render(&sdl, &image, &usr);
            state.needs_render = false;
        }
        
        process_events(&usr, &sdl, &state, &efx);
        SDL_Delay(10);
    }
    
    if(output_prompt(&output, &state.do_output) != 0) {
        perror("Failed to create the output file");
        goto exit;
    }

    if(state.do_output) {
        fprintf(output.file, "P6\n# Made in PPM EFX.\n%d %d\n255\n", image.width, image.height);
        fwrite(image.framebuffer, sizeof(Uint8), image.framebuffer_size, output.file);
        fclose(output.file);

        if(output.format == PPM) {
            printf("File saved successfully at %s!\n", output.path);
        }
        else if(output.format == PNG) {
            convert_to_png(output.path);
        }
    }

exit:
    if(image.path)        free(image.path);
    if(image.framebuffer) free(image.framebuffer);
    if(image.original)    free(image.original);
    if(output.path)       free(output.path);

    if(sdl.window)   SDL_DestroyWindow(sdl.window);
    if(sdl.renderer) SDL_DestroyRenderer(sdl.renderer);
    if(sdl.texture)  SDL_DestroyTexture(sdl.texture);
    cleanup_gui();
    SDL_PumpEvents();
    SDL_Quit();

    return 0;
}

int init_sdl(SDL_Context *sdl, const Image *image) {
    if(SDL_Init(SDL_INIT_VIDEO) != 0) { return 1; }
    if(init_gui() != 0) {
        return 1;
    }

    sdl->win_width = image->width;
    sdl->win_height = image->height > 1000 ? 1000 : image->height;
    
    sdl->window = SDL_CreateWindow("PPM EFX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, sdl->win_width, sdl->win_height, SDL_WINDOW_RESIZABLE);
    if(sdl->window == NULL) { return 1; }

    sdl->renderer = SDL_CreateRenderer(sdl->window, -1, SDL_RENDERER_ACCELERATED);
    if(sdl->renderer == NULL) { return 1; }

    sdl->texture = SDL_CreateTexture(sdl->renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, image->width, image->height);
    sdl->texture_rect = (SDL_Rect){0, 0, image->width, image->height};
    sdl->image_vp = (SDL_Rect){MARGIN_X / 2, MARGIN_Y / 2, sdl->win_width - MARGIN_X, sdl->win_height - MARGIN_Y};
    if(sdl->texture == NULL) { return 1; }

    return 0;
}

void update(SDL_Context *sdl, Image *image, const EffectFlags *efx, const EffectParams *params) {
    apply_efx(image, efx, params);
    SDL_UpdateTexture(sdl->texture, NULL, image->framebuffer, image->width * 3);
}

void render(SDL_Context *sdl, const Image *image, UserParams *usr) {
    SDL_SetRenderDrawColor(sdl->renderer, 80, 80, 80, 255);
    SDL_RenderClear(sdl->renderer);
    
    sdl->image_vp.w = sdl->win_width - MARGIN_X;
    sdl->image_vp.h = sdl->win_height - MARGIN_Y;

    SDL_SetRenderDrawColor(sdl->renderer, 100, 100, 100, 255);
    SDL_RenderFillRect(sdl->renderer, &sdl->image_vp);

    float scale_x = (float)sdl->image_vp.w / image->width;
    float scale_y = (float)sdl->image_vp.h / image->height;
    float scale = (scale_x < scale_y) ? scale_x : scale_y;
    usr->scale = clampf(usr->scale, 0.1f, 20.0f);
    scale *= usr->scale;

    sdl->texture_rect.w = (int)image->width  * scale;
    sdl->texture_rect.h = (int)image->height * scale;
    sdl->texture_rect.x = (sdl->image_vp.w / 2) - (sdl->texture_rect.w / 2) + usr->x_offset;
    sdl->texture_rect.y = (sdl->image_vp.h / 2) - (sdl->texture_rect.h / 2) + usr->y_offset;

    SDL_RenderSetViewport(sdl->renderer, &sdl->image_vp);
    SDL_RenderCopy(sdl->renderer, sdl->texture, NULL, &sdl->texture_rect);
    
    SDL_RenderSetViewport(sdl->renderer, NULL);
    draw_debug_info(sdl, image, usr, scale);

    SDL_RenderPresent(sdl->renderer);
}

void process_events(UserParams *usr, SDL_Context *sdl, AppState *state, EffectFlags *efx) {
    SDL_Event event;
    while(SDL_PollEvent(&event) != 0) {
        switch(event.type) {
            case SDL_QUIT:
                state->running = false;
                break;

            case SDL_WINDOWEVENT:
                state->needs_render = true;
                break;

            case SDL_MOUSEBUTTONDOWN:
                if(event.button.button == SDL_BUTTON_LEFT) {
                    SDL_Point mouse_point = { usr->mx, usr->my };

                    SDL_Rect image_box = {
                        sdl->texture_rect.x + (MARGIN_X / 2),
                        sdl->texture_rect.y + (MARGIN_Y / 2),
                        sdl->texture_rect.w,
                        sdl->texture_rect.h
                    };

                    if(SDL_PointInRect(&mouse_point, &image_box)) {
                        usr->panning = true;
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if(event.button.button == SDL_BUTTON_LEFT) {
                    usr->panning = false;
                }
                break;

            case SDL_MOUSEMOTION:
                if(usr->panning) {
                    usr->x_offset += event.motion.xrel;
                    usr->y_offset += event.motion.yrel;
                    int max_x = sdl->image_vp.w * 0.5;
                    int max_y = sdl->image_vp.h * 0.5;
                    usr->x_offset = clamp(usr->x_offset, -max_x, max_x);
                    usr->y_offset = clamp(usr->y_offset, -max_y, max_y);
                }

                state->needs_render = true;
                break;

            case SDL_MOUSEWHEEL:
                usr->scale += event.wheel.y * usr->scale / 10;

                state->needs_render = true;
                break;

            case SDL_KEYDOWN:
                #define TOGGLE_EFFECT(flag) \
                    do { \
                        (flag) = !(flag); \
                        state->needs_update = true; \
                        state->needs_render = true; \
                    } while(0)

                switch(event.key.keysym.sym) {
                    case SDLK_d: TOGGLE_EFFECT(efx->dither); break;
                    case SDLK_w: TOGGLE_EFFECT(efx->warp); break;
                    case SDLK_m: TOGGLE_EFFECT(efx->mono); break;
                    case SDLK_q: TOGGLE_EFFECT(efx->quantize); break;
                    case SDLK_z: TOGGLE_EFFECT(efx->color_shift); break;
                    case SDLK_e: TOGGLE_EFFECT(efx->exposure); break;
                    case SDLK_c: TOGGLE_EFFECT(efx->contrast); break;
                    case SDLK_x: TOGGLE_EFFECT(efx->color_bias); break;
                    case SDLK_s: TOGGLE_EFFECT(efx->saturation); break;
                    case SDLK_p: TOGGLE_EFFECT(efx->pixelate); break;
                    case SDLK_i: TOGGLE_EFFECT(efx->invert); break;

                    case SDLK_MINUS:
                        usr->scale -= 0.1;
                        state->needs_render = true;
                        break;

                    case SDLK_PLUS:
                        usr->scale += 0.1;
                        state->needs_render = true;
                        break;

                    case SDLK_ESCAPE:
                        usr->scale = 1.0;
                        usr->x_offset = 0;
                        usr->y_offset = 0;
                        state->needs_render = true;
                        break;
                }
        }
    }
}
