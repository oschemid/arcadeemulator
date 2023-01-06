#pragma once

#include "types.h"
#include "machine.h"

#include "xprocessors.h"
#include "ppu.h"
#include "apu.h"
#include "mbc.h"
#include "mmu.h"


namespace ae
{
	namespace gameboy
	{
		class Gameboy : public IMachine
		{
		public:
			std::unique_ptr<Mmu> _mmu;
			Ppu _ppu;

			std::shared_ptr<Mbc> _cartridge;
			std::shared_ptr<BootRom> _bootrom;

			Apu _apu;

			xprocessors::Cpu* cpu;

		public:
			Gameboy();
			virtual ~Gameboy() = default;

			const string getName() const override { return "Gameboy"; }
			const string getID() const override { return "Gameboy"; }
			const string getDescription() const override { return "Nintendo"; }
			std::list<ae::IParameter*> getParameters() const override {
				return { };
			}

			bool init() override;
			bool run() override;
		};
	}
}