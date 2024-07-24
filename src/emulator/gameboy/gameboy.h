#pragma once

#include "types.h"
#include "core.h"

#include "sm83.h"
#include "ppu/ppu.h"
#include "apu.h"
#include "mbc.h"
#include "mmu.h"
#include "serial.h"


namespace aos::gameboy
{
	class Gameboy : public aos::Core
	{
	public:
		std::unique_ptr<Mmu> _mmu;
		SerialLink _serial;
		Ppu _ppu;
		tools::Clock _clock{ 4194 };

		std::string _rom;
		std::shared_ptr<Mbc> _cartridge;
		std::shared_ptr<BootRom> _bootrom;

		Apu _apu;

		xprocessors::cpu::sm83 _cpu;

	public:
		Gameboy(const string);
		virtual ~Gameboy() = default;

		virtual json getRequirements() const override;

		void init(map<string, Device::SharedPtr>) override;
		void run() override;
	};
}