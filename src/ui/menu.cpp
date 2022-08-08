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

menu::response renderWindow(ae::IMachine* si) {
	int width, height;
	SDL_GetWindowSize(MainWindow, &width, &height);

	auto tmp = NkContext->style.window.fixed_background;
	NkContext->style.window.fixed_background = nk_style_item_color(nk_color(0, 0, 0, 0));
	if (nk_begin(NkContext, "Menu", nk_rect(50, height - 300, width - 100, 250),
				 NK_WINDOW_NO_SCROLLBAR))
	{
		nk_layout_row_dynamic(NkContext, 60, 1);
		if (createButton("Launch", (si != nullptr))) {
			nk_end(NkContext);
			NkContext->input.mouse.buttons[0].down = 0;
			return menu::response::LAUNCH;
		}
		if (createButton("Game Selection", true)) {
			nk_end(NkContext);
			return menu::response::GAMESELECTION;
		}
		if (createButton("Game Settings", (si != nullptr))) {
			nk_end(NkContext);
			return menu::response::GAMESETTINGS;
		}
		if (createButton("Quit", true)) {
			nk_end(NkContext);
			return menu::response::QUIT;
		}
		nk_end(NkContext);
	}
	NkContext->style.window.fixed_background = tmp;
	return menu::response::NOTHING;
}

menu::menu() {

}

menu::response menu::run(IMachine* si) {
	response r = NOTHING;
	while (r == NOTHING) {
		/* Input */
		SDL_Event evt;
		nk_input_begin(NkContext);
		while (SDL_PollEvent(&evt)) {
			nk_sdl_handle_event(&evt);
		}
		nk_input_end(NkContext);

		SDL_RenderClear(Renderer);
		r = renderWindow(si);
		nk_sdl_render(NK_ANTI_ALIASING_ON);

		SDL_RenderPresent(Renderer);
	}
	return r;
}

