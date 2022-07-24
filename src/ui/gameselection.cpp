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

InterfaceGameSelection::InterfaceGameSelection(const string& current) :
	selection(current)
{
}

bool InterfaceGameSelection::render() {
	int width, height;
	SDL_GetWindowSize(MainWindow, &width, &height);

	if (nk_begin(NkContext, "GAME SELECTION", nk_rect(50, 150, width - 100, height - 250),
				 NK_WINDOW_TITLE | NK_WINDOW_NO_SCROLLBAR | NK_WINDOW_DYNAMIC))
	{
		nk_layout_row_dynamic(NkContext, height - 350, 1);
		if (nk_group_begin(NkContext, "Row", 0)) {
			nk_layout_row_template_begin(NkContext, 80);
			nk_layout_row_template_push_static(NkContext, 80);
			nk_layout_row_template_push_dynamic(NkContext);
			nk_layout_row_template_end(NkContext);

			machinesList machines = getMachines();
			for (auto machine : machines) {
				string name = machine.second()->getName();
				string ID = machine.first;
				string description = machine.second()->getDescription();

				nk_label(NkContext, "VIDE", NK_TEXT_CENTERED);

				auto tmp = NkContext->style.window.fixed_background;
				if (nk_widget_is_mouse_clicked(NkContext, nk_buttons::NK_BUTTON_LEFT))
					selection = ID;
				if ((nk_widget_is_hovered(NkContext)) || (selection == ID))
					NkContext->style.window.fixed_background = nk_style_item_color(nk_color(40, 40, 40, 255));
				if (nk_group_begin(NkContext, "Info", NK_WINDOW_NO_SCROLLBAR)) {
					nk_layout_row_dynamic(NkContext, 20, 1);
					nk_label(NkContext, name.c_str(), NK_TEXT_LEFT);
					nk_label(NkContext, description.c_str(), NK_TEXT_LEFT);
					nk_group_end(NkContext);
				}
				NkContext->style.window.fixed_background = tmp;
			}
			nk_group_end(NkContext);
		}
		nk_layout_row_dynamic(NkContext, 50, 2);
		if (nk_button_label(NkContext, "OK")) {
			nk_end(NkContext);
			return false;
		}
		if (nk_button_label(NkContext, "Cancel")) {
			selection = "";
			nk_end(NkContext);
			return false;
		}
		nk_end(NkContext);
		return true;
	}
	return false;
}

const string InterfaceGameSelection::getSelection() const {
	return selection;
}