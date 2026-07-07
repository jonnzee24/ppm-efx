#ifndef EFFECTS_H
#define EFFECTS_H

#include <SDL.h>
#include <stdbool.h>

int clamp(int value, int min, int max);
float clampf(float value, float min, float max);

typedef struct {
    bool warp;
    bool invert;
    bool mono;
    bool quantize;
    bool dither;
    bool shift;
    bool exposure;
    bool contrast;
    bool saturation;
    bool color;
    bool pixelate;
} EffectFlags;

typedef struct {
    int bit_depth;
    float dither_brightness;
    bool mono_do_thresh;
    int mono_thresh;
    int warp_mode;
    float sine_length;
    float sine_amp;
    float color_shift;
    float exposure_val;
    int contrast_val;
    float saturation_val;
    int color_bias;
    int pixel_size;
} EffectParams;

void warp(Uint8 *framebuffer, int width, int height, int option, float sine_lenght, float amplitude);
void invert(int *r, int *g, int *b);
void monochrome(int *r, int *g, int *b, bool do_threshold, int threshold);
void quantize(int *r, int *g, int *b, int bit_depth);
void dither(Uint8 *framebuffer, size_t framebuffer_size, int width, int height, float brightness);
void shift(int *r, int *g, int *b, float color_shift);
void exposure(int *r, int *g, int *b, float exposure_val);
void contrast(int *r, int *g, int *b, int contrast_val);
void saturation(int *r, int *g, int *b, float saturation_val);
void colorb(int *r, int *g, int *b, int color_bias);
void blur(Uint8 *framebuffer, int width, int height);
void pixelate(Uint8 *framebuffer, int width, int height, int pixel_size);

#endif
