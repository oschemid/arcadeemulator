#pragma once

#include "types.h"
#include "emulator.h"

#include "xprocessors.h"
#include "ppu/ppu.h"
#include "apu.h"
#include "mbc.h"
#include "mmu.h"
#include "serial.h"


namespace ae::gameboy
{
	class Gameboy : public emulator::Emulator
	{
	public:
		std::unique_ptr<Mmu> _mmu;
		SerialLink _serial;
		Ppu _ppu;

		std::shared_ptr<Mbc> _cartridge;
		std::shared_ptr<BootRom> _bootrom;

		Apu _apu;

		xprocessors::Cpu::Ptr cpu;

	public:
		Gameboy();
		virtual ~Gameboy() = default;

		emulator::SystemInfo getSystemInfo() const override;
		void init(const json&) override;
		void run(ae::gui::RasterDisplay*) override;
	};
}