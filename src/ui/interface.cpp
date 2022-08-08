#include "SDL2/SDL.h"
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear/nuklear.h"
#include "nuklear_sdl_renderer.h"
#include "ui.h"

extern SDL_Window* MainWindow;
extern SDL_Renderer* Renderer;
extern nk_context* NkContext;


using namespace ae::ui;

bool IInterface::setStyle() {
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
	return true;
}

bool IInterface::run() {
	if (!setStyle())
		return false;

	bool mustContinue = true;
	while (mustContinue) {
		SDL_Event evt;
		nk_input_begin(NkContext);
		while (SDL_PollEvent(&evt)) {
			nk_sdl_handle_event(&evt);
		}
		nk_input_end(NkContext);

		mustContinue = render();
		nk_sdl_render(NK_ANTI_ALIASING_ON);

		SDL_RenderPresent(Renderer);
	}
	return true;
}