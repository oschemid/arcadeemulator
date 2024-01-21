#include "types.h"

using aos::string;
using aos::uint8_t;
using aos::uint16_t;


void pacplus_decodeRom(const string rom, uint8_t* memory, const size_t size)
{
	static const uint8_t swap_xor_table[6][9] =
	{
		{ 7,6,5,4,3,2,1,0, 0x00 },
		{ 7,6,5,4,3,2,1,0, 0x28 },
		{ 6,1,3,2,5,7,0,4, 0x96 },
		{ 6,1,5,2,3,7,0,4, 0xbe },
		{ 0,3,7,6,4,2,1,5, 0xd5 },
		{ 0,3,4,6,7,2,1,5, 0xdd }
	};
	static const int picktable[32] =
	{
		0,2,4,2,4,0,4,2,2,0,2,2,4,0,4,2,
		2,2,4,0,4,2,4,0,0,4,0,4,4,2,4,2
	};

	if (rom != "cpu")
		return;

	uint32_t method = 0;
	const uint8_t* tbl;

	for (size_t offset = 0; offset < size; ++offset)
	{
		uint8_t value = memory[offset];

		method = picktable[
			(offset & 0x001) |
				((offset & 0x004) >> 1) |
				((offset & 0x020) >> 3) |
				((offset & 0x080) >> 4) |
				((offset & 0x200) >> 5)];

		if ((offset & 0x800) == 0x800)
			method ^= 1;

		tbl = swap_xor_table[method];
		uint8_t res = 0;
		res |= (((value >> tbl[0]) & 1) << 7);
		res |= (((value >> tbl[1]) & 1) << 6);
		res |= (((value >> tbl[2]) & 1) << 5);
		res |= (((value >> tbl[3]) & 1) << 4);
		res |= (((value >> tbl[4]) & 1) << 3);
		res |= (((value >> tbl[5]) & 1) << 2);
		res |= (((value >> tbl[6]) & 1) << 1);
		res |= ((value >> tbl[7]) & 1);
		res ^= tbl[8];

		memory[offset] = res;
	}
}

void jumpshot_decodeRom(const string rom, uint8_t* memory, const size_t size)
{
	static const uint8_t swap_xor_table[6][9] =
	{
		{ 7,6,5,4,3,2,1,0, 0x00 },
		{ 7,6,3,4,5,2,1,0, 0x20 },
		{ 5,0,4,3,7,1,2,6, 0xa4 },
		{ 5,0,4,3,7,1,2,6, 0x8c },
		{ 2,3,1,7,4,6,0,5, 0x6e },
		{ 2,3,4,7,1,6,0,5, 0x4e }
	};
	static const int picktable[32] =
	{
		0,2,4,4,4,2,0,2,2,0,2,4,4,2,0,2,
		5,3,5,1,5,3,5,3,1,5,1,5,5,3,5,3
	};

	if (rom != "cpu")
		return;

	uint32_t method = 0;
	const uint8_t* tbl;

	for (size_t offset = 0; offset < size; ++offset)
	{
		uint8_t value = memory[offset];

		method = picktable[
			(offset & 0x001) |
				((offset & 0x004) >> 1) |
				((offset & 0x020) >> 3) |
				((offset & 0x080) >> 4) |
				((offset & 0x200) >> 5)];

		if ((offset & 0x800) == 0x800)
			method ^= 1;

		tbl = swap_xor_table[method];
		uint8_t res = 0;
		res |= (((value >> tbl[0]) & 1) << 7);
		res |= (((value >> tbl[1]) & 1) << 6);
		res |= (((value >> tbl[2]) & 1) << 5);
		res |= (((value >> tbl[3]) & 1) << 4);
		res |= (((value >> tbl[4]) & 1) << 3);
		res |= (((value >> tbl[5]) & 1) << 2);
		res |= (((value >> tbl[6]) & 1) << 1);
		res |= ((value >> tbl[7]) & 1);
		res ^= tbl[8];

		memory[offset] = res;
	}
}

void woodpecker_decodeRom(const string rom, uint8_t* memory, const size_t size)
{
	if (rom == "gfx")
	{
		for (size_t i = 0; i < size; i += 8)
		{
			uint8_t tmp;
			auto f = [](uint8_t d) { return (d & 0xaf) | ((d & 0x40) >> 2) | ((d & 0x10) << 2); };
			memory[i] = f(memory[i]);
			tmp = memory[i + 1]; memory[i + 1] = f(memory[i + 4]); memory[i + 4] = f(tmp);
			memory[i + 2] = f(memory[i + 2]);
			tmp = memory[i + 3]; memory[i + 3] = f(memory[i + 6]); memory[i + 6] = f(tmp);
			memory[i + 5] = f(memory[i + 5]);
			memory[i + 7] = f(memory[i + 7]);
		}
	}
}

void eyes_decodeRom(const string rom, uint8_t* memory, const size_t size)
{
	if ((rom == "cpu")||(rom == "cpu2"))
	{
		auto lambda = [](const uint16_t address, const uint8_t value) { return  (value & 0xd7) | ((value & 0x08) << 2) | ((value & 0x20) >> 2); };
		for (size_t offset = 0; offset < size; ++offset)
		{
			memory[offset] = lambda(offset, memory[offset]);
		}
	}
	if (rom == "gfx")
	{
		woodpecker_decodeRom(rom, memory, size);
	}
}

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
