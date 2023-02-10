#pragma once
#include "types.h"


namespace ae::gameboy
{
	enum MemoryMap : uint16_t
	{
		VRAM = 0x8000,
		EXTRA_RAM = 0xa000,
		REGISTER_SB = 0xff01,
		REGISTER_SC = 0xff02
	};
}
