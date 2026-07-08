#ifndef EFFECTS_H
#define EFFECTS_H

#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>

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
    bool color_bias;
    bool pixelate;
} EffectFlags;

typedef struct {
    int warp_mode;
    float sine_length;
    float sine_amp;
    bool mono_do_thresh;
    int mono_thresh;
    int bit_depth;
    float dither_brightness;
    float color_shift;
    float exposure_val;
    int contrast_val;
    float saturation_val;
    int color_bias;
    int pixel_size;
} EffectParams;

void warp(uint8_t *framebuffer, uint8_t *original, int width, int height, int option, float sine_lenght, float sine_amp);
void invert(int *r, int *g, int *b);
void mono(int *r, int *g, int *b, bool do_threshold, int threshold);
void quantize(int *r, int *g, int *b, int bit_depth);
void dither(uint8_t *framebuffer, int width, int height, float brightness);
void shift(int *r, int *g, int *b, float shift);
void exposure(int *r, int *g, int *b, float exposure_val);
void contrast(int *r, int *g, int *b, int contrast_val);
void saturation(int *r, int *g, int *b, float saturation_val);
void color_bias(int *r, int *g, int *b, int bias);
void pixelate(uint8_t *framebuffer, int width, int height, int pixel_size);

#endif
