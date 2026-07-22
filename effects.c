#include <math.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

#include "common.h"
#include "effects.h"

void invert(Image *image, float params[], float x_split, float y_split) {
    (void)params;

    int max_x = (int)(image->width * x_split);
    int max_y = (int)(image->height * y_split);

    uint8_t *fb = image->framebuffer;
    
    for(int y = 0; y < max_y; y++) {
        for(int x = 0; x < max_x; x++) {
            int pixel_pos = (y * image->width + x) * 3;

            uint8_t r = fb[pixel_pos    ];
            uint8_t g = fb[pixel_pos + 1];
            uint8_t b = fb[pixel_pos + 2];

            r = 255 - r;
            g = 255 - g;
            b = 255 - b;

            fb[pixel_pos    ] = r;
            fb[pixel_pos + 1] = g;
            fb[pixel_pos + 2] = b;
        }
    }
}

void mono(Image *image, float params[], float x_split, float y_split) {
    (void)params;

    int max_x = (int)(image->width * x_split);
    int max_y = (int)(image->height * y_split);

    uint8_t *fb = image->framebuffer;
    
    for(int y = 0; y < max_y; y++) {
        for(int x = 0; x < max_x; x++) {
            int pixel_pos = (y * image->width + x) * 3;

            uint8_t r = fb[pixel_pos    ];
            uint8_t g = fb[pixel_pos + 1];
            uint8_t b = fb[pixel_pos + 2];

            uint8_t luminance = (r + g + b) / 3;
            r = g = b = luminance;

            fb[pixel_pos    ] = r;
            fb[pixel_pos + 1] = g;
            fb[pixel_pos + 2] = b;
        }
    }
}

void threshold(Image *image, float params[], float x_split, float y_split) {
    int max_x = (int)(image->width * x_split);
    int max_y = (int)(image->height * y_split);

    int threshold_mode = (int)(params[0] * 2);
    int threshold_val = (int)(20 + params[1] * 200);

    uint8_t *fb = image->framebuffer;

    for(int y = 0; y < max_y; y++) {
        for(int x = 0; x < max_x; x++) {
            int pixel_pos = (y * image->width + x) * 3;

            uint8_t r = fb[pixel_pos    ];
            uint8_t g = fb[pixel_pos + 1];
            uint8_t b = fb[pixel_pos + 2];

            uint8_t luminance = (r + g + b) / 3;

            if(threshold_mode == 0) {
                r = g = b = luminance < threshold_val ? 0 : 255;
            }
            else if(threshold_mode == 1) {
                r = luminance < threshold_val ? 0 : r;
                g = luminance < threshold_val ? 0 : g;
                b = luminance < threshold_val ? 0 : b;
            }
            else if(threshold_mode == 2) {
                r = luminance < threshold_val ? r : 0;
                g = luminance < threshold_val ? g : 0;
                b = luminance < threshold_val ? b : 0;
            }

            fb[pixel_pos    ] = r;
            fb[pixel_pos + 1] = g;
            fb[pixel_pos + 2] = b;
        }
    }
}

void quantize(Image *image, float params[], float x_split, float y_split) {
    int max_x = (int)(image->width * x_split);
    int max_y = (int)(image->height * y_split);
    
    int bit_depth = (int)(1 + params[0] * 7);
    int levels = 1 << bit_depth;
    int bucket_size = 256 / levels;

    uint8_t *fb = image->framebuffer;

    for(int y = 0; y < max_y; y++) {
        for(int x = 0; x < max_x; x++) {
            int pixel_pos = (y * image->width + x) * 3;

            uint8_t r = fb[pixel_pos    ];
            uint8_t g = fb[pixel_pos + 1];
            uint8_t b = fb[pixel_pos + 2];

            r = ((r / bucket_size) * 255) / (levels - 1);
            g = ((g / bucket_size) * 255) / (levels - 1);
            b = ((b / bucket_size) * 255) / (levels - 1);

            fb[pixel_pos    ] = r;
            fb[pixel_pos + 1] = g;
            fb[pixel_pos + 2] = b;
        }
    }
}

void warp(Image *image, float params[], float x_split, float y_split) {
    int max_x = (int)(image->width * x_split);
    int max_y = (int)(image->height * y_split);

    int warp_mode = (int)(params[0] * 3);
    float sine_length = (10 + params[1] * 400.0f);
    float sine_amp = (params[2] * 200.0f);

    memcpy(image->scratch, image->framebuffer, image->framebuffer_size);

    for(int y = 0; y < max_y; y++) {
        float sine_offset = 0;
        if(warp_mode == 3) {
            sine_offset = sin((2.0f * M_PI * y) / sine_length) * sine_amp;
        }

        for(int x = 0; x < max_x; x++) {
            int x_pos = x;
            int y_pos = y;

            if(x > image->width / 2 && warp_mode == 0) {
                x_pos = image->width - x;
                y_pos = y;
            } 
            if(y > image->height / 2 && warp_mode == 1) {
                y_pos = image->height - y;
            } 
            if(warp_mode == 2) {
                x_pos = image->width - x;
                y_pos = image->height - y;
            } 
            if(warp_mode == 3) {
                x_pos += sine_offset;
            }

            x_pos = clamp(x_pos, 0, image->width - 1);
            y_pos = clamp(y_pos, 0, image->height - 1);

            int old_pos = (y * image->width + x) * 3;
            int new_pos = (y_pos * image->width + x_pos) * 3;

            image->framebuffer[old_pos    ] = image->scratch[new_pos    ];
            image->framebuffer[old_pos + 1] = image->scratch[new_pos + 1];
            image->framebuffer[old_pos + 2] = image->scratch[new_pos + 2];
        }
    }
}

void dither(Image *image, float params[], float x_split, float y_split) {
    int max_x = (int)(image->width * x_split);
    int max_y = (int)(image->height * y_split);

    int dither_mode = (int)(params[0] * 3);
    int threshold = (int)(40 + params[1] * 160);
    int bit_depth = (int)(1 + 7 * params[2]);

    uint8_t *fb = image->framebuffer;
    int stride = image->width * 3;

    for(int y = 0; y < max_y; y++) {
        for(int x = 0; x < max_x; x++) {
            int pixel_pos = (y * image->width + x) * 3;

            int op_r = fb[pixel_pos    ];
            int op_g = fb[pixel_pos + 1];
            int op_b = fb[pixel_pos + 2];

            int qp_r = 0, qp_g = 0, qp_b = 0;

            if(dither_mode == 0 || dither_mode == 3) {
                uint8_t luminance = (op_r + op_g + op_b) / 3;
                qp_r = qp_g = qp_b = luminance < threshold ? 0 : 255;
            }
            else if(dither_mode == 1) {
                int levels = 1 << bit_depth;
                int bucket_size = 256 / levels;
                qp_r = ((op_r / bucket_size) * 255) / (levels - 1);
                qp_g = ((op_g / bucket_size) * 255) / (levels - 1);
                qp_b = ((op_b / bucket_size) * 255) / (levels - 1);
            }
            else if(dither_mode == 2) {
                int levels = 1 << bit_depth;
                int bucket_size = 256 / levels;
                qp_r = ((op_r / bucket_size) * 255) / (levels - 1);
                qp_g = ((op_g / bucket_size) * 255) / (levels - 1);
                qp_b = ((op_b / bucket_size) * 255) / (levels - 1);

                uint8_t luminance = (qp_r + qp_g + qp_b) / 3;
                qp_r = luminance < threshold ? 0 : qp_r;
                qp_g = luminance < threshold ? 0 : qp_g;
                qp_b = luminance < threshold ? 0 : qp_b;
            }

            if(dither_mode != 3) {
                fb[pixel_pos    ] = qp_r;
                fb[pixel_pos + 1] = qp_g;
                fb[pixel_pos + 2] = qp_b;
            }

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

void color_shift(Image *image, float params[], float x_split, float y_split) {
    int max_x = (int)(image->width * x_split);
    int max_y = (int)(image->height * y_split);

    float shift = params[0];

    uint8_t *fb = image->framebuffer;
    
    for(int y = 0; y < max_y; y++) {
        for(int x = 0; x < max_x; x++) {
            int pixel_pos = (y * image->width + x) * 3;

            uint8_t r = fb[pixel_pos    ];
            uint8_t g = fb[pixel_pos + 1];
            uint8_t b = fb[pixel_pos + 2];

            float rb = (float)r;
            float gb = (float)g;
            float bb = (float)b;

            r = (shift * bb) + (1 - shift) * rb;
            g = (shift * rb) + (1 - shift) * gb;
            b = (shift * gb) + (1 - shift) * bb;

            fb[pixel_pos    ] = r;
            fb[pixel_pos + 1] = g;
            fb[pixel_pos + 2] = b;
        }
    }
}

void exposure(Image *image, float params[], float x_split, float y_split) {
    int max_x = (int)(image->width * x_split);
    int max_y = (int)(image->height * y_split);

    float exposure_val = (0.5f + params[0] * 2.5f);

    uint8_t *fb = image->framebuffer;
    
    for(int y = 0; y < max_y; y++) {
        for(int x = 0; x < max_x; x++) {
            int pixel_pos = (y * image->width + x) * 3;

            uint8_t r = fb[pixel_pos    ];
            uint8_t g = fb[pixel_pos + 1];
            uint8_t b = fb[pixel_pos + 2];

            r = clamp(r * exposure_val, 0, 255);
            g = clamp(g * exposure_val, 0, 255);
            b = clamp(b * exposure_val, 0, 255);

            fb[pixel_pos    ] = r;
            fb[pixel_pos + 1] = g;
            fb[pixel_pos + 2] = b;
        }
    }
}

void contrast(Image *image, float params[], float x_split, float y_split) {
    int max_x = (int)(image->width * x_split);
    int max_y = (int)(image->height * y_split);

    int contrast_val = (int)(((params[0] * 2) - 0.5f) * 170);
    float factor = 259.0f * ((contrast_val + 255)) / (255 * (259 - contrast_val));
    
    uint8_t *fb = image->framebuffer;
    
    for(int y = 0; y < max_y; y++) {
        for(int x = 0; x < max_x; x++) {
            int pixel_pos = (y * image->width + x) * 3;

            uint8_t r = fb[pixel_pos    ];
            uint8_t g = fb[pixel_pos + 1];
            uint8_t b = fb[pixel_pos + 2];

            r = clamp((factor * (r - 128) + 128), 0, 255);
            g = clamp((factor * (g - 128) + 128), 0, 255);
            b = clamp((factor * (b - 128) + 128), 0, 255);

            fb[pixel_pos    ] = r;
            fb[pixel_pos + 1] = g;
            fb[pixel_pos + 2] = b;
        }
    }
}

void saturation(Image *image, float params[], float x_split, float y_split) {
    int max_x = (int)(image->width * x_split);
    int max_y = (int)(image->height * y_split);

    float saturation_val = (params[0] * 5.0f);
        
    uint8_t *fb = image->framebuffer;
    
    for(int y = 0; y < max_y; y++) {
        for(int x = 0; x < max_x; x++) {
            int pixel_pos = (y * image->width + x) * 3;

            uint8_t r = fb[pixel_pos    ];
            uint8_t g = fb[pixel_pos + 1];
            uint8_t b = fb[pixel_pos + 2];

            int gray = (r + g + b) / 3;

            r = clamp(gray + (r - gray) * saturation_val, 0, 255);
            g = clamp(gray + (g - gray) * saturation_val, 0, 255);
            b = clamp(gray + (b - gray) * saturation_val, 0, 255);

            fb[pixel_pos    ] = r;
            fb[pixel_pos + 1] = g;
            fb[pixel_pos + 2] = b;
        }
    }
}

void color_bias(Image *image, float params[], float x_split, float y_split) {
    int max_x = (int)(image->width * x_split);
    int max_y = (int)(image->height * y_split);

    int bias = (int)(params[0] * 2);
 
    uint8_t *fb = image->framebuffer;
    
    for(int y = 0; y < max_y; y++) {
        for(int x = 0; x < max_x; x++) {
            int pixel_pos = (y * image->width + x) * 3;

            uint8_t r = fb[pixel_pos    ];
            uint8_t g = fb[pixel_pos + 1];
            uint8_t b = fb[pixel_pos + 2];

            if(bias == 0) {
                g = g / 3;
                b = b / 3;
            } else if(bias == 1) {
                r = r / 3;
                b = b / 3;
            } else if(bias == 2) {
                r = r / 3;
                g = g / 3;
            }

            fb[pixel_pos    ] = r;
            fb[pixel_pos + 1] = g;
            fb[pixel_pos + 2] = b;
        }
    }
}

void pixelate(Image *image, float params[], float x_split, float y_split) {
    int max_x = (int)(image->width * x_split);
    int max_y = (int)(image->height * y_split);

    int pixel_size = (int)(2 + params[0] * 38);
 
    uint8_t *fb = image->framebuffer;

    for(int y = 0; y < max_y; y += pixel_size) {
        for(int x = 0; x < max_x; x += pixel_size) {
            int pixel_pos = (y * image->width + x) * 3;

            uint8_t r = fb[pixel_pos];
            uint8_t g = fb[pixel_pos + 1];
            uint8_t b = fb[pixel_pos + 2];

            for(int ph = 0; ph < pixel_size && (ph + y) < max_y; ph++) {
                for(int pw = 0; pw < pixel_size && (pw + x) < max_x; pw++) {
                    int pixel_pos_2 = pixel_pos + (ph * image->width + pw) * 3;
                    if(pixel_pos_2 <= image->width * image->height * 3) {
                        fb[pixel_pos_2] = r;
                        fb[pixel_pos_2 + 1] = g;
                        fb[pixel_pos_2 + 2] = b;
                    }
                }
            }
        }
    }
}

void blur(Image *image, float params[], float x_split, float y_split) {
    int max_x = (int)(image->width * x_split);
    int max_y = (int)(image->height * y_split);

    int size = (int)(2 + params[0] * 38);
 
    uint8_t *fb = image->framebuffer;
    uint8_t *sb = image->scratch;

    int width = image->width;
    int height = image->height;

    for(int t = 0; t < 3; t++) {
        for(int y = 0; y < max_y; y++) {
            int row_start = y * width * 3;
            int r_sum = 0, g_sum = 0, b_sum = 0, count = 0;

            for(int i = 0; i <= size; i++) {
                int p = row_start + (i * 3);
                if(i < width) {
                    r_sum += fb[p    ];
                    g_sum += fb[p + 1];
                    b_sum += fb[p + 2];
                    count++;
                }
            }

            for(int x = 0; x < max_x; x++) {
                int p = row_start + (x * 3);

                sb[p    ] = r_sum / count;
                sb[p + 1] = g_sum / count;
                sb[p + 2] = b_sum / count;

                int leaving  = x - size;
                int entering = x + size + 1;

                if(leaving >= 0) {
                    int p = row_start + (leaving * 3);
                    r_sum -= fb[p    ];
                    g_sum -= fb[p + 1];
                    b_sum -= fb[p + 2];
                    count--;
                }
                if(entering < width) {
                    int p = row_start + (entering * 3);
                    r_sum += fb[p    ];
                    g_sum += fb[p + 1];
                    b_sum += fb[p + 2];
                    count++;
                }
            }
        }

        for(int x = 0; x < max_x; x++) {
            int col_start = x * 3;
            int stride = width * 3;
            int r_sum = 0, g_sum = 0, b_sum = 0, count = 0;

            for(int i = 0; i <= size; i++) {
                int p = col_start + (i * stride);
                if(i < height) {
                    r_sum += sb[p    ];
                    g_sum += sb[p + 1];
                    b_sum += sb[p + 2];
                    count++;
                }
            }

            for(int y = 0; y < max_y; y++) {
                int p = col_start + (y * stride);

                fb[p    ] = r_sum / count;
                fb[p + 1] = g_sum / count;
                fb[p + 2] = b_sum / count;

                int leaving  = y - size;
                int entering = y + size + 1;

                if(leaving >= 0) {
                    int p = col_start + (leaving * stride);
                    r_sum -= sb[p    ];
                    g_sum -= sb[p + 1];
                    b_sum -= sb[p + 2];
                    count--;
                }
                if(entering < height) {
                    int p = col_start + (entering * stride);
                    r_sum += sb[p    ];
                    g_sum += sb[p + 1];
                    b_sum += sb[p + 2];
                    count++;
                }
            }
        }
    }
}

int init_efx(Efx efx_list[]) {
    for(int i = 0; i < NUM_EFX; i++) {
        efx_list[i] = (Efx){
            .id = i,
            .animate = false,
            .x_split = 1.0f,
            .y_split = 1.0f,
            .params = {0.5f, 0.5f, 0.5f, 0.5f}};
    }

    efx_list[INVERT].name = "INVERT";
    efx_list[INVERT].efx_func = invert;
    efx_list[INVERT].num_params = 0;

    efx_list[MONO].name = "MONO";
    efx_list[MONO].efx_func = mono;
    efx_list[MONO].num_params = 0;

    efx_list[THRESHOLD].name = "THRESHOLD";
    efx_list[THRESHOLD].efx_func = threshold;
    efx_list[THRESHOLD].num_params = 2;
    efx_list[THRESHOLD].param_names[0] = "Threshold Mode";
    efx_list[THRESHOLD].param_names[1] = "Threshold Val";
   
    efx_list[QUANTIZE].name = "QUANTIZE";
    efx_list[QUANTIZE].efx_func = quantize;
    efx_list[QUANTIZE].num_params = 1;
    efx_list[QUANTIZE].param_names[0] = "Bit Depth";

    efx_list[SATURATION].name = "SATURATION";
    efx_list[SATURATION].efx_func = saturation;
    efx_list[SATURATION].num_params = 1;
    efx_list[SATURATION].param_names[0] = "Saturation Val";
    
    efx_list[CONTRAST].name = "CONTRAST";
    efx_list[CONTRAST].efx_func = contrast;
    efx_list[CONTRAST].num_params = 1;
    efx_list[CONTRAST].param_names[0] = "Contrast Val";

    efx_list[EXPOSURE].name = "EXPOSURE";
    efx_list[EXPOSURE].efx_func = exposure;
    efx_list[EXPOSURE].num_params = 1;
    efx_list[EXPOSURE].param_names[0] = "Exposure Val";

    efx_list[COLOR_BIAS].name = "COLOR BIAS";
    efx_list[COLOR_BIAS].efx_func = color_bias;
    efx_list[COLOR_BIAS].num_params = 0;

    efx_list[COLOR_SHIFT].name = "COLOR SHIFT";
    efx_list[COLOR_SHIFT].efx_func = color_shift;
    efx_list[COLOR_SHIFT].num_params = 1;
    efx_list[COLOR_SHIFT].param_names[0] = "Shift Amount";

    efx_list[WARP].name = "WARP";
    efx_list[WARP].efx_func = warp;
    efx_list[WARP].num_params = 3;
    efx_list[WARP].param_names[0] = "Warp Mode";
    efx_list[WARP].param_names[1] = "Sine Length";
    efx_list[WARP].param_names[2] = "Sine Amp";
    
    efx_list[PIXELATE].name = "PIXELATE";
    efx_list[PIXELATE].efx_func = pixelate;
    efx_list[PIXELATE].num_params = 1;
    efx_list[PIXELATE].param_names[0] = "Pixel Size";
    
    efx_list[DITHER].name = "DITHER";
    efx_list[DITHER].efx_func = dither;
    efx_list[DITHER].num_params  = 3;
    efx_list[DITHER].param_names[0] = "Dither Mode";
    efx_list[DITHER].param_names[1] = "Dither Threshold";
    efx_list[DITHER].param_names[2] = "Dither Bit Depth";

    efx_list[BLUR].name = "BLUR";
    efx_list[BLUR].efx_func = blur;
    efx_list[BLUR].num_params = 1;
    efx_list[BLUR].param_names[0] = "Blur Size";

    for(int i = 0; i < NUM_EFX; i++) {
        if(efx_list[i].name == NULL || efx_list[i].efx_func == NULL) {
            return 1;
        }
    }
    return 0;
}

void append_to_pipeline(EfxPipeline *efx, EfxID id) {
    if(efx->count < NUM_EFX && efx->efx_list[id].efx_func != NULL) {
        for(int i = 0; i < efx->count; i++) {
            if(efx->pipeline[i]->id == id) return;
        }
        efx->pipeline[efx->count] = &efx->efx_list[id];
        efx->count++;
    }
}

void remove_from_pipeline(EfxPipeline *efx, EfxID id) {
    bool removed = false;
    for(int i = 0; i < efx->count; i++) {
        if(efx->pipeline[i]->id == id) {
            efx->pipeline[i] = NULL;
            removed = true;
        } else {
            if(removed) {
                efx->pipeline[i - 1] = efx->pipeline[i];
            }
        }
    }
    if(removed) {
        efx->count--;
        efx->pipeline[efx->count] = NULL;
    }
}

void swap_in_pipeline(EfxPipeline *efx, EfxID id_1, EfxID id_2) {
    int idx_1 = -1;
    int idx_2 = -1;
    for(int i = 0; i < efx->count; i++) {
        if(efx->pipeline[i]->id == id_1) { idx_1 = i; }
        if(efx->pipeline[i]->id == id_2) { idx_2 = i; }
    }
    if(idx_1 != -1 && idx_2 != -1) {
        Efx *temp = efx->pipeline[idx_1];
        efx->pipeline[idx_1] = efx->pipeline[idx_2];
        efx->pipeline[idx_2] = temp;
    }
}

void clear_pipeline(EfxPipeline *efx) {
    for(int i = 0; i < efx->count; i++) {
        efx->pipeline[i] = NULL;
    }
    efx->count = 0;
}

void apply_efx(Image *image, EfxPipeline *efx) {
    memcpy(image->framebuffer, image->original, image->framebuffer_size);
    
    for(int i = 0; i < efx->count; i++) {
        efx->pipeline[i]->efx_func(image, efx->pipeline[i]->params, efx->pipeline[i]->x_split, efx->pipeline[i]->y_split);
    }
}

