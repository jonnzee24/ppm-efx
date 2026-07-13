#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <SDL.h>

#include "common.h"
#include "file_io.h"
#include "effects.h"
#include "gui.h"

int init_sdl(AppContext *ctx);
int create_image_texture(SDL_Context *sdl, Image *image);
void update(AppContext *ctx, Image *image);
void render(AppContext *ctx, Image *image);
void process_events(AppContext *ctx, Image *image);

int main(void) {
    AppContext ctx = {0};
    Image image = {0};

    ctx.state.running = true;
    ctx.state.needs_update = true;
    ctx.state.image_loaded = false;
    ctx.usr.scale = 1.0f;
    
    // initial effect parameter values
    ctx.params.warp_mode = MIRROR_X;
    ctx.params.color_bias = R;
    ctx.params.sine_length = 0.5f;
    ctx.params.sine_amp = 0.5f;
    ctx.params.dither_brightness = 0.5f;
    ctx.params.pixel_size = 0.1f;
    ctx.params.threshold_val = 0.5f;
    ctx.params.threshold_mode = 0;
    ctx.params.bit_depth = 0.5f;
    ctx.params.exposure_val = 0.5f;
    ctx.params.contrast_val = 0.5f;
    ctx.params.saturation_val = 0.5f;
    ctx.params.color_shift_val = 0.5f;
    ctx.params.dither_mode = 0;

    if(init_sdl(&ctx) != 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());  
        goto exit;
    }
    if(init_gui(&ctx, &image) != 0) {
        fprintf(stderr, "Failed to initialize the GUI: %s\n", SDL_GetError());
        return 1;
    }

    while(ctx.state.running) {
        SDL_GetWindowSize(ctx.sdl.window, &ctx.sdl.win_width, &ctx.sdl.win_height);
        SDL_GetMouseState(&ctx.usr.mx, &ctx.usr.my);

        if(image.needs_reload) {
            if(image.texture) {
                SDL_DestroyTexture(image.texture);
            }

            if(create_image_texture(&ctx.sdl, &image) != 0) {
                fprintf(stderr, "Failed to create image texture: %s\n", SDL_GetError());
            }

            ctx.usr.scale = 1.0f;
            ctx.usr.x_offset = 0;
            ctx.usr.y_offset = 0;

            ctx.state.needs_update = true;
            image.needs_reload = false;
            ctx.state.image_loaded = true;
        }

        if(ctx.state.needs_update) {
            update(&ctx, &image);
            ctx.state.needs_update = false;
        }

        render(&ctx, &image);
        process_events(&ctx, &image);
        SDL_Delay(10);
    }
   
exit:
    if(ctx.sdl.window)   SDL_DestroyWindow(ctx.sdl.window);
    if(ctx.sdl.renderer) SDL_DestroyRenderer(ctx.sdl.renderer);
    if(image.texture)  SDL_DestroyTexture(image.texture);
    cleanup_gui();
    SDL_Quit();

    if(image.path)        free(image.path);
    if(image.framebuffer) free(image.framebuffer);
    if(image.original)    free(image.original);

    return 0;
}

int init_sdl(AppContext *ctx) {
    if(SDL_Init(SDL_INIT_VIDEO) != 0) { return 1; }

    ctx->sdl.win_width = 1600;
    ctx->sdl.win_height = 900;
    
    ctx->sdl.window = SDL_CreateWindow("PPM EFX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
                                       ctx->sdl.win_width, ctx->sdl.win_height, SDL_WINDOW_RESIZABLE);
    if(ctx->sdl.window == NULL) { return 1; }

    SDL_SetWindowMinimumSize(ctx->sdl.window, 1600, 900);

    ctx->sdl.renderer = SDL_CreateRenderer(ctx->sdl.window, -1, SDL_RENDERER_ACCELERATED);
    if(ctx->sdl.renderer == NULL) { return 1; }

    ctx->sdl.image_vp = (SDL_Rect){MARGIN_X / 2, MARGIN_Y / 2, ctx->sdl.win_width - MARGIN_X, ctx->sdl.win_height - MARGIN_Y};

    return 0;
}

int create_image_texture(SDL_Context *sdl, Image *image) {
    image->texture = SDL_CreateTexture(sdl->renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, image->width, image->height);
    if(image->texture == NULL) { return 1; }
    return 0;
}

void update(AppContext *ctx, Image *image) {
    if(ctx->state.image_loaded) { apply_efx(image, &ctx->efx, &ctx->params); }
    SDL_UpdateTexture(image->texture, NULL, image->framebuffer, image->width * 3);
}

void render(AppContext *ctx, Image *image) {
    ctx->sdl.image_vp.w = ctx->sdl.win_width - MARGIN_X;
    ctx->sdl.image_vp.h = ctx->sdl.win_height - MARGIN_Y;

    SDL_SetRenderDrawColor(ctx->sdl.renderer, 80, 80, 80, 255);
    SDL_RenderClear(ctx->sdl.renderer);
    
    SDL_SetRenderDrawColor(ctx->sdl.renderer, 60, 60, 60, 255);
    SDL_RenderFillRect(ctx->sdl.renderer, &ctx->sdl.image_vp);

    render_gui(ctx, image);

    if(ctx->state.image_loaded) {
        image->texture_rect.w = (int)image->width  * ctx->usr.scale;
        image->texture_rect.h = (int)image->height * ctx->usr.scale;
        image->texture_rect.x = (ctx->sdl.image_vp.w / 2) - (image->texture_rect.w / 2) + ctx->usr.x_offset;
        image->texture_rect.y = (ctx->sdl.image_vp.h / 2) - (image->texture_rect.h / 2) + ctx->usr.y_offset;

        SDL_RenderSetViewport(ctx->sdl.renderer, &ctx->sdl.image_vp);
        SDL_RenderCopy(ctx->sdl.renderer, image->texture, NULL, &image->texture_rect);
    }

    SDL_RenderSetViewport(ctx->sdl.renderer, NULL);
    SDL_RenderPresent(ctx->sdl.renderer);
}

void process_events(AppContext *ctx, Image *image) {
    SDL_Event event;
    while(SDL_PollEvent(&event) != 0) {
        process_gui_events(&event, ctx);

        switch(event.type) {
            case SDL_QUIT:
                ctx->state.running = false;
                break;

            case SDL_WINDOWEVENT:
                update_gui(ctx);
                break;

            case SDL_MOUSEBUTTONDOWN:
                if(event.button.button == SDL_BUTTON_LEFT) {
                    SDL_Point mouse_pos = {ctx->usr.mx, ctx->usr.my};

                    SDL_Rect image_box = {
                        image->texture_rect.x + (MARGIN_X * 0.5),
                        image->texture_rect.y + (MARGIN_Y * 0.5),
                        image->texture_rect.w,
                        image->texture_rect.h
                    };

                    if(SDL_PointInRect(&mouse_pos, &image_box) && SDL_PointInRect(&mouse_pos, &ctx->sdl.image_vp)) {
                        ctx->usr.panning = true;
                    }
                }
                break;

            case SDL_MOUSEBUTTONUP:
                if(event.button.button == SDL_BUTTON_LEFT) {
                    ctx->usr.panning = false;
                }
                break;

            case SDL_MOUSEMOTION:
                if(ctx->usr.panning) {
                    ctx->usr.x_offset += event.motion.xrel;
                    ctx->usr.y_offset += event.motion.yrel;
                    int max_x = ctx->sdl.image_vp.w * 0.5;
                    int max_y = ctx->sdl.image_vp.h * 0.5;
                    ctx->usr.x_offset = clamp(ctx->usr.x_offset, -max_x, max_x);
                    ctx->usr.y_offset = clamp(ctx->usr.y_offset, -max_y, max_y);
                }
                break;

            case SDL_MOUSEWHEEL:
                ctx->usr.scale += event.wheel.y * ctx->usr.scale / 10;
                ctx->usr.scale = clampf(ctx->usr.scale, 0.1, 20);
                break;

            case SDL_KEYDOWN:
                switch(event.key.keysym.sym) {
                    case SDLK_MINUS:
                        ctx->usr.scale -= 0.1;
                        break;

                    case SDLK_PLUS:
                        ctx->usr.scale += 0.1;
                        break;

                    case SDLK_ESCAPE:
                        ctx->usr.scale = 1.0;
                        ctx->usr.x_offset = 0;
                        ctx->usr.y_offset = 0;
                        break;
                }
        }
    }
}
