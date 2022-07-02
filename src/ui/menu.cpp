#include "SDL2/SDL.h"
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#define _CRT_SECURE_NO_WARNINGS
#include "nuklear/nuklear.h"
#include "nuklear_sdl_renderer.h"
#include "ui.h"


extern SDL_Window* MainWindow;
extern SDL_Renderer* Renderer;
extern nk_context* NkContext;


using namespace ae::ui;

static void setStyle() {
	// Window
	nk_color transparent = nk_color();
	transparent.a = 255;

	nk_color flou = nk_color();
	flou.b = 50;
	flou.g = 50;
	flou.r = 50;
	flou.a = 150;

	NkContext->style.window.header.normal = nk_style_item_color(transparent);
	NkContext->style.window.header.active = nk_style_item_color(transparent);
	NkContext->style.window.fixed_background = nk_style_item_color(flou);
}

menu::response renderWindow() {
	int width, height;
	SDL_GetWindowSize(MainWindow, &width, &height);

	if (nk_begin(NkContext, "Menu", nk_rect(50, 150, width - 100, height - 250),
				 NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic(NkContext, 30, 1);
		if (nk_button_label(NkContext, "Launch")) {
			nk_end(NkContext);
			return menu::response::LAUNCH;
		}
		if (nk_button_label(NkContext, "Game Settings")) {
			nk_end(NkContext);
			return menu::response::SETTINGS;
		}
		if (nk_button_label(NkContext, "Quit")) {
			nk_end(NkContext);
			return menu::response::QUIT;
		}
		nk_end(NkContext);
	}
	return menu::response::NOTHING;
}

menu::menu() {

}

menu::response menu::run() {
	setStyle();
	response r = NOTHING;
	while (r == NOTHING) {
		/* Input */
		SDL_Event evt;
		nk_input_begin(NkContext);
		while (SDL_PollEvent(&evt)) {
			nk_sdl_handle_event(&evt);
		}
		nk_input_end(NkContext);

		r = renderWindow();
		nk_sdl_render(NK_ANTI_ALIASING_ON);

		SDL_RenderPresent(Renderer);
	}
	return r;
}

