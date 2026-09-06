#ifndef IMAGE_PROCESSING_H
#define IMAGE_PROCESSING_H

#include "image.h"

void apply_grayscale(Image *img);
void apply_invert(Image *img);
void apply_brightness(Image *img, int amount);
void apply_flip_horizontal(Image *img);
Image* rotate_90_clockwise(const Image *src);
Image* apply_blur(const Image *src);

#endif
