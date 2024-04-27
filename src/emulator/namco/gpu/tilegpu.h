#pragma once
#include "device/rasterdisplay.h"
#include "tilemap.h"

#include <vector>

namespace aos::namco
{
	class TileGpu
	{
	public:
		virtual ~TileGpu();
		virtual void init(aos::device::RasterDisplay*,
			              const std::vector<aos::mmu::RomMapping>&);
		virtual void draw();

		uint8_t readVRAM(const uint16_t p) { return _vram[p]; }
		void writeVRAM(const uint16_t p, const uint8_t v) { _vram[p] = v; }

//		vector<palette_t> getPalettes() const { return _palettes; }
//		aos::tilemap::Tiles getTiles() const { return _tiles; }
//		aos::tilemap::Tiles getSprites() const { return _sprites; }

	protected:
		TileGpu();

		aos::device::RasterDisplay* _display{ nullptr };
		uint8_t* _vram{ nullptr };

		aos::tilemap::TileMap* _tilemap{ nullptr };
		aos::tilemap::Tiles _tiles;
		aos::tilemap::Tiles _sprites;
		std::vector<palette_t> _palettes;
	};
}