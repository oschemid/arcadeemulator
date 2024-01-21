#include "tilegpu.h"


using namespace aos::namco;


TileGpu::TileGpu()
{
	_palettes.reserve(64);
}


TileGpu::~TileGpu()
{
	if (_tilemap)
		delete _tilemap;
	if (_vram)
		delete[] _vram;
}


void TileGpu::init(aos::display::RasterDisplay* raster,
	               const vector<aos::mmu::RomMapping>& roms)
{
	_display = raster;
	_tilemap = new aos::tilemap::TileMap(288, 224);
	_vram = new uint8_t[0x1000]{ 0 };
}


void TileGpu::draw()
{
	_display->refresh();
}
