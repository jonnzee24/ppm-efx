#ifndef EFFECTS_H
#define EFFECTS_H

#include <SDL.h>
#include <stdbool.h>

int clamp(int value, int min, int max);
float clampf(float value, float min, float max);

void warp(Uint8 *framebuffer, int width, int height, int option);
void invert(int *r, int *g, int *b);
void monochrome(int *r, int *g, int *b, bool do_threshold, int threshold);
void quantize(int *r, int *g, int *b, int bit_depth);
void dither(Uint8 *framebuffer, size_t framebuffer_size, int width, int height, int thresh, float brightness);
void shift(int *r, int *g, int *b, int color_shift);
void exposure(int *r, int *g, int *b, float exposure_val);

#endif
