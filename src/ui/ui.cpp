#include "ui.h"
#include <iostream>


static SDL_Window* MainWindow = NULL;
static SDL_Renderer* Renderer = NULL;
static SDL_Texture* Display = NULL;
static uint16_t DisplayWidth = 0;
static uint16_t DisplayHeight = 0;

bool ae::ui::init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        std::cout << "SDL could not be initialized: " << SDL_GetError();
        return false;
    }
    MainWindow = SDL_CreateWindow("Arcade Emulator",
                                  SDL_WINDOWPOS_UNDEFINED,
                                  SDL_WINDOWPOS_UNDEFINED,
                                  1024, 768,
                                  SDL_WINDOW_SHOWN);
    if (!MainWindow) {
        std::cout << "SDL Window could not be initialized: " << SDL_GetError();
        return false;
    }
    Renderer = SDL_CreateRenderer(MainWindow, -1, SDL_RENDERER_ACCELERATED);
    if (!Renderer) {
        std::cout << "SDL Renderer could not be initialized: " << SDL_GetError();
        return false;
    }
    return true;
}


bool ae::ui::createDisplay(const uint16_t width,
                           const uint16_t height) {
    Display = SDL_CreateTexture(Renderer,
                                SDL_PIXELFORMAT_ARGB4444, SDL_TEXTUREACCESS_STREAMING,
                                width, height);
    if (!Display) {
        std::cout << "SDL Display could not be initialized : " << SDL_GetError();
        return false;
    }
    DisplayWidth = width;
    DisplayHeight = height;
    return true;
}

bool ae::ui::updateDisplay(const uint16_t* pixels) {
    SDL_UpdateTexture(Display, NULL, pixels, 2 * DisplayWidth);
    return true;
}

bool ae::ui::destroyDisplay() {
    if (Display) {
        SDL_DestroyTexture(Display);
        Display = NULL;
        DisplayWidth = 0;
        DisplayHeight = 0;
    }
    return true;
}

bool ae::ui::refresh() {
    int x, y;
    SDL_GetWindowSize(MainWindow, &x, &y);
    SDL_Rect rect;
    rect.x = x / 2 - DisplayWidth;
    rect.y = y / 2 - DisplayHeight;
    rect.w = DisplayWidth * 2;
    rect.h = DisplayHeight * 2;

    SDL_RenderCopy(Renderer, Display, NULL, &rect);
    SDL_RenderPresent(Renderer);
    return true;
}

bool ae::ui::destroy() {
    if (Display)
        ae::ui::destroyDisplay();
    if (Renderer) {
        SDL_DestroyRenderer(Renderer);
        Renderer = NULL;
    }
    if (MainWindow) {
        SDL_DestroyWindow(MainWindow);
        MainWindow = NULL;
    }
    return true;
}


