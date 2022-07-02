#pragma once
#include <cstdint>


namespace ae
{
	namespace ui
	{
		// UI creation
		bool init();
		bool refresh();
		bool destroy();

		// Console Display
		bool createDisplay(const std::uint16_t, const std::uint16_t);
		bool updateDisplay(const std::uint16_t*);
		bool destroyDisplay();

		// Menu
		class menu
		{
		public:
			enum response { NOTHING, LAUNCH, SETTINGS, QUIT };

		public:
			menu();
			response run();
		};
	}
}