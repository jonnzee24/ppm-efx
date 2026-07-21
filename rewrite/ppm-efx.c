#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <stdbool.h>
#include <SDL3/SDL.h>
#include <time.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "common.h"
#include "file_io.h"
#include "effects.h"
#include "gui.h"
#include "app.h"

int init_sdl(AppContext *ctx);
int create_image_texture(SDLContext *sdl, Image *image);
void update(AppContext *ctx, Image *image);
void recenter_image(AppContext *ctx, Image *image);
void render(AppContext *ctx, Image *image);
void process_events(AppContext *ctx, Image *image);

int main(void) {
    srand((unsigned int)time(NULL));
    AppContext ctx = {0};
    Image image = {0};

    ctx.state.running = true;
    ctx.state.needs_update = true;
    ctx.state.image_loaded = false;

    image.needs_reload = false;
    image.needs_update = false;
    
    if(init_sdl(&ctx) != 0) {
        fprintf(stderr, "Failed to initialize SDL: %s\n", SDL_GetError());  
        goto exit;
    }
    if(init_gui(&ctx, &image) != 0) {
        fprintf(stderr, "Failed to initialize the GUI: %s\n", SDL_GetError());
        return 1;
    }
    if(init_efx(ctx.efx.efx_list) != 0) {
        perror("Failed to initialize the EFX");
        return 1;
    }

    while(ctx.state.running) {
        process_events(&ctx, &image);

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

            ctx.state.image_loaded = true;
            image.needs_update = true;
            image.needs_reload = false;

        }

        if(image.needs_update) {
            apply_efx(&image, &ctx.efx);
            SDL_UpdateTexture(image.texture, NULL, image.framebuffer, image.width * 3);
            image.needs_update = false;
        }
        if(ctx.state.needs_update) {
            update(&ctx, &image);
            ctx.state.needs_update = false;
        }

        render(&ctx, &image);
        SDL_Delay(10);
    }
   
exit:
    if(ctx.sdl.window)   SDL_DestroyWindow(ctx.sdl.window);
    if(ctx.sdl.renderer) SDL_DestroyRenderer(ctx.sdl.renderer);
    if(image.texture)    SDL_DestroyTexture(image.texture);
    cleanup_gui();
    SDL_Quit();

    if(image.path)        free(image.path);
    if(image.framebuffer) free(image.framebuffer);
    if(image.original)    stbi_image_free(image.original);
    if(image.scratch)     free(image.scratch);

    return 0;
}

int init_sdl(AppContext *ctx) {
    if(!SDL_Init(SDL_INIT_VIDEO)) { return 1; }

    ctx->sdl.win_width = 1600;
    ctx->sdl.win_height = 900;
    
    ctx->sdl.window = SDL_CreateWindow("PPM EFX", ctx->sdl.win_width, ctx->sdl.win_height, SDL_WINDOW_RESIZABLE);
    if(ctx->sdl.window == NULL) { return 1; }
    SDL_SetWindowMinimumSize(ctx->sdl.window, 1600, 900);

    ctx->sdl.renderer = SDL_CreateRenderer(ctx->sdl.window, NULL);
    if(ctx->sdl.renderer == NULL) { return 1; }

    ctx->sdl.image_vp = (SDL_Rect){MARGIN_LEFT, MARGIN_Y / 2, ctx->sdl.win_width - MARGIN_RIGHT, ctx->sdl.win_height - MARGIN_Y};

    return 0;
}

int create_image_texture(SDLContext *sdl, Image *image) {
    image->texture = SDL_CreateTexture(sdl->renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, image->width, image->height);
    if(image->texture == NULL) { return 1; }
    SDL_SetTextureScaleMode(image->texture, SDL_SCALEMODE_NEAREST);
    image->texture_rect = (SDL_FRect){
        (sdl->image_vp.x + sdl->image_vp.w / 2) - (image->width / 2),
        (sdl->image_vp.y + sdl->image_vp.h / 2) - (image->height / 2),
        image->width,
        image->height
    };
    return 0;
}

void update(AppContext *ctx, Image *image) {
    ctx->sdl.image_vp.w = ctx->sdl.win_width - MARGIN_LEFT - MARGIN_RIGHT;
    ctx->sdl.image_vp.h = ctx->sdl.win_height - MARGIN_Y;

    image->texture_rect.w = image->width  * ctx->usr.scale;
    image->texture_rect.h = image->height * ctx->usr.scale;

    float img_max_x = (ctx->sdl.image_vp.x + ctx->sdl.image_vp.w) - image->texture_rect.w / 2;
    float img_min_x = ctx->sdl.image_vp.x - image->texture_rect.w / 2;
    float img_max_y = (ctx->sdl.image_vp.y + ctx->sdl.image_vp.h) - image->texture_rect.h / 2;
    float img_min_y = ctx->sdl.image_vp.y - image->texture_rect.h / 2;

    image->texture_rect.x = clampf(image->texture_rect.x, img_min_x, img_max_x);
    image->texture_rect.y = clampf(image->texture_rect.y, img_min_y, img_max_y);
}

void recenter_image(AppContext *ctx, Image *image) {
    ctx->sdl.image_vp.w = ctx->sdl.win_width - MARGIN_LEFT - MARGIN_RIGHT;
    ctx->sdl.image_vp.h = ctx->sdl.win_height - MARGIN_Y;
    image->texture_rect.w = image->width  * ctx->usr.scale;
    image->texture_rect.h = image->height * ctx->usr.scale;
    image->texture_rect.x = (ctx->sdl.image_vp.x + ctx->sdl.image_vp.w / 2) - (image->texture_rect.w / 2);
    image->texture_rect.y = (ctx->sdl.image_vp.y + ctx->sdl.image_vp.h / 2) - (image->texture_rect.h / 2);
}

void render(AppContext *ctx, Image *image) {
    SDL_SetRenderDrawColor(ctx->sdl.renderer, 80, 80, 80, 255);
    SDL_RenderClear(ctx->sdl.renderer);
    
    SDL_FRect vp_fill_rect = {ctx->sdl.image_vp.x, ctx->sdl.image_vp.y, ctx->sdl.image_vp.w, ctx->sdl.image_vp.h};
    SDL_SetRenderDrawColor(ctx->sdl.renderer, 60, 60, 60, 255);
    SDL_RenderFillRect(ctx->sdl.renderer, &vp_fill_rect);
    
    render_gui(ctx, image);

    if(ctx->state.image_loaded) {
        SDL_SetRenderClipRect(ctx->sdl.renderer, &ctx->sdl.image_vp);
        SDL_RenderTexture(ctx->sdl.renderer, image->texture, NULL, &image->texture_rect);
        SDL_SetRenderClipRect(ctx->sdl.renderer, NULL);
    }

    SDL_RenderPresent(ctx->sdl.renderer);
}

void process_events(AppContext *ctx, Image *image) {
    SDL_Event event;
    while(SDL_PollEvent(&event) != 0) {
        process_gui_events(&event, ctx);

        switch(event.type) {
            case SDL_EVENT_QUIT:
                ctx->state.running = false;
                break;
            
            case SDL_EVENT_WINDOW_MAXIMIZED:
            case SDL_EVENT_WINDOW_MINIMIZED:
            case SDL_EVENT_WINDOW_ENTER_FULLSCREEN:
            case SDL_EVENT_WINDOW_LEAVE_FULLSCREEN:
            case SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED:
            case SDL_EVENT_WINDOW_RESIZED:
                SDL_GetWindowSize(ctx->sdl.window, &ctx->sdl.win_width, &ctx->sdl.win_height);
                recenter_image(ctx, image);
                ctx->state.needs_update = true;
                break;

            case SDL_EVENT_MOUSE_BUTTON_DOWN:
                if(event.button.button == SDL_BUTTON_LEFT) {
                    SDL_Point mouse_pos = {ctx->usr.mx, ctx->usr.my};

                    SDL_Rect image_box = {
                        image->texture_rect.x,
                        image->texture_rect.y,
                        image->texture_rect.w,
                        image->texture_rect.h
                    };

                    if(SDL_PointInRect(&mouse_pos, &image_box) && SDL_PointInRect(&mouse_pos, &ctx->sdl.image_vp)) {
                        ctx->usr.panning = true;
                    }
                }
                break;

            case SDL_EVENT_MOUSE_BUTTON_UP:
                if(event.button.button == SDL_BUTTON_LEFT) {
                    ctx->usr.panning = false;
                }
                break;

            case SDL_EVENT_MOUSE_MOTION: {
                if(ctx->usr.panning) {
                    image->texture_rect.x += event.motion.xrel;
                    image->texture_rect.y += event.motion.yrel;
                    ctx->state.needs_update = true;
                }
                break;
            }

            case SDL_EVENT_MOUSE_WHEEL: {
                float old_scale = ctx->usr.scale;

                ctx->usr.scale += event.wheel.y * ctx->usr.scale / 10;
                ctx->usr.scale = clampf(ctx->usr.scale, 0.1, 100);

                float scale_ratio = ctx->usr.scale / old_scale;

                image->texture_rect.x = ctx->usr.mx - (ctx->usr.mx - image->texture_rect.x) * scale_ratio;
                image->texture_rect.y = ctx->usr.my - (ctx->usr.my - image->texture_rect.y) * scale_ratio;

                ctx->state.needs_update = true;
                break;
            }

            case SDL_EVENT_KEY_DOWN:
                switch(event.key.key) {
                    case SDLK_MINUS:
                        ctx->usr.scale -= 0.1;
                        break;

                    case SDLK_PLUS:
                        ctx->usr.scale += 0.1;
                        break;

                    case SDLK_ESCAPE:
                        ctx->usr.scale = 1.0;
                        recenter_image(ctx, image);
                        image->needs_update = true;
                        break;
                }
        }
    }
}
