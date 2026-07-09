#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>

#include "common.h"
#include "file_io.h"



int load_image(Image *image) {
    image->path = malloc(512);

    printf("Welcome to PPM EFX!\n");
    printf("Please specify the path to the P6 .ppm image you want to load: ");
    scanf("%511s", image->path);

    if(image->path[0] == '~') {
        char temp_path[512];
        char *home_dir = getenv("HOME");
        if (home_dir == NULL) {
            printf("Error: HOME environment variable is not set.\n");
            return 1;
        }
        snprintf(temp_path, sizeof(temp_path), "%s%s", home_dir, &image->path[1]);
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

int output_prompt(Output *output, bool *do_output) {
    bool done = false;    
    char reply;
    char ch;

    while(!done) {
        printf("Do you want to save the result? [y/n]: ");
        scanf(" %c", &reply);

        if(reply == 'y' || reply == 'Y') {
            output->path = malloc(512);
            if(!output->path) {
                return 1;
            }

            printf("You can save the file as a .ppm or a .png file.\n");
            printf("Conversion to .png depends on imagemagick, so you must have it installed for it to work.\n");
            printf("Please specify the path to the output file: ");
            scanf(" %511s", output->path);

            if(output->path[0] == '~') {
                char temp_path[512];
                char *home_dir = getenv("HOME");
                if (home_dir == NULL) {
                    printf("Error: HOME environment variable is not set.\n");
                    return 1;
                }
                snprintf(temp_path, sizeof(temp_path), "%s%s", home_dir, &output->path[1]);
                strncpy(output->path, temp_path, sizeof(temp_path));
            }

            if(strstr(output->path, ".ppm") != NULL) {
                output->format = PPM;
                output->file = fopen(output->path, "wb");
            }
            else if(strstr(output->path, ".png") != NULL) {
                output->format = PNG;
                output->file = fopen("temp.ppm", "wb");
            }
            else {
                printf("Invalid format. only .ppm and .png are supported.\n");
                return 1;
            }

            if(output->file == NULL) {
                return 1;
            }
            
            *do_output = true;
            done = true;
        }
        else if(reply == 'n' || reply == 'N') {
            done = true;
        }
        else {
            while ((ch = getchar()) != '\n' && ch != EOF);
        }
    }
    return 0;
}

void convert_to_png(char *output_path) {
    char *fake_argv[4] = {"magick", "temp.ppm", output_path, NULL};

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
            printf("Filed saved successfully at %s!\n", output_path);

        } else {
            perror("Imagemagick failed to convert the image to .png");
        }
    } else {
        perror("Failed to fork child process");
    }
    remove("temp.ppm");
}
