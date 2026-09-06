#include <stdio.h>
#include <stdlib.h>

typedef struct {
    unsigned char b, g, r;
} Pixel;

typedef struct {
    int width;
    int height;
    Pixel *pixels;
} Image;

Image* load_bmp(const char *filename) {
    FILE *f = fopen(filename, "rb");
    if (!f) {
        printf(" Error: File '%s' not found!\n", filename);
        return NULL;
    }

    unsigned char header[54];
    if (fread(header, 1, 54, f) != 54) {
        printf(" Error: Invalid BMP header.\n");
        fclose(f);
        return NULL;
    }

    Image *img = (Image*) malloc(sizeof(Image));
    img->width = *(int*)&header[18];
    img->height = *(int*)&header[22];
    int size = 3 * img->width * img->height;

    unsigned char *data = (unsigned char*) malloc(size);
    fread(data, 1, size, f);
    fclose(f);

    img->pixels = (Pixel*) malloc(sizeof(Pixel) * img->width * img->height);
    for (int i = 0; i < img->width * img->height; i++) {
        img->pixels[i].b = data[i * 3];
        img->pixels[i].g = data[i * 3 + 1];
        img->pixels[i].r = data[i * 3 + 2];
    }
    free(data);
    printf(" Image loaded successfully (%dx%d px)\n", img->width, img->height);
    return img;
}

int save_bmp(const char *filename, const Image *img) {
    if (!img) {
        printf(" Error: No image loaded to save.\n");
        return 0;
    }

    FILE *f = fopen(filename, "wb");
    if (!f) {
        printf(" Error: Cannot write to file '%s'\n", filename);
        return 0;
    }

    int filesize = 54 + 3 * img->width * img->height;
    unsigned char header[54] = {
        'B','M', filesize, filesize>>8, filesize>>16, filesize>>24,
        0,0, 0,0, 54,0,0,0, 40,0,0,0,
        img->width, img->width>>8, img->width>>16, img->width>>24,
        img->height, img->height>>8, img->height>>16, img->height>>24,
        1,0, 24,0
    };

    fwrite(header, 1, 54, f);
    for (int i = 0; i < img->width * img->height; i++) {
        fputc(img->pixels[i].b, f);
        fputc(img->pixels[i].g, f);
        fputc(img->pixels[i].r, f);
    }
    fclose(f);
    printf(" Image saved as '%s'\n", filename);
    return 1;
}

void convert_to_grayscale(Image *img) {
    if (!img) return;
    for (int i = 0; i < img->width * img->height; i++) {
        unsigned char gray = (unsigned char)(0.299 * img->pixels[i].r + 0.587 * img->pixels[i].g + 0.114 * img->pixels[i].b);
        img->pixels[i].r = gray;
        img->pixels[i].g = gray;
        img->pixels[i].b = gray;
    }
    printf(" Applied Grayscale filter.\n");
}

void invert_colors(Image *img) {
    if (!img) return;
    for (int i = 0; i < img->width * img->height; i++) {
        img->pixels[i].r = 255 - img->pixels[i].r;
        img->pixels[i].g = 255 - img->pixels[i].g;
        img->pixels[i].b = 255 - img->pixels[i].b;
    }
    printf(" Applied Color Invert filter.\n");
}

void adjust_brightness(Image *img, int value) {
    if (!img) return;
    for (int i = 0; i < img->width * img->height; i++) {
        int r = img->pixels[i].r + value;
        int g = img->pixels[i].g + value;
        int b = img->pixels[i].b + value;

        img->pixels[i].r = (r > 255) ? 255 : ((r < 0) ? 0 : r);
        img->pixels[i].g = (g > 255) ? 255 : ((g < 0) ? 0 : g);
        img->pixels[i].b = (b > 255) ? 255 : ((b < 0) ? 0 : b);
    }
    printf(" Adjusted brightness by %d.\n", value);
}

void free_image(Image *img) {
    if (img) {
        if (img->pixels) free(img->pixels);
        free(img);
    }
}

int main() {
    Image *current_image = NULL;
    int choice;
    char filename[256];

    while (1) {
        printf("\n====================================\n");
        printf("       TERMINAL IMAGE EDITOR        \n");
        printf("====================================\n");
        printf(" 1. Load BMP Image\n");
        printf(" 2. Convert to Grayscale\n");
        printf(" 3. Invert Colors\n");
        printf(" 4. Adjust Brightness\n");
        printf(" 5. Save Image\n");
        printf(" 6. Exit\n");
        printf("------------------------------------\n");
        printf(" Select an option (1-6): ");
        if (scanf("%d", &choice) != 1) break;

        switch (choice) {
            case 1:
                printf("Enter BMP file path: ");
                scanf("%s", filename);
                if (current_image) free_image(current_image);
                current_image = load_bmp(filename);
                break;
            case 2:
                if (current_image) convert_to_grayscale(current_image);
                else printf(" Please load an image first!\n");
                break;
            case 3:
                if (current_image) invert_colors(current_image);
                else printf(" Please load an image first!\n");
                break;
            case 4:
                if (current_image) {
                    int b;
                    printf("Enter brightness change (-255 to 255): ");
                    scanf("%d", &b);
                    adjust_brightness(current_image, b);
                } else printf(" Please load an image first!\n");
                break;
            case 5:
                if (current_image) {
                    printf("Enter output filename (e.g. output.bmp): ");
                    scanf("%s", filename);
                    save_bmp(filename, current_image);
                } else printf(" Please load an image first!\n");
                break;
            case 6:
                if (current_image) free_image(current_image);
                printf("Exiting Image Editor. Goodbye!\n");
                return 0;
            default:
                printf(" Invalid option. Try again.\n");
        }
    }
    return 0;
}
