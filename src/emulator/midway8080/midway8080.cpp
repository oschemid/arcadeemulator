#include "midway8080.h"
#include "library.h"


static aos::library::Console Midway8080{ "midway8080", "Midway 8080" };


namespace aos::midway8080
{
	Midway8080::Midway8080(const aos::mmu::RomMappings& roms) :
		_raster { nullptr },
		_roms{ roms }
	{
		_clockPerMs = 1997;
	}

	Midway8080::~Midway8080()
	{
	}

	void Midway8080::init(aos::display::RasterDisplay* raster,
		                  aos::controller::Controller*)
	{
		_cpu = xprocessors::Cpu::create("i8080");
		_raster = raster;
		_cpu->in([this](const uint8_t p) { return in(p); });
		_cpu->out([this](const uint8_t p, const uint8_t v) { return out(p, v); });
		_cpu->read([this](const uint16_t p) { return _mmu.read(p); });
		_cpu->write([this](const uint16_t p, const uint8_t v) { _mmu.write(p, v); });
	}

	emulator::SystemInfo Midway8080::getSystemInfo() const {
		if (_rotating)
			return emulator::SystemInfo{
			.geometry = {.width = 256, .height = 224}
			};
		return emulator::SystemInfo{
			.geometry = {.width = 224, .height = 256}
		};
	}

	void Midway8080::updateDisplay()
	{
		uint8_t* _memory = _mmu.getvram();
		for (int y = 0; y < 224; y++) {
			for (int x = 0; x < 256; x += 8) {
				uint8_t VRAMByte = _memory[(y << 5) + (x >> 3)];

				for (int bit = 0; bit < 8; bit++) {
					uint8_t CoordX;
					uint8_t CoordY;

					if (_rotating) {
						CoordX = x + bit;
						CoordY = y;
					}
					else {
						CoordX = y;
						CoordY = (256 - 1 - (x + bit));
					}

					aos::rgb_t color = { 0,0,0 };
					if (_colorfn)
						color = _colorfn(CoordX, CoordY, (VRAMByte >> bit) & 1);
					else
						if ((VRAMByte >> bit) & 1) {
							color = aos::rgb_t{ 255,255,255 };
					}
					_raster->set(CoordX, CoordY, color);
				}
			}
		}
	}

	uint8_t Midway8080::tick()
	{
		static uint64_t deltaDisplay{ 0 };
		static uint64_t deltaController{ 0 };
		static uint8_t draw{ 0 };
		uint64_t previous = _cpu->elapsed_cycles();
		_cpu->executeOne();
		uint64_t deltaClock = _cpu->elapsed_cycles() - previous;

		if (deltaDisplay > 16641) { // 120 Hz
			if (draw) {
				updateDisplay();
				_raster->refresh();
				_cpu->interrupt(2);
			}
			else
				_cpu->interrupt(1);
			draw = 1 - draw;
			deltaDisplay -= 16641;
		}
		deltaDisplay += deltaClock;

		if (deltaController > 66566) { // 30 Hz
			_controller->tick(); _port0.tick(*_controller); _port1.tick(*_controller); _port2.tick(*_controller);
			deltaController -= 66566;
		}
		deltaController += deltaClock;
		return static_cast<uint8_t>(deltaClock);
	}
}
