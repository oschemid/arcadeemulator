#include "midway8080.h"
#include "spaceinvaders.h"
#include "amazingmaze.h"
#include "spacechaser.h"


static aos::library::Console Midway8080{ "midway8080", "Midway 8080" };


namespace aos::midway8080
{
	Midway8080::Midway8080() :
		_raster { nullptr }
	{
		_clockPerMs = 1997;
	}

	Midway8080::~Midway8080()
	{
		if (_memory)
			delete[] _memory;
	}

	void Midway8080::init(aos::display::RasterDisplay* raster)
	{
		_cpu = xprocessors::Cpu::create("i8080");
		_raster = raster;
		_cpu->in([this](const uint8_t p) { return in(p); });
		_cpu->out([this](const uint8_t p, const uint8_t v) { return out(p, v); });
	}

}
