#include "tilemap.h"


namespace ae::tilemap
{
	std::vector<Tile> decodeTiles(const uint16_t tile_count, const uint8_t tile_size, const uint8_t* src)
	{
		assert(tile_size == 8);

		static std::array<uint16_t, 8> xoffset = { 8,8,8,8,0,0,0,0 };
		static std::array<uint16_t, 8> yoffset = { 0,1,2,3,4,5,6,7 };

		// 8 8 8 8 0 0 0 0 // (80 08) (40 04) (20 02) (10 01) 
		// 9 9 9 9 1 1 1 1
		// ...
		auto psrc = src;
		std::vector<Tile> tiles;
		for (uint16_t tileno = 0; tileno < tile_count; ++tileno)
		{
			Tile tile = Tile(tile_size);
			for (uint8_t y = 0; y < tile_size; ++y) {
				for (uint8_t x = 0; x < tile_size; ++x) {
					uint8_t v = *(psrc + yoffset[y] + xoffset[x]);
					uint8_t color = 0;
					switch (x & 0x3) {
					case 0:
						color = ((v & 0x80) >> 6) | ((v & 0x08) >> 3);
						break;
					case 1:
						color = ((v & 0x40) >> 5) | ((v & 0x04) >> 2);
						break;
					case 2:
						color = ((v & 0x20) >> 4) | ((v & 0x02) >> 1);
						break;
					case 3:
						color = ((v & 0x10) >> 3) | (v & 0x01);
						break;
					}
					tile.pixel(x, y) = color;
				}
			}
			tiles.push_back(tile);
			psrc += 16;
		}
		return tiles;
	}
}