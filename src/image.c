#include "SDL3/SDL.h"
#include "common.h"
#include "image.h"

int create_image_texture(SDLContext *sdl, Image *image) {
    image->texture = SDL_CreateTexture(
            sdl->renderer,
            SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING,
            image->width, image->height);
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

void recenter_image(AppContext *ctx, Image *image) {
    ctx->sdl.image_vp.w = ctx->sdl.win_width - MARGIN_LEFT - MARGIN_RIGHT;
    ctx->sdl.image_vp.h = ctx->sdl.win_height - MARGIN_Y;
    image->texture_rect.w = image->width  * ctx->usr.scale;
    image->texture_rect.h = image->height * ctx->usr.scale;
    image->texture_rect.x = (ctx->sdl.image_vp.x + ctx->sdl.image_vp.w / 2) - (image->texture_rect.w / 2);
    image->texture_rect.y = (ctx->sdl.image_vp.y + ctx->sdl.image_vp.h / 2) - (image->texture_rect.h / 2);
}

/*
void *scale_image(void *data) {
    Image *image = (Image *)data;
}
*/
