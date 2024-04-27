#pragma once
#include "types.h"
#include "memory.h"


namespace aos::amstrad
{
	class Memory
	{
	public:
		Memory();
		~Memory();
		void init(aos::mmu::RomMappings&);

		void enableRom1(const bool flag) { _rom1_enabled = flag; }
		void enableRom2(const bool flag) { _rom2_enabled = flag; }
		void selectRom(const uint8_t index) { _rom2_selected = index; }

		uint8_t read(const uint16_t) const;
		void write(const uint16_t, const uint8_t);

		uint8_t* getVRAM() { return _ram; }

	protected:
		uint8_t* _rom1{ nullptr };
		uint8_t* _rom2{ nullptr };
		uint8_t* _romamsdos{ nullptr };
		uint8_t* _ram{ nullptr };

		bool _rom1_enabled{ true };
		bool _rom2_enabled{ true };
		uint8_t _rom2_selected{ 0 };
	};
}