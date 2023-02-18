#pragma once
#include "types.h"
#include "../gameboy.h"
#include "../../../gui/vulkan/engine.h"


namespace ae::gameboy::debug
{
	class Debugger
	{
	protected:
		Gameboy* _console;
		ae::gui::RasterDisplay* _raster;

		uint32_t _map[256 * 256];

	public:
		Debugger(Gameboy* console, ae::gui::RasterDisplay* raster) : _console(console), _raster(raster) {}

		void run();
	};
}