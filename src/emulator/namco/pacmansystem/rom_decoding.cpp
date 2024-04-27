#include "types.h"

using aos::string;
using aos::uint8_t;
using aos::uint16_t;


void ponpoko_decodeRom(const string rom, uint8_t* memory, const size_t size)
{
	uint8_t temp;
	if (rom == "gfx")
	{
		size_t offset = 0;
		for (; offset < size / 2; offset += 0x10)
		{
			for (size_t offset2 = 0; offset2 < 8; ++offset2)
			{
				temp = memory[offset + offset2 + 0x08];
				memory[offset + offset2 + 0x08] = memory[offset+offset2];
				memory[offset + offset2] = temp;
			}
		}
		for (; offset < size; offset += 0x20)
		{
			for (size_t offset2 = 0; offset2 < 8; ++offset2)
			{
				temp = memory[offset + offset2 + 0x18];
				memory[offset + offset2 + 0x18] = memory[offset + offset2 + 0x10];
				memory[offset + offset2 + 0x10] = memory[offset + offset2 + 0x08];
				memory[offset + offset2 + 0x08] = memory[offset + offset2];
				memory[offset + offset2] = temp;
			}
		}
	}
}
