#pragma once
#include "types.h"
#include <memory>
#include "../gui/widgets.h"


namespace ae::emulator
{
	class Emulator;
	using UEmulator = std::unique_ptr<Emulator>;

	struct SystemInfo
	{
		struct Geometry {
			uint16_t width;
			uint16_t height;
		} geometry;
	};

	class Emulator
	{
	public:
		virtual void init() = 0;
		virtual void load(const json&) = 0;

		virtual SystemInfo getSystemInfo() const = 0;
		virtual void run(ae::gui::RasterDisplay*) = 0;

		// Factory
		static UEmulator create(const string&);
	};
}