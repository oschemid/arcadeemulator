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


void loadFont() {
	struct nk_font_atlas* atlas;
	struct nk_font_config config = nk_font_config(0);
	struct nk_font* font;

	nk_sdl_font_stash_begin(&atlas);
	font = nk_font_atlas_add_from_file(atlas, "assets/HandoSoftTrial-Regular.ttf", 20, &config);
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
	SDL_SetRenderDrawBlendMode(Renderer, SDL_BLENDMODE_MOD);
	NkContext = nk_sdl_init(MainWindow, Renderer);
	loadFont();
	return true;
}

SDL_Renderer* ae::ui::getRenderer() {
	return Renderer;
}


bool refreshBackground() {
	SDL_SetRenderDrawColor(Renderer, 0, 0, 0, 0);
	SDL_RenderClear(Renderer);
	return true;
}

bool ae::ui::destroy() {
	nk_sdl_shutdown();
	if (Renderer) {
		SDL_DestroyRenderer(Renderer);
		Renderer = NULL;
	}
	if (MainWindow) {
		SDL_DestroyWindow(MainWindow);
		MainWindow = NULL;
	}
	SDL_Quit();
	return true;
}

bool ae::ui::createButton(const string& label, bool is_active) {
	nk_style_item button_hover = NkContext->style.button.hover;
	nk_style_item button_active = NkContext->style.button.active;

	if (!is_active) {
		NkContext->style.button.hover = NkContext->style.button.normal;
		NkContext->style.button.active = NkContext->style.button.normal;
		NkContext->style.button.text_normal.a = 100;
		NkContext->style.button.text_hover.a = 100;
		NkContext->style.button.text_active.a = 100;
	}
	nk_bool res = nk_button_label(NkContext, label.c_str());
	if (!is_active) {
		NkContext->style.button.hover = button_hover;
		NkContext->style.button.active = button_active;
		NkContext->style.button.text_normal.a = 255;
		NkContext->style.button.text_hover.a = 255;
		NkContext->style.button.text_active.a = 255;
		return false;
	}
	return (bool)res;
}