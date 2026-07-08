#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "headers/file_io.h"
#include "headers/ppm-efx.h"

int load_image(Image *image) {
    image->path = malloc(512);

    printf("Welcome to ppm-efx!\n");
    printf("Please specify the path to the P6 .ppm image you want to load: ");
    scanf("%s", image->path);

    if(image->path[0] == '~') {
        char temp_path[512];
        char *home = getenv("HOME");
        if(home == NULL) {
            return 1;
        }
        snprintf(temp_path, sizeof(temp_path), "%s%s", home, &image->path[1]);
        strncpy(image->path, temp_path, 512);
    }
    
    FILE *image_file = fopen(image->path, "rb");
    if(image_file == NULL) { return 1; }

    if(parse_header(image, image_file) != 0) { return 1; }

    // Allocating the framebuffer
    image->framebuffer_size = (image->width * image->height * 3);
    image->framebuffer = malloc(image->framebuffer_size);
    if(image->framebuffer == NULL) {
        return 1;
    }
    memset(image->framebuffer, 0, image->framebuffer_size);

    // Writing the image data from the image file to the framebuffer
    fread(image->framebuffer, 1, image->framebuffer_size, image_file);

    // And copying it to save an original to be able to toggle effects
    image->original = malloc(image->framebuffer_size);
    if (image->original == NULL) { return 1; }
    memcpy(image->original, image->framebuffer, image->framebuffer_size);
    
    fclose(image_file);
    return 0;
}

int parse_header(Image *image, FILE *image_file) {
    // Line 1: Format specifier (P3/P6)
    char temp[1024];
    fgets(temp, sizeof(temp), image_file);
    if(strncmp(temp, "P6", 2) != 0) {
        printf("Only P6 .ppm images are supported. The specified image has the format: %s", temp);
        return 1;
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
    fgets(dimensions, sizeof(dimensions), image_file);
    sscanf(dimensions, "%d %d", &image->width, &image->height);

    // Line 4: Maximum color value
    fgets(temp, sizeof(temp), image_file);

    return 0;
}
