#include "rallyxgpu.h"

#include <ranges>


using namespace aos::namco;


static std::vector<uint16_t> xoffset8 = { 8,8,8,8,0,0,0,0 };
static std::vector<uint16_t> yoffset8 = { 0,1,2,3,4,5,6,7 };
static std::vector<uint16_t> xoffset16 = { 8,8,8,8,16,16,16,16,24,24,24,24,0,0,0,0 };
static std::vector<uint16_t> yoffset16 = { 0,1,2,3,4,5,6,7,32,33,34,35,36,37,38,39 };


RallyXGpu::RallyXGpu() :
	TileGpu()
{
}


RallyXGpu::~RallyXGpu()
{
}


void RallyXGpu::init(aos::display::RasterDisplay* raster, const vector<aos::emulator::RomConfiguration>& roms)
{
	TileGpu::init(raster, roms);

	initPalettes(roms);
	initTilesSprites(roms);
}


void RallyXGpu::initPalettes(const vector<aos::emulator::RomConfiguration>& roms)
{
	uint8_t* paletterom = new uint8_t[0x120];
	size_t offset = 0;
	for (const auto& rom : roms | std::ranges::views::filter([](const auto i) { return i.region == "palette"; })) {
		offset += rom.rom.read(paletterom + offset);
	}

	palette_t colors;
	colors.reserve(32);
	for (uint8_t i = 0; i < 32; ++i)
	{
		const uint8_t data = paletterom[i];
		const uint8_t r = (data & 1) * 0x21 + ((data >> 1) & 1) * 0x47 + ((data >> 2) & 1) * 0x97;
		const uint8_t g = ((data >> 3) & 1) * 0x21 + ((data >> 4) & 1) * 0x47 + ((data >> 5) & 1) * 0x97;
		const uint8_t b = ((data >> 6) & 1) * 0x51 + ((data >> 7) & 1) * 0xae;
		const uint8_t a = (i == 0) ? 0 : 0xff;
		colors.push_back({ .red = r, .green = g, .blue = b, .alpha = a });
	}

	for (uint16_t i = 0; i < 64; ++i)
	{
		palette_t palette;
		const uint8_t data1 = paletterom[0x20 + 4 * i];
		const uint8_t data2 = paletterom[0x21 + 4 * i];
		const uint8_t data3 = paletterom[0x22 + 4 * i];
		const uint8_t data4 = paletterom[0x23 + 4 * i];
		palette.push_back(colors[data1 & 0x3f]);
		palette.push_back(colors[data2 & 0x3f]);
		palette.push_back(colors[data3 & 0x3f]);
		palette.push_back(colors[data4 & 0x3f]);

		_palettes.push_back(palette);
	}
	delete[] paletterom;
}

void RallyXGpu::initTilesSprites(const vector<aos::emulator::RomConfiguration>& roms)
{

	uint8_t* videorom = new uint8_t[0x2000];
	size_t offset = 0;
	for (const auto& rom : roms | std::ranges::views::filter([](const auto i) { return i.region == "video"; })) {
		offset += rom.rom.read(videorom + offset);
	}
	const uint16_t sprites_offset = (offset == 0x2000) ? 0x1000 : 0;

	_tiles = aos::tilemap::decodeTiles(256, 8, videorom, xoffset8, yoffset8);
	_sprites = aos::tilemap::decodeTiles(0x40, 16, videorom + sprites_offset, xoffset16, yoffset16);

	delete[] videorom;
}

void RallyXGpu::draw()
{
	drawBackground(0);
	drawSprites();
	drawBackground(1);
	drawSidebar();
	TileGpu::draw();
}

void RallyXGpu::drawBackground(const uint8_t layer)
{
	int16_t scrollX = 3 - _scrollX;
	int16_t scrollY = -_scrollY - 16;

	for (uint16_t offset = 0; offset < 0x400; ++offset)
	{
		uint8_t tile = _vram[0x400 + offset];
		uint8_t color = _vram[0xc00 + offset];
		if ((color & 0x20) >> 5 != layer)
			continue;
		uint8_t xFlip = ~color & 0x40;
		uint8_t yFlip = color & 0x80;
		color &= 0x3f;

		int16_t x = (offset % 32) * 8;
		x += scrollX;
		if (x < -7) x += 256;
		int16_t y = (offset / 32) * 8;
		y += scrollY;
		if (y < -7) y += 256;

		_tilemap->drawTile(*_display, _tiles[tile], x, y, _palettes[color], xFlip>0, yFlip>0);
		_tilemap->drawTile(*_display, _tiles[tile], x-256, y, _palettes[color], xFlip>0, yFlip>0);
	}
}

void RallyXGpu::drawSprites()
{
	for (uint16_t offset = 0x14; offset < 0x20; offset+=2)
	{
		uint8_t tile = (_vram[offset] & 0xfc) >> 2;
		uint8_t xFlip = _vram[offset] & 1;
		uint8_t yFlip = _vram[offset] & 2;
		uint8_t color = _vram[0x800 + offset+1]& 0x3f;
		int16_t x = _vram[offset + 1] + ((_vram[0x800 + offset + 1] & 0x80) << 1) - 1;
		int16_t y = 241 - _vram[0x800 + offset] - 1 - 16;

		_tilemap->drawMaskTile(*_display, _sprites[tile], x, y, _palettes[color], xFlip > 0, yFlip > 0);
	}
}

void RallyXGpu::drawSidebar()
{
	for (uint16_t my = 0; my < 32; my++)
	{
		for (uint16_t mx = 0; mx < 8; mx++)
		{
			uint16_t offset = mx + (my << 5);
			uint8_t tile = _vram[offset];
			uint8_t color = _vram[0x800 + offset];
			uint8_t xFlip = ~color & 0x40;
			uint8_t yFlip = color & 0x80;
			color &= 0x3f;

			int16_t x = mx * 8-32;
			if (x < 0) x += 64;
			x += 224;
			int16_t y = my * 8-16;
			
			_tilemap->drawTile(*_display, _tiles[tile], x, y, _palettes[color], xFlip > 0, yFlip > 0);
		}
	}
}
