#include <cstdint>
#include <array>
#include "tools.h"


const uint32_t Polynomial = 0xEDB88320;
constexpr auto Crc32Lookup{ []() constexpr {
    std::array<uint32_t,256> table{0};
    for (unsigned int i = 0; i <= 0xFF; i++)
    {
        uint32_t crc = i;
        for (unsigned int j = 0; j < 8; j++)
            crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
        table[i] = crc;
    }
    return table;
    }()
};

uint32_t ae::filemanager::Crc32(const uint8_t* data, size_t length)
{
	uint32_t crc = 0xFFFFFFFF;
	for (size_t i = 0; i < length; i++)
		crc = (crc >> 8) ^ Crc32Lookup[(crc & 0xFF) ^ data[i]];
	return crc ^ 0xFFFFFFFF;
}
