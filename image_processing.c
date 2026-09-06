#include <stdio.h>
#include <stdlib.h>
#include "image_processing.h"

void apply_grayscale(Image *img) {
    if (!img) return;
    for (int i = 0; i < img->width * img->height; i++) {
        unsigned char gray = (unsigned char)(0.299 * img->data[i].r + 0.587 * img->data[i].g + 0.114 * img->data[i].b);
        img->data[i].r = gray;
        img->data[i].g = gray;
        img->data[i].b = gray;
    }
}

void apply_invert(Image *img) {
    if (!img) return;
    for (int i = 0; i < img->width * img->height; i++) {
        img->data[i].r = 255 - img->data[i].r;
        img->data[i].g = 255 - img->data[i].g;
        img->data[i].b = 255 - img->data[i].b;
    }
}

void apply_brightness(Image *img, int amount) {
    if (!img) return;
    for (int i = 0; i < img->width * img->height; i++) {
        int r = img->data[i].r + amount;
        int g = img->data[i].g + amount;
        int b = img->data[i].b + amount;
        img->data[i].r = (r > 255) ? 255 : ((r < 0) ? 0 : r);
        img->data[i].g = (g > 255) ? 255 : ((g < 0) ? 0 : g);
        img->data[i].b = (b > 255) ? 255 : ((b < 0) ? 0 : b);
    }
}

void apply_flip_horizontal(Image *img) {
    if (!img) return;
    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width / 2; x++) {
            int idx1 = y * img->width + x;
            int idx2 = y * img->width + (img->width - 1 - x);
            Pixel temp = img->data[idx1];
            img->data[idx1] = img->data[idx2];
            img->data[idx2] = temp;
        }
    }
}

Image* rotate_90_clockwise(const Image *src) {
    if (!src) return NULL;
    Image *dst = create_image(src->height, src->width);
    if (!dst) return NULL;
    for (int y = 0; y < src->height; y++) {
        for (int x = 0; x < src->width; x++) {
            int src_idx = y * src->width + x;
            int dst_x = src->height - 1 - y;
            int dst_y = x;
            int dst_idx = dst_y * dst->width + dst_x;
            dst->data[dst_idx] = src->data[src_idx];
        }
    }
    return dst;
}

Image* apply_blur(const Image *src) {
    if (!src) return NULL;
    Image *dst = create_image(src->width, src->height);
    if (!dst) return NULL;
    for (int y = 0; y < src->height; y++) {
        for (int x = 0; x < src->width; x++) {
            int r = 0, g = 0, b = 0, count = 0;
            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int nx = x + dx;
                    int ny = y + dy;
                    if (nx >= 0 && nx < src->width && ny >= 0 && ny < src->height) {
                        Pixel p = src->data[ny * src->width + nx];
                        r += p.r; g += p.g; b += p.b;
                        count++;
                    }
                }
            }
            dst->data[y * src->width + x].r = r / count;
            dst->data[y * src->width + x].g = g / count;
            dst->data[y * src->width + x].b = b / count;
        }
    }
    return dst;
}
