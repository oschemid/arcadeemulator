#pragma once

#include "types.h"
#include <cassert>
#include "device/rasterdisplay.h"
#include <vector>

using aos::palette_t;


namespace aos::tilemap
{
	class Tile
	{
	public:
		Tile(uint8_t size): _size{ size }
		{
			_pixels.resize(_size, std::vector<uint8_t>(_size, 0));
		}
		uint8_t size() const
		{
			return _size;
		}
		uint8_t pixel(const uint8_t x, const uint8_t y) const
		{
			assert(x < _size);
			assert(y < _size);
			return _pixels[y][x];
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

	using Tiles = std::vector<Tile>;

	class TileMap
	{
	public:
		TileMap(const uint16_t w, const uint16_t h) : _width{ w }, _height{ h }
		{
		}

		void drawTile(aos::device::RasterDisplay&, const Tile&, const uint16_t, const uint16_t, const palette_t, const bool, const bool);
		void drawMaskTile(aos::device::RasterDisplay&, const Tile&, const uint16_t, const uint16_t, const palette_t, const bool, const bool);

	protected:
		uint16_t _width{ 0 };
		uint16_t _height{ 0 };
	};

	Tiles decodeTiles(const uint16_t, const uint8_t, const uint8_t*, const std::vector<uint16_t>&, const std::vector<uint16_t>&);
}