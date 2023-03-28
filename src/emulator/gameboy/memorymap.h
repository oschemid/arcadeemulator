#pragma once
#include "types.h"


namespace ae::gameboy
{
	enum MemoryMap : uint16_t
	{
		VRAM = 0x8000,
		EXTRA_RAM = 0xa000,
		OAM = 0xfe00,
		NOTUSE = 0xfea0,
		REGISTER_SB = 0xff01,
		REGISTER_SC = 0xff02,
		REGISTER_IF = 0xff0f,

		REGISTER_LCDC = 0xff40,
		REGISTER_STAT = 0xff41,
		REGISTER_SCY = 0xff42,
		REGISTER_SCX = 0xff43,
		REGISTER_LY = 0xff44,
		REGISTER_WY = 0xff4a,
		REGISTER_WX = 0xff4b
	};

	const MemoryMap PpuRegisters[] = {REGISTER_LCDC, REGISTER_STAT, REGISTER_SCY, REGISTER_SCX, REGISTER_LY, REGISTER_WY, REGISTER_WX};
}
