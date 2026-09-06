#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image.h"

Image* create_image(int width, int height) {
    Image *img = (Image*)malloc(sizeof(Image));
    if (!img) return NULL;
    img->width = width;
    img->height = height;
    img->data = (Pixel*)malloc(sizeof(Pixel) * width * height);
    if (!img->data) {
        free(img);
        return NULL;
    }
    return img;
}

void free_image(Image *img) {
    if (img) {
        if (img->data) free(img->data);
        free(img);
    }
}

Image* copy_image(const Image *src) {
    if (!src) return NULL;
    Image *dst = create_image(src->width, src->height);
    if (!dst) return NULL;
    memcpy(dst->data, src->data, sizeof(Pixel) * src->width * src->height);
    return dst;
}

Image* load_bmp(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) return NULL;

    BMPFileHeader file_hdr;
    BMPInfoHeader info_hdr;

    if (fread(&file_hdr, sizeof(BMPFileHeader), 1, f) != 1 ||
        fread(&info_hdr, sizeof(BMPInfoHeader), 1, f) != 1) {
        fclose(f);
        return NULL;
    }

    if (file_hdr.type != 0x4D42) {
        fclose(f);
        return NULL;
    }

    int width = info_hdr.width;
    int height = abs(info_hdr.height);
    Image *img = create_image(width, height);
    if (!img) {
        fclose(f);
        return NULL;
    }

    fseek(f, file_hdr.offset, SEEK_SET);
    int bpp = info_hdr.bit_count / 8;
    if (bpp == 0) bpp = 3;
    int padding = (4 - (width * bpp) % 4) % 4;

    for (int y = 0; y < height; y++) {
        int row = (info_hdr.height > 0) ? (height - 1 - y) : y;
        for (int x = 0; x < width; x++) {
            unsigned char pixel_bytes[4] = {0};
            if (fread(pixel_bytes, 1, bpp, f) != (size_t)bpp) {
                free_image(img);
                fclose(f);
                return NULL;
            }
            img->data[row * width + x].b = pixel_bytes[0];
            img->data[row * width + x].g = pixel_bytes[1];
            img->data[row * width + x].r = pixel_bytes[2];
        }
        fseek(f, padding, SEEK_CUR);
    }

    fclose(f);
    return img;
}

int save_bmp(const char *filename, const Image *img) {
    if (!img) return 0;
    FILE *f = fopen(filename, "wb");
    if (!f) return 0;

    int padding = (4 - (img->width * 3) % 4) % 4;
    uint32_t image_size = (img->width * 3 + padding) * img->height;

    BMPFileHeader file_hdr = {
        .type = 0x4D42,
        .size = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + image_size,
        .reserved1 = 0,
        .reserved2 = 0,
        .offset = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader)
    };

    BMPInfoHeader info_hdr = {
        .size = sizeof(BMPInfoHeader),
        .width = img->width,
        .height = img->height,
        .planes = 1,
        .bit_count = 24,
        .compression = 0,
        .image_size = image_size,
        .x_pixels_per_m = 2835,
        .y_pixels_per_m = 2835,
        .colors_used = 0,
        .colors_important = 0
    };

    fwrite(&file_hdr, sizeof(BMPFileHeader), 1, f);
    fwrite(&info_hdr, sizeof(BMPInfoHeader), 1, f);

    unsigned char pad[3] = {0, 0, 0};
    for (int y = img->height - 1; y >= 0; y--) {
        for (int x = 0; x < img->width; x++) {
            Pixel p = img->data[y * img->width + x];
            unsigned char bgr[3] = {p.b, p.g, p.r};
            fwrite(bgr, 1, 3, f);
        }
        fwrite(pad, 1, padding, f);
    }

    fclose(f);
    return 1;
}
