#include "pacmansystemdecoder.h"


namespace aos::namco
{
	void eyes_decoder(uint8_t* memory, const size_t size)
	{
		auto lambda = [](const uint16_t address, const uint8_t value) { return  (value & 0xd7) | ((value & 0x08) << 2) | ((value & 0x20) >> 2); };
		for (size_t offset = 0; offset < size; ++offset)
		{
			memory[offset] = lambda(offset, memory[offset]);
		}
	}
	void woodpecker_decoder(uint8_t* memory, const size_t size)
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
	void pacplus_decoder(uint8_t* memory, const size_t size)
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
	void jumpshot_decoder(uint8_t* memory, const size_t size)
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
	void mspacman_decoderU7(uint8_t* memory, const size_t)
	{
		uint8_t* tmp = new uint8_t[0x1000];
		for (size_t offset = 0; offset < 0x1000; ++offset)
		{
			const size_t newoffset =
				(offset & 0xf807) |
				((offset & 0x0270) >> 1) |
				((offset & 0x0008) << 7) |
				((offset & 0x0080) << 2) |
				((offset & 0x0400) >> 3) |
				((offset & 0x0100) >> 2);

			const uint8_t value = memory[0x3000 + newoffset];
			tmp[offset] = ((value & 0x01) << 7) | ((value & 0x10) << 2) | (value & 0x20) | ((value & 0xc0) >> 3) | ((value & 0x0e) >> 1);
		}
		for (size_t offset = 0; offset < 0x1000; ++offset)
		{
			memory[0x3000 + offset] = tmp[offset];
		}
		delete[] tmp;
	}
	void mspacman_decoderU56(uint8_t* memory, const size_t)
	{
		uint8_t* tmp = new uint8_t[0x1000];
		for (size_t offset = 0; offset < 0x0800; ++offset)
		{
			const size_t newoffset =
				(offset & 0xf807) |
				((offset & 0x0400) >> 4) |
				((offset & 0x0200) >> 2) |
				((offset & 0x0180) << 2) |
				((offset & 0x0050) >> 1) |
				((offset & 0x0008) << 1) |
				((offset & 0x0020) << 3);
			const uint8_t value = memory[newoffset];

			tmp[offset] = ((value & 0x01) << 7) | ((value & 0x10) << 2) | (value & 0x20) | ((value & 0xc0) >> 3) | ((value & 0x0e) >> 1);
		}
		for (size_t offset = 0; offset < 0x0800; ++offset)
		{
			memory[offset] = tmp[offset];
		}
		for (size_t offset = 0; offset < 0x1000; ++offset)
		{
			const size_t newoffset =
				(offset & 0x807) |
				((offset & 0x400) >> 3) |
				((offset & 0x100) >> 2) |
				((offset & 0x080) << 2) |
				((offset & 0x270) >> 1) |
				((offset & 0x008) << 7);
			const uint8_t value = memory[0x0800 + newoffset];
			tmp[offset] = ((value & 0x01) << 7) | ((value & 0x10) << 2) | (value & 0x20) | ((value & 0xc0) >> 3) | ((value & 0x0e) >> 1);
		}
		for (size_t offset = 0; offset < 0x0800; ++offset)
		{
			memory[0x0800 + offset] = tmp[0x0800 + offset];
			memory[0x1000 + offset] = tmp[offset];
		}
		delete[] tmp;
	}
	void impeuropex_patch(uint8_t* memory, const size_t)
	{
		if (memory[0] != 0xc9)
		{
			memory[0x18b5] = 0x4e;
			memory[0x197d] = 0xda;
			memory[0x1a61] = 0x21;
		}
		else
		{
			memory[0x2c0d] = 0xeb;
		}
	}
	void mspacman_decoder(uint8_t* memory, const size_t size)
	{
		if (memory[0] == 0xc9)
			return;
		for (size_t i = 0x1000; i < 0x2000; i += 4)
		{
			if (!(i & 8))
			{
				uint8_t temp = memory[i + 1];
				memory[i + 1] = memory[i + 2];
				memory[i + 2] = temp;
			}
		}
	}
	std::function<void(uint8_t*, const size_t)> getDecoder(const string& name)
	{
		static std::map<string, std::function<void(uint8_t*, const size_t)>> mappings = {
			{ "eyes", eyes_decoder },
			{ "woodpecker", woodpecker_decoder },
			{ "pacmanplus", pacplus_decoder },
			{ "jumpshot", jumpshot_decoder },
			{ "mspacmanU7", mspacman_decoderU7 },
			{ "mspacmanU56", mspacman_decoderU56 },
			{ "impeuropex_patch", impeuropex_patch},
			{ "mspacman", mspacman_decoder }
		};
		auto it = mappings.find(name);
		if (it != mappings.end())
			return it->second;
		return nullptr;
	}

	uint8_t piranha_decoder(const uint8_t value)
	{
		return (value == 0xfa) ? 0x78 : value;
	}
	uint8_t nmouse_decoder(const uint8_t value)
	{
		if (value==0xbf) return 0x3c;
		if (value==0xc6) return 0x40;
		return value;
	}

	std::function<uint8_t(const uint8_t)> getInterruptDecoder(const string& name)
	{
		if (name == "piranha")
			return piranha_decoder;
		if (name == "nmouse")
			return nmouse_decoder;
		return nullptr;
	}
}
