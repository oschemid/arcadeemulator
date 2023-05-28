#pragma once
#include "types.h"
#include "emulator.h"
#include "library.h"
#include "xprocessors.h"


namespace aos::midway8080
{
	// Console Taito/Midway8080
	// ------------------------
	// Cpu : Intel 8080 (1.997MHz)

	class Midway8080 : public emulator::Emulator
	{
	public:
		Midway8080();
		virtual ~Midway8080();

		void init(ae::display::RasterDisplay*) override;

	protected:
		xprocessors::Cpu::Ptr _cpu{ nullptr };
		uint8_t* _memory{ nullptr };
		ae::display::RasterDisplay* _raster;

		virtual uint8_t in(const uint8_t) = 0;
		virtual void out(const uint8_t, const uint8_t) = 0;
	};
}