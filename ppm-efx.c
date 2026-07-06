#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <string.h>

#include "effects.h"
#include "gui.h"

int create_output_file(FILE **output_file, char *output_path, char **ppm_path, char **png_path);
void convert_to_png(char *ppm_path, char *png_path);

int main(int argc, char **argv) {
    FILE *image_file = NULL;
    FILE *output_file = NULL;
    Uint8 *original = NULL;
    Uint8 *framebuffer = NULL;

    char *ppm_path = NULL;
    char *png_path = NULL;
    bool do_output = false;
    int output_format = 0;

    bool cli_mode = false;

    bool needs_update = true;

    if(argc > 4) {
        printf("Too many arguments.\n");
        printf("See the README for usage.\n");
        goto exit;
    } else if(argc == 1) {
        char image_path[256];
        printf("Welcome to ppm-efx!\n");
        printf("Please specify the path to the P6 .ppm image you want to load: ");
        scanf("%s", image_path);

        image_file = fopen(image_path, "rb");
        if(image_file == NULL) {
            perror("Failed to load the image");
            goto exit;
        }
        cli_mode = true;
    } else {
        if(strstr(argv[1], ".ppm") == NULL) {
            printf("Please specify the path to a P6 .ppm image file.\n");
            goto exit;
        }
        image_file = fopen(argv[1], "rb");
        if(image_file == NULL) {
            perror("Failed to load the image");
            goto exit;
        }
    }

    // Line 1: Format specifier (P3/P6)
    char temp[1024];
    fgets(temp, sizeof(temp), image_file);
    if(strncmp(temp, "P6", 2) != 0) {
        printf("Only P6 .ppm images are supported. The specified .ppm image has the format: %s\n", temp);
        goto exit;
    }

    // Line 2: Comment
    bool comment_consumed = false;
    while(!comment_consumed) {
        int c = fgetc(image_file);
        ungetc(c, image_file);
        if(c == '#') {
            fgets(temp, sizeof(temp), image_file);
        } else {
            comment_consumed = true;
        }
    }

    // Line 3: Dimensions (width height)
    char dimensions[32];
    int width = 0;
    int height = 0;
    fgets(dimensions, sizeof(dimensions), image_file);
    sscanf(dimensions, "%d %d", &width, &height);

    // Line 4: Maximum color value
    fgets(temp, sizeof(temp), image_file);

    // Create and open output file if -s flag is set
    if (argc == 4 && strcmp(argv[2], "-s") == 0) {
        output_format = create_output_file(&output_file, argv[3], &ppm_path, &png_path);
        if(output_file == NULL || output_format == -1) {
            perror("Failed to create the output file");
            goto exit;
        }
        fprintf(output_file, "P6\n# Made in PPM EFX.\n%d %d\n255\n", width, height);
        do_output = true;
    }
    
    // Allocate framebuffer
    size_t framebuffer_size = (((width * height) * 3));
    framebuffer = malloc(framebuffer_size);
    if(framebuffer == NULL) {
        perror("Failed to allocate memory for the framebuffer");
        goto exit;
    }
    memset(framebuffer, 0, framebuffer_size);

    // Writing the image data from the image file to the framebuffer
    fread(framebuffer, 1, framebuffer_size, image_file);

    // And copying it to save an original to be able to toggle effects
    original = malloc(framebuffer_size);
    if (original == NULL) {
        perror("Memory allocation failed.");
        goto exit;
    }
    memcpy(original, framebuffer, framebuffer_size);

    // Initialize SDL
    if(SDL_Init(SDL_INIT_VIDEO) != 0) {
        fprintf(stderr, "Failed to initialize SDL%s\n", SDL_GetError());
        goto exit;
    }
    
    int gui = init_gui();
    if(gui != 0) {
        goto exit;
    }

    int window_width = width + HUD_WIDTH;
    int window_height = height > 1000 ? 1000 : height;

    SDL_Window *window = SDL_CreateWindow("PPM EFX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         window_width, window_height, SDL_WINDOW_RESIZABLE);
    if(window == NULL){
        fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        goto exit;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == NULL) {
        fprintf(stderr, "Failed to create SDL renderer: %s\n", SDL_GetError());
        goto exit;
    }
    SDL_RenderSetLogicalSize(renderer, width + HUD_WIDTH, height);

    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, width, height);
    if(texture == NULL) {
        fprintf(stderr, "Failed to create SDL texture: %s\n", SDL_GetError());
        goto exit;
    }
    SDL_Rect texture_rect = {0};

    EffectFlags effects = {
        .warp = false,
        .invert = false,
        .mono = false,
        .quantize = false,
        .dither = false,
        .shift = false,
        .exposure = false,
        .contrast = false,
        .saturation = false,
        .color = false,
        .blur = false,
        .pixelate = false
    };

    EffectParams params = {
        .bit_depth = 4,
        .dither_brightness = 0.8,
        .mono_do_thresh = false,
        .mono_thresh = 140,
        .warp_mode = 1,
        .color_shift = 0.4,
        .exposure_val = 1.5,
        .contrast_val = 30,
        .saturation_val = 1.4,
        .color_bias = 0,
        .pixel_size = 4,
        .pixel_mode = 1
    };

    int mode = 0;

    int r = 0;
    int g = 0;
    int b = 0;

    // SDL event loop
    int app_running = 1;
    while(app_running) {
        
        // Apply effects if the image needs to be updated
        if(needs_update) {
            needs_update = false;
            memcpy(framebuffer, original, framebuffer_size);

            params.bit_depth = clamp(params.bit_depth, 1, 8);
            params.dither_brightness = clampf(params.dither_brightness, 0.0, 1.0);
            params.mono_thresh = clamp(params.mono_thresh, 0, 255);
            params.color_shift = clampf(params.color_shift, 0.0, 1.0);
            params.exposure_val = clampf(params.exposure_val, 0.5, 2.0);
            params.contrast_val = clamp(params.contrast_val, 0, 200);
            params.saturation_val = clampf(params.saturation_val, 0.5, 4.0);
            params.color_bias = clamp(params.color_bias, 0, 2);
            params.pixel_size = clamp(params.pixel_size, 1, 21);
            params.pixel_mode = clamp(params.pixel_mode, 1, 2);

            // PIXEL EFX
            if (effects.dither) {
                dither(framebuffer, framebuffer_size, width, height, params.dither_brightness);
            }
            if(effects.warp) {
                warp(framebuffer, width, height, params.warp_mode);
            }
            if(effects.blur) {
                blur(framebuffer, width, height);
            }
            if(effects.pixelate) {
                pixelate(framebuffer, width, height, params.pixel_size, params.pixel_mode);
            }
            
            // COLOR EFX
            for (int y = 0; y < height; y++) { 
                for (int x = 0; x < width; x++) {
                    int pixel_pos = (y * width + x) * 3;

                    r = framebuffer[pixel_pos    ];
                    g = framebuffer[pixel_pos + 1];
                    b = framebuffer[pixel_pos + 2];

                    if(effects.invert)     { invert(&r, &g, &b); }
                    if(effects.mono)       { monochrome(&r, &g, &b, params.mono_do_thresh, params.mono_thresh); }
                    if(effects.quantize)   { quantize(&r, &g, &b, params.bit_depth); }
                    if(effects.exposure)   { exposure(&r, &g, &b, params.exposure_val); }
                    if(effects.contrast)   { contrast(&r, &g, &b, params.contrast_val); }
                    if(effects.color)      { colorb(&r, &g, &b, params.color_bias); }
                    if(effects.shift)      { shift(&r, &g, &b, params.color_shift); }
                    if(effects.saturation) { saturation(&r, &g, &b, params.saturation_val); }

                    framebuffer[pixel_pos    ] = (Uint8)r;
                    framebuffer[pixel_pos + 1] = (Uint8)g;
                    framebuffer[pixel_pos + 2] = (Uint8)b;
                }
            }

            // Rendering
            SDL_RenderClear(renderer);

            texture_rect.x = 0;
            texture_rect.y = 0;
            texture_rect.w = width;
            texture_rect.h = height;
            SDL_UpdateTexture(texture, NULL, framebuffer, width * 3);

            SDL_RenderCopy(renderer, texture, NULL, &texture_rect);
            draw_hud(renderer, width + 20, 0, effects, params, mode);

            SDL_RenderPresent(renderer);
        }

        SDL_Event event;
        while(SDL_PollEvent(&event) != 0) {
            switch(event.type) {
                case SDL_QUIT:
                    app_running = 0;
                    break;
                case SDL_WINDOWEVENT:
                    needs_update = true;
                    break;
                case SDL_KEYUP:
                    needs_update = true;
                    switch(event.key.keysym.sym) {
                        case SDLK_ESCAPE:
                            mode = 0;
                            break;
                        case SDLK_d:
                            if(!effects.dither) { effects.dither = true; mode = 1; }
                            else if(effects.dither && mode != 1) { mode = 1; }
                            else { effects.dither = false; }
                            break;
                        case SDLK_w:
                            if(!effects.warp) { effects.warp = true; mode = 2; }
                            else if(effects.warp && mode != 2) { mode = 2; }
                            else { effects.warp = false; }
                            break;
                        case SDLK_i:
                            effects.invert = !effects.invert; 
                            break;
                        case SDLK_m:
                            if(!effects.mono) { effects.mono = true; mode = 3; }
                            else if(effects.mono && mode != 3) { mode = 3; }
                            else { effects.mono = false; }
                            break;
                        case SDLK_q:
                            if(!effects.quantize) { effects.quantize = true; mode = 4; }
                            else if(effects.quantize && mode != 4) { mode = 4; }
                            else { effects.quantize = false; }
                            break;
                        case SDLK_z:
                            if(!effects.shift) { effects.shift = true; mode = 5; }
                            else if(effects.shift && mode != 5) { mode = 5; }
                            else { effects.shift = false; }
                            break;
                        case SDLK_e:
                            if(!effects.exposure) { effects.exposure = true; mode = 6; }
                            else if(effects.exposure && mode != 6) { mode = 6; }
                            else { effects.exposure = false; }
                            break;
                        case SDLK_c:
                            if(!effects.contrast) { effects.contrast = true; mode = 7; }
                            else if(effects.contrast && mode != 7) { mode = 7; }
                            else { effects.contrast = false; }
                            break;
                        case SDLK_x:
                            if(!effects.color) { effects.color = true; mode = 8; }
                            else if(effects.color && mode != 8) { mode = 8; }
                            else { effects.color = false; }
                            break;
                        case SDLK_s:
                            if(!effects.saturation) { effects.saturation = true; mode = 9; }
                            else if(effects.saturation && mode != 9) { mode = 9; }
                            else { effects.saturation = false; }
                            break;
                        case SDLK_p:
                            if(!effects.pixelate) { effects.pixelate = true; mode = 10; }
                            else if(effects.pixelate && mode != 10) { mode = 10; }
                            else { effects.pixelate = false; }
                    }

                    if(mode == 1) {
                        switch(event.key.keysym.sym) {
                            case SDLK_UP:   params.dither_brightness += 0.1; break;
                            case SDLK_DOWN: params.dither_brightness -= 0.1;
                        }
                    }
                    else if(mode == 2) {
                        switch(event.key.keysym.sym) {
                            case SDLK_1: params.warp_mode = 1; break;
                            case SDLK_2: params.warp_mode = 2; break;
                            case SDLK_3: params.warp_mode = 3; break;
                            case SDLK_4: params.warp_mode = 4;
                        }
                    }
                    else if(mode == 3) {
                        switch(event.key.keysym.sym) {
                            case SDLK_t:    params.mono_do_thresh = !params.mono_do_thresh; break;
                            case SDLK_UP:   params.mono_thresh += 10; break;
                            case SDLK_DOWN: params.mono_thresh -= 10;
                        }
                    }
                    else if(mode == 4) {
                        switch(event.key.keysym.sym) {
                            case SDLK_UP:   params.bit_depth += 1; break;
                            case SDLK_DOWN: params.bit_depth -= 1;
                        }
                    }
                    else if(mode == 5) {
                        switch(event.key.keysym.sym) {
                            case SDLK_UP:   params.color_shift += 0.1; break;
                            case SDLK_DOWN: params.color_shift -= 0.1;
                        }
                    }
                    else if(mode == 6) {
                        switch(event.key.keysym.sym) {
                            case SDLK_UP:   params.exposure_val += 0.25; break;
                            case SDLK_DOWN: params.exposure_val -= 0.25;
                        }
                    }
                    else if(mode == 7) {
                        switch(event.key.keysym.sym) {
                            case SDLK_UP:   params.contrast_val += 10; break;
                            case SDLK_DOWN: params.contrast_val -= 10;
                        }
                    }
                    else if(mode == 8) {
                        switch(event.key.keysym.sym) {
                            case SDLK_r: params.color_bias = 0; break;
                            case SDLK_g: params.color_bias = 1; break;
                            case SDLK_b: params.color_bias = 2;
                        }
                    }
                    else if(mode == 9) {
                        switch(event.key.keysym.sym) {
                            case SDLK_UP:   params.saturation_val += 0.3; break;
                            case SDLK_DOWN: params.saturation_val -= 0.3;
                        }
                    }
                    else if(mode == 10) {
                        int step = 1;
                        if(params.pixel_mode == 2) { step = 2; }

                        switch(event.key.keysym.sym) {
                            case SDLK_UP:   params.pixel_size += step; break;
                            case SDLK_DOWN: params.pixel_size -= step; break;
                            case SDLK_1: params.pixel_mode = 1; break;
                            case SDLK_2: params.pixel_mode = 2;
                                         params.pixel_size = (params.pixel_size % 2 == 0) ? params.pixel_size + 1 : params.pixel_size;
                        }
                    }
                }
            }
        SDL_Delay(10);
    }


    
exit:
    // Clean up
    if(window)   SDL_DestroyWindow(window);
    if(renderer) SDL_DestroyRenderer(renderer);
    if(texture)  SDL_DestroyTexture(texture);
    cleanup_gui();

    SDL_Quit();

    if(cli_mode && framebuffer != NULL) {
        char reply;
        printf("Do you want to save the result? [y/n]: ");
        scanf(" %c", &reply);

        if(reply == 'y' || reply == 'Y') {
            char output_path[256];
            printf("You can save the file as a .ppm or a .png file.\n");
            printf("Conversion to .png depends on imagemagick, so you must have it installed for it to work.\n");
            printf("Please specify the path to the output file: ");
            scanf(" %s", output_path);

            output_format = create_output_file(&output_file, output_path, &ppm_path, &png_path);

            if(output_file == NULL || output_format == -1) {
                perror("Failed to create the output file");
            }

            fprintf(output_file, "P6\n# Made in PPM EFX.\n%d %d\n255\n", width, height);
            do_output = true;
        }
    }

    if (image_file != NULL)  { fclose(image_file); }

    if(do_output) {
        fwrite(framebuffer, sizeof(Uint8), framebuffer_size, output_file);
        fclose(output_file);

        if(output_format == 0) { 
            printf("File saved successfully at %s!\n", ppm_path);
        }
        else if(output_format == 1) {
            convert_to_png(ppm_path, png_path);
        }
    }
    if(ppm_path) free(ppm_path);
    if(original) free(original);
    if(framebuffer) free(framebuffer);
    return 0;
}

int create_output_file(FILE **output_file, char *output_path, char **ppm_path, char **png_path) {
    if(strstr(output_path, ".png") != NULL) {
        *png_path = output_path;
        *ppm_path = malloc(strlen(*png_path) + 5);
        if(*ppm_path == NULL) {
            perror("Failed to allocate memory for ppm_path.");
            return -1;
        }
        strcpy(*ppm_path, *png_path);
        strcat(*ppm_path, ".ppm");

        *output_file = fopen(*ppm_path, "wb");
        return 1;
    } else {
        *ppm_path = strdup(output_path);
        if(*ppm_path == NULL) {
            perror("Failed to allocate memory for ppm_path.");
            return -1;
        }
        *output_file = fopen(*ppm_path, "wb");
        return 0;
    }
}

void convert_to_png(char *ppm_path, char *png_path) {
    char *fake_argv[4] = {"magick", ppm_path, png_path, NULL};

    pid_t pid = fork();
    if(pid == 0) {
        execvp(fake_argv[0], fake_argv);
        perror("Conversion to .png failed");
        exit(1);
    } 
    else if(pid > 0) {
        int status;
        waitpid(pid, &status, 0);

        if(remove(ppm_path) != 0) {
            perror("Failed to remove temporary .ppm file");
        } 
        else {
            printf("Filed saved successfully at %s!\n", png_path);
        }
    } 
    else {
        perror("Failed to fork child process");
    }
}
