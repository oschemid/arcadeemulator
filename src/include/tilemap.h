#pragma once

#include "types.h"
#include <vector>


namespace ae::tilemap
{
	class Tile
	{
	public:
		Tile(uint8_t size): _size{ size }
		{
			_pixels.resize(_size, std::vector<uint8_t>(_size, 0));
		}
		uint8_t& pixel(const uint8_t x, const uint8_t y)
		{
			assert(x < _size);
			assert(y < _size);
			return _pixels[y][x];
		}
	protected:
		uint8_t _size;
		std::vector<std::vector<uint8_t>> _pixels;
	};

	std::vector<Tile> decodeTiles(const uint16_t, const uint8_t, const uint8_t*);
}