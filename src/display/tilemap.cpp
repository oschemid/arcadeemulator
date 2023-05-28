#include "tilemap.h"


namespace ae::tilemap
{
	Tiles decodeTiles(const uint16_t tile_count, const uint8_t tile_size, const uint8_t* src, const std::vector<uint16_t>& xoffset, const std::vector<uint16_t>& yoffset)
	{
		auto psrc = src;
		Tiles tiles;
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
			psrc += tile_size * tile_size / 4;
		}
		return tiles;
	}

	void TileMap::drawTile(display::RasterDisplay& raster, const Tile& tile, const uint16_t x, const uint16_t y, const palette_t palette, const bool flipX, const bool flipY)
	{
		const uint8_t size = tile.size();
		if ((x >= _width) || (y >= _height))
			return;
		for (uint8_t my = 0; my < size; ++my)
		{
			int16_t yy = (flipY) ? y + size - my - 1 : y + my;

			if ((yy < 0) || (yy >= _height))
				continue;
			for (uint8_t mx = 0; mx < size; ++mx)
			{
				int16_t xx = (flipX) ? x + size - mx - 1 : x + mx;
				if ((xx < 0) || (xx >= _width))
					continue;
				rgb_t col = palette[tile.pixel(mx, my)];
				raster.set(xx, yy, col);
			}
		}
	}
	void TileMap::drawMaskTile(display::RasterDisplay& raster, const Tile& tile, const uint16_t x, const uint16_t y, const palette_t palette, const bool flipX, const bool flipY)
	{
		const uint8_t size = tile.size();
		if ((x >= _width) || (y >= _height))
			return;
		for (uint8_t my = 0; my < size; ++my)
		{
			int16_t yy = (flipY) ? y + size - my - 1 : y + my;

			if ((yy < 0) || (yy >= _height))
				continue;
			for (uint8_t mx = 0; mx < size; ++mx)
			{
				int16_t xx = (flipX) ? x + size - mx - 1 : x + mx;
				if ((xx < 0) || (xx >= _width))
					continue;
				aos::rgba_t col = palette[tile.pixel(mx, my)];
				if (col.alpha > 0) {
					raster.set(xx, yy, static_cast<rgb_t>(col));
				}
			}
		}
	}
}