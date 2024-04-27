#pragma once
#include "types.h"
#include "emulator.h"
#include "xprocessors.h"
#include "memory.h"
#include "../../controller/arcadecontroller.h"
#include "io.h"


namespace aos::midway8080
{
	// Console Taito/Midway8080
	// ------------------------
	// Cpu : Intel 8080 (1.997MHz)

	class Midway8080 : public emulator::Emulator
	{
	public:
		Midway8080(const aos::mmu::RomMappings&);
		virtual ~Midway8080();

		emulator::SystemInfo getSystemInfo() const override;

		void rotateDisplay() { _rotating = true; }
		void colorfn(std::function<rgb_t(const uint8_t, const uint8_t, const bool)> fn) { _colorfn = fn; }

		void init(aos::display::RasterDisplay*, aos::controller::Controller*) override;
		uint8_t tick() override;

	protected:
		xprocessors::Cpu::Ptr _cpu{ nullptr };
		mmu::Mmu _mmu;
		aos::display::RasterDisplay* _raster;

		aos::io::Port _port0{ 0 };
		aos::io::Port _port1{ 0 };
		aos::io::Port _port2{ 0 };
		ae::controller::ArcadeController::Ptr _controller{ nullptr };

		vector<aos::mmu::RomMapping> _roms;

		bool _rotating{ false };
		std::function<rgb_t(const uint8_t, const uint8_t, const bool)>	_colorfn;
		virtual uint8_t in(const uint8_t) = 0;
		virtual void out(const uint8_t, const uint8_t) = 0;

		void updateDisplay();
	};
}