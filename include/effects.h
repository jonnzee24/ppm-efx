#ifndef EFFECTS_H
#define EFFECTS_H

#include <stdint.h>
#include <stdbool.h>
#include <SDL3/SDL.h>

typedef struct Image Image;

typedef enum {
    INVERT,
    SATURATION,
    CONTRAST,
    EXPOSURE,
    MONO,
    THRESHOLD,
    QUANTIZE,
    COLOR_BIAS,
    COLOR_SHIFT,
    WARP,
    DITHER,
    BLUR,

    NUM_EFX
} EfxID;

typedef struct Efx {
    EfxID id;
    char *name;

    int num_params;
    float params[4];
    char *param_names[4];
    int param_types[4]; // O is for sliders and 1 is for buttons
    
    void (*efx_func)(Image *image, float params[]);
} Efx;

typedef struct EfxPipeline {
    Efx efx_list[NUM_EFX];
    Efx *pipeline[NUM_EFX];
    int count;
} EfxPipeline;

int init_efx(Efx efx_list[]);
void append_to_pipeline(EfxPipeline *efx, EfxID id);
void remove_from_pipeline(EfxPipeline *efx, EfxID id);
void swap_in_pipeline(EfxPipeline *efx, EfxID id_1, EfxID id_2);
void clear_pipeline(EfxPipeline *efx);
bool check_in_pipeline(EfxPipeline *efx, EfxID id);
void apply_efx(Image *image, EfxPipeline *efx);

#endif
