#include "ui/amstradkeyboardwidget.h"


using mapp = struct { ImGuiKey key; uint8_t line; uint8_t bit; bool shift; };

// French Keyboard Mapping
static std::vector < mapp > mappings_shift = {
	{ ImGuiKey_1, 8, 0, true },
	{ ImGuiKey_2, 8, 1, true },
	{ ImGuiKey_3, 7, 1, true },
	{ ImGuiKey_4, 7, 0, true },
	{ ImGuiKey_5, 6, 1, true },
	{ ImGuiKey_6, 6, 0, true },
	{ ImGuiKey_7, 5, 1, true },
	{ ImGuiKey_8, 5, 0, true },
	{ ImGuiKey_9, 4, 1, true },
	{ ImGuiKey_0, 4, 0, true },
	{ ImGuiKey_A, 8, 3, true },
	{ ImGuiKey_Z, 7, 3, true },
	{ ImGuiKey_E, 7, 2, true },
	{ ImGuiKey_R, 6, 2, true },
	{ ImGuiKey_T, 6, 3, true },
	{ ImGuiKey_Y, 5, 3, true },
	{ ImGuiKey_U, 5, 2, true },
	{ ImGuiKey_I, 4, 3, true },
	{ ImGuiKey_O, 4, 2, true },
	{ ImGuiKey_P, 3, 3, true },
	{ ImGuiKey_Q, 8, 5, true },
	{ ImGuiKey_S, 7, 4, true },
	{ ImGuiKey_D, 7, 5, true },
	{ ImGuiKey_F, 6, 5, true },
	{ ImGuiKey_G, 6, 4, true },
	{ ImGuiKey_H, 5, 4, true },
	{ ImGuiKey_J, 5, 5, true },
	{ ImGuiKey_K, 4, 5, true },
	{ ImGuiKey_L, 4, 4, true },
	{ ImGuiKey_M, 3, 5, true },
	{ ImGuiKey_W, 8, 7, true },
	{ ImGuiKey_X, 7, 7, true },
	{ ImGuiKey_C, 7, 6, true },
	{ ImGuiKey_V, 6, 7, true },
	{ ImGuiKey_B, 6, 6, true },
	{ ImGuiKey_N, 5, 6, true },
	{ ImGuiKey_Comma, 4, 6, true },
	{ ImGuiKey_Semicolon, 4, 7, true },
};
static std::vector < mapp > mappings = {
	{ ImGuiKey_1, 8, 0, false },
	{ ImGuiKey_2, 8, 1, false },
	{ ImGuiKey_3, 7, 1, false },
	{ ImGuiKey_4, 7, 0, false },
	{ ImGuiKey_5, 6, 1, false },
	{ ImGuiKey_6, 3, 0, false },
	{ ImGuiKey_7, 5, 1, false },
	{ ImGuiKey_8, 3, 0, true },
	{ ImGuiKey_9, 4, 1, false },
	{ ImGuiKey_0, 4, 0, false },
	{ ImGuiKey_Minus, 3, 1, false },
	{ ImGuiKey_Equal, 3, 6, false },
	{ ImGuiKey_A, 8, 3, false },
	{ ImGuiKey_Z, 7, 3, false },
	{ ImGuiKey_E, 7, 2, false },
	{ ImGuiKey_R, 6, 2, false },
	{ ImGuiKey_T, 6, 3, false },
	{ ImGuiKey_Y, 5, 3, false },
	{ ImGuiKey_U, 5, 2, false },
	{ ImGuiKey_I, 4, 3, false },
	{ ImGuiKey_O, 4, 2, false },
	{ ImGuiKey_P, 3, 3, false },
	{ ImGuiKey_Q, 8, 5, false },
	{ ImGuiKey_S, 7, 4, false },
	{ ImGuiKey_D, 7, 5, false },
	{ ImGuiKey_F, 6, 5, false },
	{ ImGuiKey_G, 6, 4, false },
	{ ImGuiKey_H, 5, 4, false },
	{ ImGuiKey_J, 5, 5, false },
	{ ImGuiKey_K, 4, 5, false },
	{ ImGuiKey_L, 4, 4, false },
	{ ImGuiKey_M, 3, 5, false },
	{ ImGuiKey_W, 8, 7, false },
	{ ImGuiKey_X, 7, 7, false },
	{ ImGuiKey_C, 7, 6, false },
	{ ImGuiKey_V, 6, 7, false },
	{ ImGuiKey_B, 6, 6, false },
	{ ImGuiKey_N, 5, 6, false },
	{ ImGuiKey_Space, 5, 7, false },
	{ ImGuiKey_Comma, 4, 6, false },
	{ ImGuiKey_Semicolon, 4, 7, false },
	{ ImGuiKey_Backspace, 9, 7, false },
	{ ImGuiKey_Delete, 2, 0, false },
	{ ImGuiKey_LeftArrow, 1, 0, false },
	{ ImGuiKey_UpArrow, 0, 0, false },
	{ ImGuiKey_RightArrow, 0, 1, false },
	{ ImGuiKey_DownArrow, 0, 2, false },
	{ ImGuiKey_Enter, 2, 2, false }
};

namespace aos::ui
{
	AmstradKeyboardWidget::AmstradKeyboardWidget()
		: AmstradKeyboard{},
		Widget{}
	{
	}

	void AmstradKeyboardWidget::draw(aos::ui::Manager*)
	{
	}

	void AmstradKeyboardWidget::tickui()
	{
		bool shift = ImGui::IsKeyDown(ImGuiKey_ModShift);
		bool ctrl = ImGui::IsKeyDown(ImGuiKey_ModCtrl);
		bool alt = ImGui::IsKeyDown(ImGuiKey_ModAlt);
		
		for (int i = 0; i < 10; _lines[i++] = 0xff);
		auto io = ImGui::GetIO();
		
		if (shift)
		{
			bool keyshift = true;
			for (const auto mapping : mappings_shift)
			{
				if (ImGui::IsKeyDown(mapping.key))
				{
					_lines[mapping.line] &= ~(1<<mapping.bit);
					keyshift &= mapping.shift;
				}
			}
			if (keyshift)
				_lines[2] &= ~(1 << 5);
		}
		else
		{
			bool keyshift = false;
			for (const auto mapping : mappings)
			{
				if (ImGui::IsKeyDown(mapping.key))
				{
					_lines[mapping.line] &= ~(1 << mapping.bit);
					keyshift |= mapping.shift;
				}
			}
			if (keyshift)
				_lines[2] &= ~(1 << 5);
		}
	}
}