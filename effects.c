#include <math.h>

#include "common.h"
#include "effects.h"

void invert(int *r, int *g, int *b) {
    *r = 255 - *r;
    *g = 255 - *g;
    *b = 255 - *b;
}

void mono(int *r, int *g, int *b) {
    uint8_t luminance = (*r + *g + *b) / 3;
    *r = *g = *b = luminance;
}

void threshold(int *r, int *g, int *b, int threshold, int threshold_mode) {
    uint8_t luminance = (*r + *g + *b) / 3;

    if(threshold_mode == 0) {
        *r = *g = *b = luminance < threshold ? 0 : 255;
    }
    else if(threshold_mode == 1) {
        *r = luminance < threshold ? 0 : *r;
        *g = luminance < threshold ? 0 : *g;
        *b = luminance < threshold ? 0 : *b;

    }
    else if(threshold_mode == 2) {
        *r = luminance < threshold ? *r : 255;
        *g = luminance < threshold ? *g : 255;
        *b = luminance < threshold ? *b : 255;
    }
}

void quantize(int *r, int *g, int *b, int bit_depth) {
    int levels = 1 << bit_depth; // 2 to the power of bit depth.
                                 // n bits can store 2 to the power of n values.

    int bucket_size = 256 / levels; // how many values are in each level

    *r = ((*r / bucket_size) * 255) / (levels - 1);
    *g = ((*g / bucket_size) * 255) / (levels - 1);
    *b = ((*b / bucket_size) * 255) / (levels - 1);
}

void warp(Image *image, int warp_mode, float sine_length, float sine_amp) {
    for(int y = 0; y < image->height; y++) {
        float sine_offset = 0;
        if(warp_mode == SINE) {
            sine_offset = sin((2.0f * M_PI * y) / sine_length) * sine_amp;
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

void dither(Image *image, float brightness, int dither_mode, int bit_depth, int threshold_val) {
    uint8_t *fb = image->framebuffer;
    const int stride = image->width * 3;
    threshold_val = clamp(threshold_val, 40, 200);

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

            if(dither_mode == 0) { threshold(&qp_r, &qp_g, &qp_b, threshold_val, 0); }
            else if(dither_mode == 1) { quantize(&qp_r, &qp_g, &qp_b, bit_depth); }
            else if(dither_mode == 2) { threshold(&qp_r, &qp_g, &qp_b, threshold_val, 1); }

            fb[pixel_pos    ] = qp_r;
            fb[pixel_pos + 1] = qp_g;
            fb[pixel_pos + 2] = qp_b;

            int error_r = op_r - qp_r;
            int error_g = op_g - qp_g;
            int error_b = op_b - qp_b;

            int errors[3] = {error_r, error_g, error_b};
            
            for(int i = 0; i < 3; i++) {
                int error = errors[i];
                
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
    float factor = 259.0f * ((contrast_val + 255)) / (255 * (259 - contrast_val));

    *r = clamp((factor * (*r - 128) + 128), 0, 255);
    *g = clamp((factor * (*g - 128) + 128), 0, 255);
    *b = clamp((factor * (*b - 128) + 128), 0, 255);
}

void saturation(int *r, int *g, int *b, float saturation_val) {
    int gray = (*r + *g + *b) / 3;

    *r = clamp(gray + (*r - gray) * saturation_val, 0, 255);
    *g = clamp(gray + (*g - gray) * saturation_val, 0, 255);
    *b = clamp(gray + (*b - gray) * saturation_val, 0, 255);
}

void color_bias(int *r, int *g, int *b, int bias) {
    if(bias == R) {
        *g = *g / 3;
        *b = *b / 3;
    } else if(bias == G) {
        *r = *r / 3;
        *b = *b / 3;
    } else if(bias == B) {
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

void apply_efx(Image *image, EffectFlags *efx, EffectParams *params) {
    memcpy(image->framebuffer, image->original, image->framebuffer_size);

    int pixel_size = (int)(1 + params->pixel_size * 20);
    int bit_depth = (int)(1 + params->bit_depth * 7);
    float sine_length = (10 + params->sine_length * 400.0f);
    float sine_amp = (params->sine_amp * 200.0f);
    int threshold_val = (int)(params->threshold_val * 255);
    float exposure_val = (params->exposure_val * 3.0f);
    int contrast_val = (int)(((params->contrast_val * 2) - 0.5) * 170);
    float saturation_val = (params->saturation_val * 5.0f);
    float dither_brightness = (0.2f + params->dither_brightness);

    for (int y = 0; y < image->height; y++) { 
        for (int x = 0; x < image->width; x++) {
            int pixel_pos = (y * image->width + x) * 3;

            int r = image->framebuffer[pixel_pos    ];
            int g = image->framebuffer[pixel_pos + 1];
            int b = image->framebuffer[pixel_pos + 2];

            if(efx->mono)        { mono(&r, &g, &b); }
            if(efx->threshold)   { threshold(&r, &g, &b, threshold_val, params->threshold_mode); }
            if(efx->quantize)    { quantize(&r, &g, &b, bit_depth); }
            if(efx->exposure)    { exposure(&r, &g, &b, exposure_val); }
            if(efx->contrast)    { contrast(&r, &g, &b, contrast_val); }
            if(efx->saturation)  { saturation(&r, &g, &b, saturation_val); }
            if(efx->color_bias)  { color_bias(&r, &g, &b, params->color_bias); }
            if(efx->color_shift) { color_shift(&r, &g, &b, params->color_shift_val); }
            if(efx->invert)      { invert(&r, &g, &b); }


            image->framebuffer[pixel_pos    ] = (Uint8)r;
            image->framebuffer[pixel_pos + 1] = (Uint8)g;
            image->framebuffer[pixel_pos + 2] = (Uint8)b;
        }
    }

    if(efx->warp) {
        warp(image, params->warp_mode, sine_length, sine_amp);
    }
    if(efx->pixelate) {
        pixelate(image, pixel_size);
    }
    if (efx->dither) {
        dither(image, dither_brightness, params->dither_mode, bit_depth, threshold_val);
    }
}
