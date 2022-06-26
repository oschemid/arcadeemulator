#pragma once

#include <SDL2/SDL.h>

namespace ae
{
	namespace ui
	{
		// UI creation
		bool init();

		// Console Display
		bool createDisplay(const uint16_t, const uint16_t);
		bool updateDisplay(const uint16_t*);
		bool destroyDisplay();

		bool refresh();

		// Ui delete
		bool destroy();
	}
}