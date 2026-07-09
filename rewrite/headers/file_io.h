#ifndef FILE_IO_H
#define FILE_IO_H

#include "common.h"

int load_image(Image *image);
int parse_header(Image *image, FILE *image_file);
int output_prompt(Output *output, bool *do_output);
void convert_to_png(char *output_path);

#endif
