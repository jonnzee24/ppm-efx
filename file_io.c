#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

#include "common.h"
#include "file_io.h"
#include "tinyfiledialogs.h"

#define STB_IMAGE_IMPLEMENTATION
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

    // clean up previous image data if another image is loaded
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

    // Allocating the framebuffer
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
