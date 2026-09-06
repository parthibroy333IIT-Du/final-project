#ifndef IMAGE_H
#define IMAGE_H

#include <stdint.h>

#pragma pack(push, 1)
typedef struct {
    uint16_t type;
    uint32_t size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t offset;
} BMPFileHeader;

typedef struct {
    uint32_t size;
    int32_t  width;
    int32_t  height;
    uint16_t planes;
    uint16_t bit_count;
    uint32_t compression;
    uint32_t image_size;
    int32_t  x_pixels_per_m;
    int32_t  y_pixels_per_m;
    uint32_t colors_used;
    uint32_t colors_important;
} BMPInfoHeader;
#pragma pack(pop)

typedef struct {
    unsigned char r, g, b;
} Pixel;

typedef struct {
    int width;
    int height;
    Pixel *data;
} Image;

Image* create_image(int width, int height);
void free_image(Image *img);
Image* copy_image(const Image *src);
Image* load_bmp(const char *filename);
int save_bmp(const char *filename, const Image *img);

#endif
