#include "SDL3/SDL.h"
#include "common.h"
#include "image.h"
#include "utils.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"

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

int scale_image(Image *image, int new_w, int new_h) {
    uint8_t *new_data = stbir_resize_uint8_linear(
            image->original, image->org_w, image->org_h,
            0, NULL, new_w, new_h, 0, STBIR_RGB
    );

    if(new_data == NULL) { return 1; }

    size_t new_size = (size_t)new_w * new_h * 3;
    uint8_t *new_fb = malloc(new_size);
    uint8_t *new_scratch = malloc(new_size);
    if(new_fb == NULL || new_scratch == NULL) {
        free(new_data);
        free(new_fb);
        free(new_scratch);
        return 1;
    }

    free(image->original);
    free(image->framebuffer);
    free(image->scratch);

    image->original = new_data;
    image->framebuffer = new_fb;
    image->scratch = new_scratch;
    image->org_w = image->width = new_w;
    image->org_h = image->height = new_h;
    image->framebuffer_size = new_size;
    memcpy(image->framebuffer, image->original, new_size);

    image->needs_reload = true;

    return 0;
}

void *scale_image_callback(void *data) {
    ImageScaleData *scale_data = (ImageScaleData *)data;
    Image *image = scale_data->image;

    float slider_val = scale_data->multiplier;
    float multiplier;

    if (slider_val >= 0.5f) {
        multiplier = 1.0f + (slider_val - 0.5f) * 8.0f;
    } else {
        multiplier = slider_val * 2.0f;
    }

    int new_w = (float)image->width * multiplier;
    int new_h = (float)image->height * multiplier;

    new_w = clamp(new_w, 1, 4000);
    new_h = clamp(new_h, 1, 4000);

    scale_image(image, new_w, new_h);
    return NULL;
}
