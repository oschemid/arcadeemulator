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
	class Gameboy : public aos::emulator::Emulator
	{
	public:
		std::unique_ptr<Mmu> _mmu;
		SerialLink _serial;
		Ppu _ppu;

		std::string _rom;
		std::shared_ptr<Mbc> _cartridge;
		std::shared_ptr<BootRom> _bootrom;

		Apu _apu;

		xprocessors::Cpu::Ptr cpu;

	public:
		Gameboy(const string);
		virtual ~Gameboy() = default;

		aos::emulator::SystemInfo getSystemInfo() const override;
		void init(aos::display::RasterDisplay*) override;
		uint8_t tick() override;
	};
}