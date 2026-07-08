#ifndef FILE_IO_H
#define FILE_IO_H

#include "ppm-efx.h"

int load_image(Image *image);
int parse_header(Image *image, FILE *image_file);

#endif
