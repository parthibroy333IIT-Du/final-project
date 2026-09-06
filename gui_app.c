#include <SDL2/SDL.h>
#include <stdio.h>

#define WIDTH 800
#define HEIGHT 600

// একটি কালারফুল স্যাম্পল ছবি তৈরি করার ফাংশন
SDL_Surface* create_sample_image() {
    SDL_Surface *surface = SDL_CreateRGBSurface(0, 400, 300, 32, 0, 0, 0, 0);
    Uint32 *pixels = (Uint32 *)surface->pixels;
    for (int y = 0; y < 300; y++) {
        for (int x = 0; x < 400; x++) {
            Uint8 r = (x * 255) / 400;
            Uint8 g = (y * 255) / 300;
            Uint8 b = 150;
            pixels[y * 400 + x] = SDL_MapRGB(surface->format, r, g, b);
        }
    }
    return surface;
}

// গ্রে-স্কেল ফিল্টার
void apply_grayscale(SDL_Surface *surface) {
    Uint32 *pixels = (Uint32 *)surface->pixels;
    int total_pixels = surface->w * surface->h;
    for (int i = 0; i < total_pixels; i++) {
        Uint8 r, g, b;
        SDL_GetRGB(pixels[i], surface->format, &r, &g, &b);
        Uint8 gray = (Uint8)(0.299 * r + 0.587 * g + 0.114 * b);
        pixels[i] = SDL_MapRGB(surface->format, gray, gray, gray);
    }
}

// কালার ইনভার্ট ফিল্টার
void apply_invert(SDL_Surface *surface) {
    Uint32 *pixels = (Uint32 *)surface->pixels;
    int total_pixels = surface->w * surface->h;
    for (int i = 0; i < total_pixels; i++) {
        Uint8 r, g, b;
        SDL_GetRGB(pixels[i], surface->format, &r, &g, &b);
        pixels[i] = SDL_MapRGB(surface->format, 255 - r, 255 - g, 255 - b);
    }
}

int main() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 1;

    SDL_Window *window = SDL_CreateWindow("Image Editor Window", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, WIDTH, HEIGHT, SDL_WINDOW_SHOWN);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);

    // ছবি লোড বা স্যাম্পল ইমেজ তৈরি
    SDL_Surface *orig_surface = SDL_LoadBMP("test.bmp");
    if (!orig_surface) orig_surface = create_sample_image();

    SDL_Surface *curr_surface = SDL_ConvertSurface(orig_surface, orig_surface->format, 0);
    SDL_Texture *img_texture = SDL_CreateTextureFromSurface(renderer, curr_surface);

    // বাটন পজিশন ও সাইজ
    SDL_Rect btn1 = { 100, 480, 160, 50 }; // Grayscale (সবুজ)
    SDL_Rect btn2 = { 320, 480, 160, 50 }; // Invert (নীল)
    SDL_Rect btn3 = { 540, 480, 160, 50 }; // Reset (লাল)

    int running = 1;
    SDL_Event event;

    while (running) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) running = 0;

            if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT) {
                int mx = event.button.x;
                int my = event.button.y;

                // Grayscale বাটনে ক্লিক
                if (mx >= btn1.x && mx <= btn1.x + btn1.w && my >= btn1.y && my <= btn1.y + btn1.h) {
                    apply_grayscale(curr_surface);
                    SDL_DestroyTexture(img_texture);
                    img_texture = SDL_CreateTextureFromSurface(renderer, curr_surface);
                }
                // Invert বাটনে ক্লিক
                else if (mx >= btn2.x && mx <= btn2.x + btn2.w && my >= btn2.y && my <= btn2.y + btn2.h) {
                    apply_invert(curr_surface);
                    SDL_DestroyTexture(img_texture);
                    img_texture = SDL_CreateTextureFromSurface(renderer, curr_surface);
                }
                // Reset বাটনে ক্লিক
                else if (mx >= btn3.x && mx <= btn3.x + btn3.w && my >= btn3.y && my <= btn3.y + btn3.h) {
                    SDL_FreeSurface(curr_surface);
                    curr_surface = SDL_ConvertSurface(orig_surface, orig_surface->format, 0);
                    SDL_DestroyTexture(img_texture);
                    img_texture = SDL_CreateTextureFromSurface(renderer, curr_surface);
                }
            }
        }

        // ব্যাকগ্রাউন্ড ড্র
        SDL_SetRenderDrawColor(renderer, 30, 30, 35, 255);
        SDL_RenderClear(renderer);

        // ছবি প্রদর্শন
        SDL_Rect img_dest = { 200, 80, 400, 300 };
        SDL_RenderCopy(renderer, img_texture, NULL, &img_dest);

        // বাটন ড্র
        SDL_SetRenderDrawColor(renderer, 46, 204, 113, 255); // সবুজ বাটন (Grayscale)
        SDL_RenderFillRect(renderer, &btn1);

        SDL_SetRenderDrawColor(renderer, 52, 152, 219, 255); // নীল বাটন (Invert)
        SDL_RenderFillRect(renderer, &btn2);

        SDL_SetRenderDrawColor(renderer, 231, 76, 60, 255);  // লাল বাটন (Reset)
        SDL_RenderFillRect(renderer, &btn3);

        SDL_RenderPresent(renderer);
    }

    SDL_FreeSurface(orig_surface);
    SDL_FreeSurface(curr_surface);
    SDL_DestroyTexture(img_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
