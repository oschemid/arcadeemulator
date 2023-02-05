#pragma once

#include "types.h"
#include "emulator.h"

#include "xprocessors.h"
#include "ppu.h"
#include "apu.h"
#include "mbc.h"
#include "mmu.h"


namespace ae
{
	namespace gameboy
	{
		class Gameboy : public emulator::Emulator
		{
		public:
			std::unique_ptr<Mmu> _mmu;
			Ppu _ppu;

			std::shared_ptr<Mbc> _cartridge;
			std::shared_ptr<BootRom> _bootrom;

			Apu _apu;

			xprocessors::UCpu cpu;

		public:
			Gameboy();
			virtual ~Gameboy() = default;

//			std::list<ae::IParameter*> getParameters() const override {
//				return { };
//			}
			emulator::SystemInfo getSystemInfo() const override;
			void init() override;
			void load(const json&) override;
			void run(ae::gui::RasterDisplay*) override;
		};
	}
}