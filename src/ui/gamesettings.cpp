#include "SDL2/SDL.h"
#define NK_INCLUDE_DEFAULT_ALLOCATOR
#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
#define NK_INCLUDE_STANDARD_IO
#define NK_INCLUDE_FONT_BAKING
#define NK_INCLUDE_DEFAULT_FONT
#include "nuklear/nuklear.h"
#include "nuklear_sdl_renderer.h"
#include "ui.h"
#include "machine.h"

extern SDL_Window* MainWindow;
extern SDL_Renderer* Renderer;
extern nk_context* NkContext;

using namespace ae::ui;
using ae::string;

InterfaceGameSettings::InterfaceGameSettings(IMachine* m) :
	_machine(m)
{
}

bool InterfaceGameSettings::render() {
	int width, height;
	SDL_GetWindowSize(MainWindow, &width, &height);

	if (nk_begin(NkContext, "GAME SETTINGS", nk_rect(50, 150, width - 100, height - 250),
				 NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_DYNAMIC))
	{
		nk_layout_row_dynamic(NkContext, height - 350, 1);
		if (nk_group_begin(NkContext, "Row", 0)) {
			nk_layout_row_template_begin(NkContext, 40);
			nk_layout_row_template_push_dynamic(NkContext);
			nk_layout_row_template_push_static(NkContext, 40);
			nk_layout_row_template_push_dynamic(NkContext);
			nk_layout_row_template_push_static(NkContext, 40);
			nk_layout_row_template_end(NkContext);
			auto parameters = _machine->getParameters();
			auto button_border = NkContext->style.button.border_color;
			NkContext->style.button.border_color = NkContext->style.button.text_background;
			for (auto p : parameters) {
				nk_label(NkContext, p->getName().c_str(), NK_TEXT_LEFT);
				const uint8_t value = p->getValue();
				const string text = p->getAlias();
				if (value > 0) {
					if (nk_widget_is_mouse_clicked(NkContext, NK_BUTTON_LEFT)) {
						p->setValue(value - 1);
					}
					nk_button_symbol(NkContext, NK_SYMBOL_TRIANGLE_LEFT);
				}
				else {
					auto tmp = NkContext->style.button.hover;
					NkContext->style.button.hover = NkContext->style.button.normal;
					nk_button_symbol(NkContext, NK_SYMBOL_NONE);
					NkContext->style.button.hover = tmp;
				}
				nk_label(NkContext, text.c_str(), NK_TEXT_CENTERED);
				if (value < p->getMaxValue()) {
					if (nk_widget_is_mouse_clicked(NkContext, NK_BUTTON_LEFT)) {
						p->setValue(value + 1);
					}
					nk_button_symbol(NkContext, NK_SYMBOL_TRIANGLE_RIGHT);
				}
				else {
					nk_button_symbol(NkContext, NK_SYMBOL_NONE);
				}
			}
			NkContext->style.button.border_color = button_border;
			nk_group_end(NkContext);
		}
		nk_layout_row_dynamic(NkContext, 50, 2);
		if (nk_button_label(NkContext, "OK")) {
			nk_end(NkContext);
			return false;
		}
		if (nk_button_label(NkContext, "Cancel")) {
			nk_end(NkContext);
			return false;
		}
		nk_end(NkContext);
		return true;
	}
	return false;
}
