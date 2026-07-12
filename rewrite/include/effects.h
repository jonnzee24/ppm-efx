#ifndef EFFECTS_H
#define EFFECTS_H

#include <stdint.h>

#include "common.h"

void warp(Image *image, int warp_mode, float sine_length, float sine_amp);
void dither(Image *image, float brightness);
void pixelate(Image *image, int pixel_size);
void invert(int *r, int *g, int *b);
void mono(int *r, int *g, int *b, bool do_threshold, int threshold);
void quantize(int *r, int *g, int *b, int bit_depth);
void exposure(int *r, int *g, int *b, float exposure_val);
void contrast(int *r, int *g, int *b, int contrast_val);
void saturation(int *r, int *g, int *b, float saturation_val);
void color_bias(int *r, int *g, int *b, int bias);
void color_shift(int *r, int *g, int *b, float shift);

void apply_efx(Image *image, EffectFlags *effects, EffectParams *params);

#endif
