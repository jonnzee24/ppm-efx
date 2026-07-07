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
    Uint8 *framebuffer = NULL;
    Uint8 *original = NULL;

    char *ppm_path = NULL;
    char *png_path = NULL;
    int output_format = 0;

    bool needs_update = true;
    bool needs_render = true;
    bool panning = false;
    bool show_hud = true;

    if(argc > 2) {
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
        printf("Only P6 .ppm images are supported. The specified image has the format: %s", temp);
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

    SDL_Window *window = SDL_CreateWindow("PPM EFX", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, window_width, window_height, SDL_WINDOW_RESIZABLE);
    if(window == NULL){
        fprintf(stderr, "Failed to create SDL window: %s\n", SDL_GetError());
        goto exit;
    }
    float usr_scale = 1.0;

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if(renderer == NULL) {
        fprintf(stderr, "Failed to create SDL renderer: %s\n", SDL_GetError());
        goto exit;
    }
    
    SDL_Texture *texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, width, height);
    if(texture == NULL) {
        fprintf(stderr, "Failed to create SDL texture: %s\n", SDL_GetError());
        goto exit;
    }
    SDL_Rect texture_rect = {0, 0, width, height};
    int image_x_pos = 0;
    int image_y_pos = 0;
    int image_usr_x_offset = 0;
    int image_usr_y_offset = 0;

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
        .pixelate = false
    };

    EffectParams params = {
        .bit_depth = 4,
        .dither_brightness = 0.8f,
        .mono_do_thresh = false,
        .mono_thresh = 140,
        .warp_mode = 1,
        .sine_length = 120.0f,
        .sine_amp = 20.0f,
        .color_shift = 0.4f,
        .exposure_val = 1.5f,
        .contrast_val = 30,
        .saturation_val = 1.4f,
        .color_bias = 0,
        .pixel_size = 4
    };

    int mode = 0;

    int app_running = 1;
    while(app_running) {
        
        // Apply effects if the image needs to be updated
        if(needs_update) {
            memcpy(framebuffer, original, framebuffer_size);

            params.bit_depth = clamp(params.bit_depth, 1, 8);
            params.dither_brightness = clampf(params.dither_brightness, 0.0, 1.0);
            params.sine_length = clamp(params.sine_length, 10, 1000);
            params.sine_amp = clamp(params.sine_amp, 0, 150);

            params.mono_thresh = clamp(params.mono_thresh, 0, 255);
            params.color_shift = clampf(params.color_shift, 0.0, 1.0);
            params.exposure_val = clampf(params.exposure_val, 0.5, 2.0);
            params.contrast_val = clamp(params.contrast_val, 0, 200);
            params.saturation_val = clampf(params.saturation_val, 0.5, 4.0);
            params.color_bias = clamp(params.color_bias, 0, 2);
            params.pixel_size = clamp(params.pixel_size, 1, 20);

            // PIXEL EFX
            if(effects.warp) {
                warp(framebuffer, width, height, params.warp_mode, params.sine_length, params.sine_amp);
            }
            if(effects.pixelate) {
                pixelate(framebuffer, width, height, params.pixel_size);
            }
            if (effects.dither) {
                dither(framebuffer, framebuffer_size, width, height, params.dither_brightness);
            }

            // COLOR EFX
            for (int y = 0; y < height; y++) { 
                for (int x = 0; x < width; x++) {
                    int pixel_pos = (y * width + x) * 3;

                    int r = framebuffer[pixel_pos    ];
                    int g = framebuffer[pixel_pos + 1];
                    int b = framebuffer[pixel_pos + 2];

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
            SDL_UpdateTexture(texture, NULL, framebuffer, width * 3);
            needs_update = false;
            needs_render = true;
        }

        // RENDERING
        if(needs_render) {
            SDL_RenderClear(renderer);

            SDL_GetWindowSize(window, &window_width, &window_height);
            int image_vp_width = window_width - HUD_WIDTH;
            if(!show_hud) {image_vp_width = window_width;}
            SDL_Rect image_vp = { 0, 0, image_vp_width, window_height};
            
            // Set the scale
            usr_scale = clampf(usr_scale, 0.1, 6.0);
            float scale_x = (float)image_vp_width / width;
            float scale_y = (float)window_height / height;
            float scale = (scale_x < scale_y) ? scale_x : scale_y;
            scale *= usr_scale;

            // Draw the image
            SDL_RenderSetViewport(renderer, &image_vp);

            texture_rect.w = (int)width  * scale;
            texture_rect.h = (int)height * scale;
            image_x_pos = ((window_width - HUD_WIDTH) / 2 - texture_rect.w / 2) + image_usr_x_offset;
            image_y_pos = ((window_height / 2) - texture_rect.h / 2) + image_usr_y_offset;
            texture_rect.x = image_x_pos;
            texture_rect.y = image_y_pos;

            SDL_RenderCopy(renderer, texture, NULL, &texture_rect);
            
            if(show_hud) {
                // Draw Stats
                char buffer[64];
                
                draw_text(renderer, 30, 30, DARK_RED, "STATS");

                snprintf(buffer, 64, "Scale: %.2f", usr_scale);
                draw_text(renderer, 30, 60, WHITE, buffer);
                snprintf(buffer, 64, "Width: %d", width);
                draw_text(renderer, 30, 90, WHITE, buffer);
                snprintf(buffer, 64, "Height: %d", height);
                draw_text(renderer, 30, 110, WHITE, buffer);
                
                // Draw HUD
                SDL_Rect hud_vp = {image_vp_width + 20, 0, HUD_WIDTH, window_height};
                SDL_RenderSetViewport(renderer, &hud_vp);
                int hud_height = draw_hud(renderer, 0, 0, effects, params, mode);
            }
           
            // Reset viewport and render everything
            SDL_RenderSetViewport(renderer, NULL);
            SDL_RenderPresent(renderer);
            needs_render = false;
        }
        
        // EVENT LOOP
        SDL_Event event;
        while(SDL_PollEvent(&event) != 0) {
            switch(event.type) {
                case SDL_QUIT:
                    app_running = 0;
                    break;
                case SDL_MOUSEBUTTONDOWN:
                    if(event.button.button == SDL_BUTTON_LEFT) { panning = true; }
                    break;
                case SDL_MOUSEBUTTONUP:
                    if(event.button.button == SDL_BUTTON_LEFT) { panning = false; }
                    break;
                case SDL_MOUSEMOTION:
                    if(panning) {
                        needs_render = true;
                        image_usr_x_offset += event.motion.xrel;
                        image_usr_y_offset += event.motion.yrel;
                    }
                    break;
                case SDL_MOUSEWHEEL:
                    usr_scale += event.wheel.y * 0.1;
                case SDL_WINDOWEVENT:
                    needs_render = true;
                    break;
                case SDL_KEYDOWN:
                    switch(event.key.keysym.sym) {
                        case SDLK_MINUS:
                            usr_scale -= 0.1;
                            break;
                        case SDLK_PLUS:
                            usr_scale += 0.1;
                            break;
                        case SDLK_ESCAPE:
                            usr_scale = 1.0;
                            image_usr_x_offset = 0;
                            image_usr_y_offset = 0;
                            break;
                        case SDLK_h:
                            show_hud = !show_hud;
                            break;
                    }
                    break;
                case SDL_KEYUP:
                    needs_update = true;

                    #define TOGGLE_EFFECT(flag, m) \
                            if(!(flag)) { (flag) = true; mode = (m); } \
                            else if(mode != (m)) { mode = (m); } \
                            else { (flag) = false; } \

                    switch(event.key.keysym.sym) {
                        case SDLK_d: TOGGLE_EFFECT(effects.dither, 1); break;
                        case SDLK_w: TOGGLE_EFFECT(effects.warp, 2); break;
                        case SDLK_m: TOGGLE_EFFECT(effects.mono, 3); break;
                        case SDLK_q: TOGGLE_EFFECT(effects.quantize, 4); break;
                        case SDLK_z: TOGGLE_EFFECT(effects.shift, 5); break;
                        case SDLK_e: TOGGLE_EFFECT(effects.exposure, 6); break;
                        case SDLK_c: TOGGLE_EFFECT(effects.contrast, 7); break;
                        case SDLK_x: TOGGLE_EFFECT(effects.color, 8); break;
                        case SDLK_s: TOGGLE_EFFECT(effects.saturation, 9); break;
                        case SDLK_p: TOGGLE_EFFECT(effects.pixelate, 10); break;
                        case SDLK_i: effects.invert = !effects.invert; break;
                    }

                    if(mode == 1) {
                        switch(event.key.keysym.sym) {
                            case SDLK_UP:   params.dither_brightness += 0.1; break;
                            case SDLK_DOWN: params.dither_brightness -= 0.1;
                        }
                    } else if(mode == 2) {
                        switch(event.key.keysym.sym) {
                            case SDLK_1: params.warp_mode = 1; break;
                            case SDLK_2: params.warp_mode = 2; break;
                            case SDLK_3: params.warp_mode = 3; break;
                            case SDLK_4: params.warp_mode = 4; break;

                            case SDLK_UP: params.sine_length += 20.0f; break;
                            case SDLK_DOWN: params.sine_length -= 20.0f; break;

                            case SDLK_RIGHT: params.sine_amp += 5.0f; break;
                            case SDLK_LEFT: params.sine_amp -= 5.0f; break;
                        }
                    } else if(mode == 3) {
                        switch(event.key.keysym.sym) {
                            case SDLK_t:    params.mono_do_thresh = !params.mono_do_thresh; break;
                            case SDLK_UP:   params.mono_thresh += 10; break;
                            case SDLK_DOWN: params.mono_thresh -= 10;
                        }
                    } else if(mode == 4) {
                        switch(event.key.keysym.sym) {
                            case SDLK_UP:   params.bit_depth += 1; break;
                            case SDLK_DOWN: params.bit_depth -= 1;
                        }
                    } else if(mode == 5) {
                        switch(event.key.keysym.sym) {
                            case SDLK_UP:   params.color_shift += 0.1; break;
                            case SDLK_DOWN: params.color_shift -= 0.1;
                        }
                    } else if(mode == 6) {
                        switch(event.key.keysym.sym) {
                            case SDLK_UP:   params.exposure_val += 0.25; break;
                            case SDLK_DOWN: params.exposure_val -= 0.25;
                        }
                    } else if(mode == 7) {
                        switch(event.key.keysym.sym) {
                            case SDLK_UP:   params.contrast_val += 10; break;
                            case SDLK_DOWN: params.contrast_val -= 10;
                        }
                    } else if(mode == 8) {
                        switch(event.key.keysym.sym) {
                            case SDLK_r: params.color_bias = 0; break;
                            case SDLK_g: params.color_bias = 1; break;
                            case SDLK_b: params.color_bias = 2;
                        }
                    } else if(mode == 9) {
                        switch(event.key.keysym.sym) {
                            case SDLK_UP:   params.saturation_val += 0.3; break;
                            case SDLK_DOWN: params.saturation_val -= 0.3;
                        }
                    } else if(mode == 10) {
                        switch(event.key.keysym.sym) {
                            case SDLK_UP:   params.pixel_size += 1; break;
                            case SDLK_DOWN: params.pixel_size -= 1; break;
                        }
                    }
                SDL_Delay(10);
            }
        }
    }

exit:
    if(window)   SDL_DestroyWindow(window);
    if(renderer) SDL_DestroyRenderer(renderer);
    if(texture)  SDL_DestroyTexture(texture);
    cleanup_gui();

    SDL_PumpEvents();
    SDL_Quit();

    bool do_output = false;
    if(framebuffer) {
        bool success = false;
        char reply;
        char ch;
        printf("Do you want to save the result? [y/n]: ");
        scanf(" %c", &reply);

        while(!success) {
            if(reply == 'y' || reply == 'Y') {
                char output_path[256];

                printf("You can save the file as a .ppm or a .png file.\n");
                printf("Conversion to .png depends on imagemagick, so you must have it installed for it to work.\n");
                printf("Please specify the path to the output file: ");
                scanf(" %s", output_path);

                output_format = create_output_file(&output_file, output_path, &ppm_path, &png_path);
                while(output_format == -1) {
                    printf("Try again: ");
                    scanf(" %s", output_path);
                    output_format = create_output_file(&output_file, output_path, &ppm_path, &png_path);
                }
                do_output = true;
                success = true;
            }
            else if(reply == 'n' || reply == 'N') {
                success = true;
            }
            else {
                printf("Do you want to save the result? [y/n]: ");
                scanf(" %c", &reply);
                while ((ch = getchar()) != '\n' && ch != EOF);
            }
        }
    }

    if(do_output) {
        fprintf(output_file, "P6\n# Made in PPM EFX.\n%d %d\n255\n", width, height);
        fwrite(framebuffer, sizeof(Uint8), framebuffer_size, output_file);

        fclose(output_file);
        output_file = NULL;

        if(output_format == 0) { 
            printf("File saved successfully at %s!\n", ppm_path);
        } else if(output_format == 1) {
            convert_to_png(ppm_path, png_path);
        }
    }

    if(image_file)  fclose(image_file);
    if(output_file) fclose(output_file);
    if(ppm_path)    free(ppm_path);
    if(framebuffer) free(framebuffer);
    if(original)    free(original);
    return 0;
}

int create_output_file(FILE **output_file, char *output_path, char **ppm_path, char **png_path) {
    int format = 0;

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
        format = 1;
    } else if(strstr(output_path, ".ppm") != NULL) {
        *ppm_path = strdup(output_path);
        if(*ppm_path == NULL) {
            perror("Failed to allocate memory for ppm_path.");
            return -1;
        }
        *output_file = fopen(*ppm_path, "wb");
    } else {
        printf("The specified output file is not a .ppm or a .png image.\n");
        return -1;
    }

    if(*output_file == NULL) {
        perror("Failed to create the output file");
        free(*ppm_path);
        *ppm_path = NULL;
        return -1;
    } 
    return format;
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

        if(WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            if(remove(ppm_path) != 0) {
                perror("Failed to remove temporary .ppm file");
            } 
            else {
                printf("Filed saved successfully at %s!\n", png_path);
            }
        } else {
            perror("Imagemagick failed to convert the image to .png");
        }
    } else {
        perror("Failed to fork child process");
    }
}
