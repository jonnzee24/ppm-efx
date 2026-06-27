#include "effects.h"

int clamp(int value, int min, int max) {
    const int t = value < min ? min : value;
    return t > max ? max : t;
}

float clampf(float value, float min, float max) {
    const float t = value < min ? min : value;
    return t > max ? max : t;
}

void warp(Uint8 *framebuffer, int width, int height, int option) {
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            int x_pos = x;
            int y_pos = y;

            if(x > width / 2 && option == 1) {
                x_pos = width - x;
                y_pos = y;
            }

            if(y > height / 2 && option == 2) {
                x_pos = x;
                y_pos = height - y;
            }
 
            if(x % 2 == 0 && option == 3) {
                x_pos = x - x / 6;
                y_pos = y - y / 6;
            }

            if(y % 2 == 0 && option == 4) {
                x_pos = x - x / 6;
                y_pos = y - y / 6;
            }

            x_pos = x_pos > width - 1 ? width - 1 : x_pos;
            x_pos = x_pos < 0 ? 0 : x_pos;
            y_pos = y_pos > height - 1 ? height - 1 : y_pos;
            y_pos = y_pos < 0 ? 0 : y_pos;

            int old_pos = (y * width + x) * 3;
            int new_pos = (y_pos * width + x_pos) * 3;

            framebuffer[old_pos    ] = framebuffer[new_pos    ];
            framebuffer[old_pos + 1] = framebuffer[new_pos + 1];
            framebuffer[old_pos + 2] = framebuffer[new_pos + 2];
        }
    }
}

void invert(int *r, int *g, int *b) {
    *r = 255 - *r;
    *g = 255 - *g;
    *b = 255 - *b;
}

void monochrome(int *r, int *g, int *b, bool do_threshold, int threshold) {
    Uint8 luminance = (*r + *g + *b) / 3;
    
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
    int divisor = 1 << (8 - bit_depth);
    int maximum_value = (1 << bit_depth) - 1;

    *r = (*r / divisor) * 255 / maximum_value;
    *g = (*g / divisor) * 255 / maximum_value;
    *b = (*b / divisor) * 255 / maximum_value;
}

void dither(Uint8 *framebuffer, size_t framebuffer_size, int width, int height, int thresh, float brightness) {
    for(int y = 0; y < height; y++) {
        for(int x = 0; x < width; x++) {
            int pixel_pos = (y * width + x) * 3;

            framebuffer[pixel_pos    ] = framebuffer[pixel_pos    ] * brightness;
            framebuffer[pixel_pos + 1] = framebuffer[pixel_pos + 1] * brightness;
            framebuffer[pixel_pos + 2] = framebuffer[pixel_pos + 2] * brightness;


            int op_r = framebuffer[pixel_pos    ];
            int op_g = framebuffer[pixel_pos + 1];
            int op_b = framebuffer[pixel_pos + 2];

            int qp_r = op_r;
            int qp_g = op_g;
            int qp_b = op_b;

            monochrome(&qp_r, &qp_g, &qp_b, true, thresh);

            framebuffer[pixel_pos    ] = qp_r;
            framebuffer[pixel_pos + 1] = qp_g;
            framebuffer[pixel_pos + 2] = qp_b;

            int error_r = op_r - qp_r;
            int error_g = op_g - qp_g;
            int error_b = op_b - qp_b;
            
            if(pixel_pos < framebuffer_size - width * 3) {
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

                    framebuffer[(pixel_pos + i + 3)            ] = clamp((framebuffer[(pixel_pos + i + 3)            ] + (error * 7) / 16), 0, 255);
                    framebuffer[(pixel_pos + i - 3) + width * 3] = clamp((framebuffer[(pixel_pos + i - 3) + width * 3] + (error * 3) / 16), 0, 255);
                    framebuffer[(pixel_pos + i    ) + width * 3] = clamp((framebuffer[(pixel_pos + i    ) + width * 3] + (error * 5) / 16), 0, 255);
                    framebuffer[(pixel_pos + i + 3) + width * 3] = clamp((framebuffer[(pixel_pos + i + 3) + width * 3] + (error * 1) / 16), 0, 255);
                }
            }
        }
    }
}

void shift(int *r, int *g, int *b, float color_shift) {
    Uint8 rb = *r;
    Uint8 gb = *g;
    Uint8 bb = *b;

    if(color_shift <= 0.5) {
        *r = (color_shift * bb) + (1 - color_shift) * rb;
        *g = (color_shift * rb) + (1 - color_shift) * gb;
        *b = (color_shift * gb) + (1 - color_shift) * bb;
    } else if(color_shift > 0.5) {
        *r = (color_shift * gb) + (1 - color_shift) * rb;
        *g = (color_shift * bb) + (1 - color_shift) * gb;
        *b = (color_shift * rb) + (1 - color_shift) * bb;
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

void colorb(int *r, int *g, int *b, int color_bias) {
    if(color_bias == 0) {
        *g = *g / 3;
        *b = *b / 3;
    } else if(color_bias == 1) {
        *r = *r / 3;
        *b = *b / 3;
    } else if(color_bias == 2) {
        *r = *r / 3;
        *g = *g / 3;
    }
}

void blur(Uint8 *framebuffer, int width, int height) {
    // Implement gaussian blur when i know how
}
