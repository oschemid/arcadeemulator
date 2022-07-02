#define _CRT_SECURE_NO_WARNINGS
#include "ui.h"

#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define NK_IMPLEMENTATION
#include <nuklear/nuklear.h>
#define NK_SDL_RENDERER_IMPLEMENTATION
#include "nuklear_sdl_renderer.h"

#include <iostream>

#include <SDL2/SDL.h>


SDL_Window* MainWindow = NULL;
SDL_Renderer* Renderer = NULL;
nk_context* NkContext = NULL;


SDL_Texture* Display = NULL;
uint16_t DisplayWidth = 0;
uint16_t DisplayHeight = 0;


void loadFont() {
	struct nk_font_atlas* atlas;
	struct nk_font_config config = nk_font_config(0);
	struct nk_font* font;

	nk_sdl_font_stash_begin(&atlas);
	font = nk_font_atlas_add_from_file(atlas, "assets/arcadeclassic.ttf", 14, &config);
	nk_sdl_font_stash_end();

	nk_style_set_font(NkContext, &font->handle);
}

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
	NkContext = nk_sdl_init(MainWindow, Renderer);
	loadFont();
	return true;
}


bool ae::ui::createDisplay(const std::uint16_t width,
						   const std::uint16_t height) {
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
	nk_sdl_shutdown();
	return true;
}
