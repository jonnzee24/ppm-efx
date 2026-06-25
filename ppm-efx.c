#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "effects.h"

void convert_to_png(char *ppm_path, char *png_path) {
    char *fake_argv[4] = {"magick", (char *)ppm_path, (char *)png_path, NULL};

    pid_t pid = fork();
    if(pid == 0) {
        execvp(fake_argv[0], fake_argv);
        perror("Convert to PNG: command failed");
        exit(1);
    } 
    else if(pid > 0) {
        int status;
        waitpid(pid, &status, 0);

        if(remove(ppm_path) != 0) {
            perror("Failed to remove temporary .ppm file");
        } 
        else {
            printf("Filed saved sucessfully at %s!\n", png_path);
        }
    } 
    else {
        perror("Failed to fork child process");
    }
}

// Effect bools
bool do_warp =  false;
bool do_invert = false;
bool do_monochrome = false;
bool do_quantization = false;
bool do_dither = false;
bool do_shift = false;
bool do_exposure = false;

// Initial values
int bit_depth = 4;
float dither_brightness = 0.8;
int dither_thresh = 120;
bool mono_do_thresh = false;
int mono_thresh = 140;
int warp_mode = 2;
int color_shift = 1;
float exposure_val = 1.5;

int main(int argc, char **argv) {
    FILE *image_file = NULL;
    FILE *output_file = NULL;
    Uint8 *original = NULL;
    Uint8 *framebuffer = NULL;

    SDL_Window *window = NULL;
    SDL_Surface *surface = NULL;

    int do_output = 0;
    int mode = 0;
    bool needs_update = true;

    if(argc > 4 || argc < 2) {
        printf("Please specify the path to a .ppm image and optionally -s <output_file.png> to save the file as a .png.\n");
        goto exit;
    }

    image_file = fopen(argv[1], "rb");
    if(image_file == NULL) {
        perror("Failed to load the image");
        goto exit;
    }

    // Line 1: Format specifier (P3/P6)
    char temp[1024];
    fgets(temp, sizeof(temp), image_file);

    // Line 2: Comment
    fgets(temp, sizeof(temp), image_file);

    // Line 3: Dimensions (width height)
    char dimensions[256];
    int width = 0;
    int height = 0;
    fgets(dimensions, sizeof(dimensions), image_file);
    sscanf(dimensions, "%d %d", &width, &height);

    // Line 4: Maximum color value
    fgets(temp, sizeof(temp), image_file);

    // Create and open output file if -s flag is set
    char *temp_ppm_path = NULL;
    char *final_png_path = NULL;

    if (argc == 4 && strcmp(argv[2], "-s") == 0) {
        final_png_path = argv[3];

        temp_ppm_path = malloc(strlen(final_png_path) + 5);
        if(temp_ppm_path == NULL) {
            perror("Memory allocation failed");
            goto exit;
        }

        strcpy(temp_ppm_path, final_png_path);
        strcat(temp_ppm_path, ".ppm");

        output_file = fopen(temp_ppm_path, "wb");
        if(output_file == NULL) {
            perror("Failed to create the output file");
            goto exit;
        }
        fprintf(output_file, "P6\n# Made in PPM EFX.\n%d %d\n255\n", width, height);
        do_output = 1;
    }
    
    // Allocate framebuffer
    size_t framebuffer_size = (((width * height) * 3));
    framebuffer = malloc(framebuffer_size);
    memset(framebuffer, 0, framebuffer_size);

    int r = 0;
    int g = 0;
    int b = 0;

    Uint32 color = 0;

    // Writing into framebuffer
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int pixel_pos = (y * width + x) * 3;

            r = fgetc(image_file);
            g = fgetc(image_file);
            b = fgetc(image_file);

            framebuffer[pixel_pos    ] = (Uint8)r;
            framebuffer[pixel_pos + 1] = (Uint8)g;
            framebuffer[pixel_pos + 2] = (Uint8)b;
        }
    }

    original = malloc(framebuffer_size);
    if (original == NULL) {
        perror("Memory allocation failed.");
        goto exit;
    }
    memcpy(original, framebuffer, framebuffer_size);
   
    // Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        perror("SDL failed to initialize");
        goto exit;
    }

    window = SDL_CreateWindow("PPM EFX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, width, height, 0);
    if(window == NULL) goto exit;

    surface = SDL_GetWindowSurface(window);
    SDL_Rect pixel = (SDL_Rect){0, 0, 1, 1};

    // SDL event loop
    int app_running = 1;
    while(app_running) {
        if(needs_update) {
            memcpy(framebuffer, original, framebuffer_size);

            bit_depth = clamp(bit_depth, 1, 24);
            dither_brightness = clampf(dither_brightness, 0.0, 1.0);
            dither_thresh = clamp(dither_thresh, 0, 255);
            mono_thresh = clamp(mono_thresh, 0, 255);
            color_shift = clamp(color_shift, 1, 2);
            
            // EFX
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    int pixel_pos = (y * width + x) * 3;

                    r = framebuffer[pixel_pos    ];
                    g = framebuffer[pixel_pos + 1];
                    b = framebuffer[pixel_pos + 2];

                    if(do_invert) { invert(&r, &g, &b); }
                    if (do_monochrome) { monochrome(&r, &g, &b, mono_do_thresh, mono_thresh); }
                    if(do_quantization) { quantize(&r, &g, &b, bit_depth); }
                    if(do_shift) { shift(&r, &g, &b, color_shift); }
                    if(do_exposure) {exposure(&r, &g, &b, exposure_val); }

                    framebuffer[pixel_pos    ] = (Uint8)r;
                    framebuffer[pixel_pos + 1] = (Uint8)g;
                    framebuffer[pixel_pos + 2] = (Uint8)b;
                }
            }

            if (do_dither) {
                // The second to last paramter is the threshold (0-255)
                // The last paramter is brightness (0.0-1.0)
                dither(framebuffer, framebuffer_size, width, height, dither_thresh, dither_brightness);
            }

            if(do_warp) {
                // DIFFERENT EFX
                // 1 = tear
                // 2 = normal mirror 
                warp(framebuffer, width, height, warp_mode);
            }

            // Rendering framebuffer on SDL window surface
            for (int y = 0; y < height; y++) {
                for (int x = 0; x < width; x++) {
                    pixel.x = x;
                    pixel.y = y;
                    
                    int pixel_pos = (y * width + x) * 3;

                    color = SDL_MapRGB(surface->format, framebuffer[pixel_pos], framebuffer[pixel_pos + 1], framebuffer[pixel_pos + 2]);
                    SDL_FillRect(surface, &pixel, color);
                }
            }
            SDL_UpdateWindowSurface(window);
            needs_update = false;
        }

        SDL_Event event;
        while(SDL_PollEvent(&event) != 0) {
            switch(event.type) {
                case SDL_QUIT:
                    app_running = 0;
                    break;

                case SDL_KEYUP:
                    needs_update = true;

                    switch(event.key.keysym.sym) {
                        case SDLK_d:
                            do_dither = !do_dither;
                            mode = 1;
                            break;
                        case SDLK_w:
                            do_warp = !do_warp;
                            mode = 2;
                            break;
                        case SDLK_i:
                            do_invert = !do_invert;
                            break;
                        case SDLK_m:
                            do_monochrome = !do_monochrome;
                            mode = 3;
                            break;
                        case SDLK_q:
                            do_quantization = !do_quantization;
                            mode = 4;
                            break;
                        case SDLK_s:
                            do_shift = !do_shift;
                            mode = 5;
                            break;
                        case SDLK_e:
                            do_exposure = !do_exposure;
                            mode = 6;
                        }

                        if(mode == 1) {
                            switch(event.key.keysym.sym) {
                                case SDLK_UP:
                                    dither_brightness += 0.1;
                                    break;
                                case SDLK_DOWN:
                                    dither_brightness -= 0.1;
                                }
                            }
                        else if(mode == 2) {
                            switch(event.key.keysym.sym) {
                                case SDLK_1:
                                    warp_mode = 1;
                                    break;
                                case SDLK_2:
                                    warp_mode = 2;
                                }
                            }
                        else if(mode == 3) {
                            switch(event.key.keysym.sym) {
                                case SDLK_t:
                                    mono_do_thresh = !mono_do_thresh;
                                    break;
                                case SDLK_UP:
                                    mono_thresh += 5;
                                    break;
                                case SDLK_DOWN:
                                    mono_thresh -= 5;
                                }
                            }
                        else if(mode == 4) {
                            switch(event.key.keysym.sym) {
                                case SDLK_UP:
                                    bit_depth += 1;
                                    break;
                                case SDLK_DOWN:
                                    bit_depth -= 1;
                            }
                        }
                        else if(mode == 5) {
                            switch(event.key.keysym.sym) {
                                case SDLK_UP:
                                    color_shift += 1;
                                    break;
                                case SDLK_DOWN:
                                    color_shift -= 1;
                            }
                        }
                        else if(mode == 6) {
                            switch(event.key.keysym.sym) {
                                case SDLK_UP:
                                    exposure_val += 0.25;
                                    break;
                                case SDLK_DOWN:
                                    exposure_val -= 0.25;
                            }
                        }
            }
        }
        SDL_Delay(10);
    }
    
exit:
    // Clean up
    SDL_DestroyWindow(window);
    SDL_Quit();

    if (image_file != NULL)  { fclose(image_file); }

    if(do_output) {
        fwrite(framebuffer, sizeof(Uint8), framebuffer_size, output_file);

        if(output_file != NULL) { fclose(output_file); }

        convert_to_png(temp_ppm_path, final_png_path);
        free(temp_ppm_path);
    }
    
    free(original);
    free(framebuffer);
    return 0;
}
