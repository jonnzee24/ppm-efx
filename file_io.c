#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

#include "common.h"
#include "tinyfiledialogs.h"

#include "stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

void *load_image(void *data) {
    Image *image = (Image *)data;
    
    const char *filter_patterns[4] = {"*.ppm", "*.png", "*.jpg", "*.jpeg"};
    const char *image_path = tinyfd_openFileDialog("Load image", "./", 4, filter_patterns, "Image files", 0);
    if(image_path == NULL) {
        return NULL;
    }

    if (image->path) {
        free(image->path);
        image->path = NULL;
    }
    if (image->framebuffer) {
        free(image->framebuffer);
        image->framebuffer = NULL;
    }
    if (image->original) {
        stbi_image_free(image->original);
        image->original = NULL;
    }
    if(image->scratch) {
        free(image->scratch);
        image->scratch = NULL;
    }

    image->path = strdup(image_path);
    if(image->path == NULL) {
        return NULL;
    }
    
    int why_channels;
    image->original = stbi_load(image->path, &image->width, &image->height, &why_channels, 3);
    if(image->original == NULL) {
        free(image->path);
        image->path = NULL;
        return NULL;
    }

    image->framebuffer_size = (image->width * image->height * 3);
    image->framebuffer = malloc(image->framebuffer_size);
    if(image->framebuffer == NULL) {
        free(image->path);
        stbi_image_free(image->original);
        image->path = NULL;
        image->original = NULL;
        return NULL;
    }
    memcpy(image->framebuffer, image->original, image->framebuffer_size);

    image->scratch = malloc(image->framebuffer_size);
    if(image->scratch == NULL) {
        free(image->path);
        stbi_image_free(image->original);
        free(image->framebuffer);
        image->path = NULL;
        image->original = NULL;
        image->scratch = NULL;
        return NULL;
    }
    memcpy(image->scratch, image->original, image->framebuffer_size);

    image->needs_reload = true;
    return NULL;
}

void *save_image(void *data) {
    Image *image = (Image *)data;

    const char *filter_patterns[1] = {"*.png"};
    const char *output_path = tinyfd_saveFileDialog("Save image", "./output.png", 1, filter_patterns, NULL);
    if(output_path == NULL) {
        return NULL;
    }

    if(!stbi_write_png(output_path, image->width, image->height, 3, image->framebuffer, image->width * 3)) {
        fprintf(stderr, "Failed to save the image at %s\n", output_path);
    }

    return NULL;
}
