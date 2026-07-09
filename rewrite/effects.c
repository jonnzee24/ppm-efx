#include <math.h>

#include "common.h"
#include "effects.h"

void warp(Image *image, int warp_mode, float sine_lenght, float sine_amp) {
    for(int y = 0; y < image->height; y++) {
        float sine_offset = 0;
        if(warp_mode == 4) {
            sine_offset = sin((2.0f * M_PI * y) / sine_lenght) * sine_amp;
        }

        for(int x = 0; x < image->width; x++) {
            int x_pos = x;
            int y_pos = y;

            if(x > image->width / 2 && warp_mode == MIRROR_X) {
                x_pos = image->width - x;
                y_pos = y;
            } 
            if(y > image->height / 2 && warp_mode == MIRROR_Y) {
                y_pos = image->height - y;
            } 
            if(warp_mode == UPSIDE_DOWN) {
                x_pos = image->width - x;
                y_pos = image->height - y;
            } 
            if(warp_mode == SINE) {
                x_pos += sine_offset;
            }

            x_pos = clamp(x_pos, 0, image->width - 1);
            y_pos = clamp(y_pos, 0, image->height - 1);

            int old_pos = (y * image->width + x) * 3;
            int new_pos = (y_pos * image->width + x_pos) * 3;

            image->framebuffer[old_pos    ] = image->original[new_pos    ];
            image->framebuffer[old_pos + 1] = image->original[new_pos + 1];
            image->framebuffer[old_pos + 2] = image->original[new_pos + 2];
        }
    }
}

void invert(int *r, int *g, int *b) {
    *r = 255 - *r;
    *g = 255 - *g;
    *b = 255 - *b;
}

void mono(int *r, int *g, int *b, bool do_threshold, int threshold) {
    uint8_t luminance = (*r + *g + *b) / 3;
    
    *r = luminance;
    *g = luminance;
    *b = luminance;

    if(do_threshold) {
        *r = *r > threshold ? 255 : 0;
        *g = *g > threshold ? 255 : 0;
        *b = *b > threshold ? 255 : 0;
    }
}

void quantize(int *r, int *g, int *b, int bit_depth) {
    int levels = 1 << bit_depth;
    int colors_amount = 256 / levels;

    *r = ((*r / colors_amount) * 255) / (levels - 1);
    *g = ((*g / colors_amount) * 255) / (levels - 1);
    *b = ((*b / colors_amount) * 255) / (levels - 1);
}

void dither(Image *image, float brightness) {
    uint8_t *fb = image->framebuffer;
    const int stride = image->width * 3;

    for(int y = 0; y < image->height; y++) {
        for(int x = 0; x < image->width; x++) {

            int pixel_pos = (y * image->width + x) * 3;

            fb[pixel_pos    ] = fb[pixel_pos    ] * brightness;
            fb[pixel_pos + 1] = fb[pixel_pos + 1] * brightness;
            fb[pixel_pos + 2] = fb[pixel_pos + 2] * brightness;

            int op_r = fb[pixel_pos    ];
            int op_g = fb[pixel_pos + 1];
            int op_b = fb[pixel_pos + 2];

            int qp_r = op_r;
            int qp_g = op_g;
            int qp_b = op_b;

            mono(&qp_r, &qp_g, &qp_b, true, 120);

            fb[pixel_pos    ] = qp_r;
            fb[pixel_pos + 1] = qp_g;
            fb[pixel_pos + 2] = qp_b;

            int error_r = op_r - qp_r;
            int error_g = op_g - qp_g;
            int error_b = op_b - qp_b;
            
            for(int i = 0; i < 3; i++) {
                int error;
                switch(i) {
                    case 0:
                        error = error_r;
                        break;
                    case 1:
                        error = error_g;
                        break;
                    case 2:
                        error = error_b;
                        break;
                }
                
                if(x != image->width - 1) {
                    fb[(pixel_pos + i + 3)] = clamp((fb[(pixel_pos + i + 3)] + (error * 7) / 16), 0, 255);
                }
                if(x != 0 && y != image->height - 1) {
                    fb[(pixel_pos + i - 3) + stride] = clamp((fb[(pixel_pos + i - 3) + stride] + (error * 3) / 16), 0, 255);
                }
                if(y != image->height - 1) {
                    fb[(pixel_pos + i) + stride] = clamp((fb[(pixel_pos + i) + stride] + (error * 5) / 16), 0, 255);
                }
                if(y != image->height - 1 && x != image->width - 1) {
                    fb[(pixel_pos + i + 3) + stride] = clamp((fb[(pixel_pos + i + 3) + stride] + (error * 1) / 16), 0, 255);
                }
            }
        }
    }
}

void color_shift(int *r, int *g, int *b, float shift) {
    uint8_t rb = *r;
    uint8_t gb = *g;
    uint8_t bb = *b;

    if(shift <= 0.5) {
        *r = (shift * bb) + (1 - shift) * rb;
        *g = (shift * rb) + (1 - shift) * gb;
        *b = (shift * gb) + (1 - shift) * bb;
    } else if(shift > 0.5) {
        *r = (shift * gb) + (1 - shift) * rb;
        *g = (shift * bb) + (1 - shift) * gb;
        *b = (shift * rb) + (1 - shift) * bb;
    }
}

void exposure(int *r, int *g, int *b, float exposure_val) {
    *r = clamp(*r * exposure_val, 0, 255);
    *g = clamp(*g * exposure_val, 0, 255);
    *b = clamp(*b * exposure_val, 0, 255);
}

void contrast(int *r, int *g, int *b, int contrast_val) {
    int pixel_val = (*r + *g + *b) / 3;

    if(pixel_val <= 100) {
        *r = clamp(*r - contrast_val, 0, 255);
        *g = clamp(*g - contrast_val, 0, 255);
        *b = clamp(*b - contrast_val, 0, 255);
    } else if(pixel_val > 140) {
        *r = clamp(*r + contrast_val, 0, 255);
        *g = clamp(*g + contrast_val, 0, 255);
        *b = clamp(*b + contrast_val, 0, 255);
    }
}

void saturation(int *r, int *g, int *b, float saturation_val) {
    int gray = (*r + *g + *b) / 3;

    *r = clamp(gray + (int)((*r - gray) * saturation_val), 0, 255);
    *g = clamp(gray + (int)((*g - gray) * saturation_val), 0, 255);
    *b = clamp(gray + (int)((*b - gray) * saturation_val), 0, 255);
}

void color_bias(int *r, int *g, int *b, int bias) {
    if(bias == 0) {
        *g = *g / 3;
        *b = *b / 3;
    } else if(bias == 1) {
        *r = *r / 3;
        *b = *b / 3;
    } else if(bias == 2) {
        *r = *r / 3;
        *g = *g / 3;
    }
}

void pixelate(Image *image, int pixel_size) {
    for(int y = 0; y < image->height; y += pixel_size) {
        for(int x = 0; x < image->width; x += pixel_size) {
            int pixel_pos = (y * image->width + x) * 3;

            int r = image->framebuffer[pixel_pos];
            int g = image->framebuffer[pixel_pos + 1];
            int b = image->framebuffer[pixel_pos + 2];

            for(int ph = 0; ph < pixel_size && (ph + y) < image->height; ph++) {
                for(int pw = 0; pw < pixel_size && (pw + x) < image->width; pw++) {
                    int pixel_pos_2 = pixel_pos + (ph * image->width + pw) * 3;
                    if(pixel_pos_2 < image->width * image->height * 3) {
                        image->framebuffer[pixel_pos_2] = r;
                        image->framebuffer[pixel_pos_2 + 1] = g;
                        image->framebuffer[pixel_pos_2 + 2] = b;
                    }
                }
            }
        }
    }
}

void apply_efx(Image *image, const EffectFlags *efx, const EffectParams *params) {
    memcpy(image->framebuffer, image->original, image->framebuffer_size);

    if(efx->warp) {
        warp(image, params->warp_mode, params->sine_length, params->sine_amp);
    }
    if(efx->pixelate) {
        pixelate(image, params->pixel_size);
    }
    if (efx->dither) {
        dither(image, params->dither_brightness);
    }


    for (int y = 0; y < image->height; y++) { 
        for (int x = 0; x < image->width; x++) {
            int pixel_pos = (y * image->width + x) * 3;

            int r = image->framebuffer[pixel_pos    ];
            int g = image->framebuffer[pixel_pos + 1];
            int b = image->framebuffer[pixel_pos + 2];

            if(efx->invert)     { invert(&r, &g, &b); }
            if(efx->mono)       { mono(&r, &g, &b, params->mono_do_thresh, params->mono_thresh); }
            if(efx->quantize)   { quantize(&r, &g, &b, params->bit_depth); }
            if(efx->exposure)   { exposure(&r, &g, &b, params->exposure_val); }
            if(efx->contrast)   { contrast(&r, &g, &b, params->contrast_val); }
            if(efx->saturation) { saturation(&r, &g, &b, params->saturation_val); }
            if(efx->color_bias)      { color_bias(&r, &g, &b, params->color_bias); }
            if(efx->color_shift)      { color_shift(&r, &g, &b, params->color_shift); }

            image->framebuffer[pixel_pos    ] = (Uint8)r;
            image->framebuffer[pixel_pos + 1] = (Uint8)g;
            image->framebuffer[pixel_pos + 2] = (Uint8)b;
        }
    }
}
