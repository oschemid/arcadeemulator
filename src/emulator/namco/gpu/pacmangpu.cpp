#include "pacmangpu.h"

#include <cstdint>
#include <ranges>


using namespace aos::namco;


static std::vector<uint16_t> yoffset8 = { 0,1,2,3,4,5,6,7 };
static std::vector<uint16_t> yoffset16 = { 0,1,2,3,4,5,6,7,32,33,34,35,36,37,38,39 };
static std::vector<uint16_t> xoffset8 = { 8,8,8,8,0,0,0,0 };
static std::vector<uint16_t> xoffset16 = { 8,8,8,8,16,16,16,16,24,24,24,24,0,0,0,0 };


PacmanGpu::PacmanGpu(const PacmanGpu::Configuration& configuration) :
	TileGpu(),
	_configuration{ configuration }
{
}

PacmanGpu::~PacmanGpu()
{
	if (_spritesxy)
		delete[] _spritesxy;
}

void PacmanGpu::init(aos::device::RasterDisplay* raster,
	                 const vector<aos::mmu::RomMapping>& roms)
{
	TileGpu::init(raster, roms);

	_spritesxy = new uint8_t[0x10]{ 0 };

	initPalettes(roms);
	initTilesSprites(roms);
}

void PacmanGpu::initPalettes(const vector<aos::mmu::RomMapping>& roms)
{
	uint8_t* paletterom = new uint8_t[0x120];
	size_t offset = 0;
	for (const auto& rom : roms | std::ranges::views::filter([](const auto i) { return i.region == "palette"; })) {
		offset += rom.rom.read(paletterom + offset, rom.mapping.start, rom.mapping.size);
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
		const uint8_t data1 = paletterom[0x20 + 4 * i] & 0xf;
		const uint8_t data2 = paletterom[0x21 + 4 * i] & 0xf;
		const uint8_t data3 = paletterom[0x22 + 4 * i] & 0xf;
		const uint8_t data4 = paletterom[0x23 + 4 * i] & 0xf;
		palette.push_back(colors[data1]);
		palette.push_back(colors[data2]);
		palette.push_back(colors[data3]);
		palette.push_back(colors[data4]);

		_palettes.push_back(palette);
	}
	for (uint16_t i = 0; i < 64; ++i)
	{
		palette_t palette;
		const uint8_t data1 = paletterom[0x20 + 4 * i] & 0xf;
		const uint8_t data2 = paletterom[0x21 + 4 * i] & 0xf;
		const uint8_t data3 = paletterom[0x22 + 4 * i] & 0xf;
		const uint8_t data4 = paletterom[0x23 + 4 * i] & 0xf;
		palette.push_back(colors[data1 | 0x10]);
		palette.push_back(colors[data2 | 0x10]);
		palette.push_back(colors[data3 | 0x10]);
		palette.push_back(colors[data4 | 0x10]);

		_palettes.push_back(palette);
	}
	delete[] paletterom;
}

void PacmanGpu::initTilesSprites(const vector<aos::mmu::RomMapping>& roms)
{
	uint8_t* videorom = new uint8_t[0x2000];
	size_t offset = 0;
	for (const auto& rom : roms | std::ranges::views::filter([](const auto i) { return i.region == "video"; })) {
		offset += rom.rom.read(videorom + offset, rom.mapping.start, rom.mapping.size);
	}
	const uint16_t sprites_offset = (offset == 0x2000) ? 0x1000 : 0;

	if (_configuration.romDecoding)
		_configuration.romDecoding(videorom, 0x2000);
	if (_romDecoder)
		_romDecoder(videorom, 0x2000);

	_tiles = aos::tilemap::decodeTiles(256, 8, videorom, xoffset8, yoffset8);
	_sprites = aos::tilemap::decodeTiles(0x40, 16, videorom + sprites_offset, xoffset16, yoffset16);

	delete[] videorom;
}

void PacmanGpu::draw()
{
	drawBackground();
	drawSprites();
	TileGpu::draw();
}

void PacmanGpu::drawBackground()
{
	uint16_t address = 0x02;

	// bottom of screen
	for (uint16_t i = 0; i < 56; ++i) {
		const uint8_t tile = _vram[address];
		const uint8_t palette = _vram[address + 0x400] & 0x7f;
		const uint16_t x = (_flip) ? 1 - i / 28 : 34 + i / 28;
		const uint16_t y = (_flip) ? 27 - (i % 28) : i % 28;
		_tilemap->drawTile(*_display, _tiles[tile], x * 8, y * 8, _palettes[palette], (_flip) ? true : false, (_flip) ? true : false);
		address++;
		if (i == 27) address += 4;
	}
	address += 2;

	// middle of screen
	for (uint16_t i = 0; i < 28 * 32; ++i) {
		const uint8_t tile = _vram[address];
		const uint8_t palette = _vram[address + 0x400] & 0x1f;
		const uint16_t x = (_flip) ? 33 - i % 32 : 2 + i % 32;
		const uint16_t y = (_flip) ? 27 - (i / 32) : i / 32;
		_tilemap->drawTile(*_display, _tiles[tile], x * 8, y * 8, _palettes[palette], (_flip) ? true : false, (_flip) ? true : false);
		address++;
	}

	address += 2;
	// top of screen
	for (uint16_t i = 0; i < 56; ++i) {
		const uint8_t tile = _vram[address];
		const uint8_t palette = _vram[address + 0x400] & 0x1f;
		const uint16_t x = (_flip) ? 35 - i / 28 : i / 28;
		const uint16_t y = (_flip) ? 27 - (i % 28) : i % 28;
		_tilemap->drawTile(*_display, _tiles[tile], x * 8, y * 8, _palettes[palette], (_flip) ? true : false, (_flip) ? true : false);
		address++;
		if (i == 27) address += 4;
	}
}

void PacmanGpu::drawSprites()
{
	// sprites
	for (int i = 7; i >= 0; --i) {
		const uint8_t spriteflips = _vram[_configuration.spriteAddress + 2 * i];
		const uint8_t palette = _vram[_configuration.spriteAddress + 1 + 2 * i] & 0x1f;
		const uint16_t x = _spritesxy[2 * i];
		const uint16_t y = _spritesxy[1 + 2 * i];
		_tilemap->drawMaskTile(*_display, _sprites[spriteflips >> 2], 272 - y, x - 31, _palettes[palette], (spriteflips & 1) > 0, (spriteflips & 2) > 0);
	}
}
