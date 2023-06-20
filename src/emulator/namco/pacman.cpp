#include "pacman.h"
#include "SDL2/SDL.h"
#include "tools.h"
#include "registry.h"

#include <iostream>



using namespace ae::namco;

static const aos::string archive = "roms/namco/puckman.zip";


static std::vector<uint16_t> xoffset8 = { 8,8,8,8,0,0,0,0 };
static std::vector<uint16_t> yoffset8 = { 0,1,2,3,4,5,6,7 };
static std::vector<uint16_t> xoffset16 = { 8,8,8,8,16,16,16,16,24,24,24,24,0,0,0,0 };
static std::vector<uint16_t> yoffset16 = { 0,1,2,3,4,5,6,7,32,33,34,35,36,37,38,39 };


Pacman::Pacman(const vector<aos::emulator::RomConfiguration>& roms, const aos::emulator::GameConfiguration& game) :
	_roms(roms)
{
	_clockPerMs = 3072;

	_port0.set(0, "_JOY1_UP", true);
	_port0.set(1, "_JOY1_LEFT", true);
	_port0.set(2, "_JOY1_RIGHT", true);
	_port0.set(3, "_JOY1_DOWN", true);
	_port0.set(4, "rackadvance");
	_port0.set(5, "_COIN", true);
	_port0.set(6, "_COIN2", true);
	_port0.set(7, "_COIN3", true);
	_port0.init(game);

	_port1.set(0, "_JOY1_UP", true);
	_port1.set(1, "_JOY1_LEFT", true);
	_port1.set(2, "_JOY1_RIGHT", true);
	_port1.set(3, "_JOY1_DOWN", true);
	_port1.set(4, "boardtest");
	_port1.set(5, "_START1", true);
	_port1.set(6, "_START2", true);
	_port1.set(7, "cabinet");
	_port1.init(game);

	_port2.set(0, "coinage");
	_port2.set(2, "lives");
	_port2.set(4, "bonus");
	_port2.set(6, "difficulty");
	_port2.set(7, "ghostname");
	_port2.init(game);
}

Pacman::~Pacman()
{
	if (_tilemap)
		delete _tilemap;
	if (_rom)
		delete[] _rom;
	if (_ram)
		delete[] _ram;
	if (_spritesxy)
		delete[] _spritesxy;
}

aos::emulator::SystemInfo Pacman::getSystemInfo() const
{
	return aos::emulator::SystemInfo{
		.geometry = {.width = 288, .height = 224, .rotation = geometry_t::rotation_t::ROT90 }
	};
}

void Pacman::initVideoRom()
{
	uint8_t* videorom = new uint8_t[0x2200];
	size_t offset = 0;
	for (const auto& rom : _roms) {
		if (rom.start == 1)
			offset += rom.rom.read(videorom + offset);
	}
	_tiles = ae::tilemap::decodeTiles(256, 8, videorom, xoffset8, yoffset8);
	_sprites = ae::tilemap::decodeTiles(0x40, 16, videorom+0x1000, xoffset16, yoffset16);

	for (uint8_t i = 0; i < 32; ++i)
	{
		const uint8_t data = videorom[0x2000 + i];
		const uint8_t r = (data & 1) * 0x21 + ((data >> 1) & 1) * 0x47 + ((data >> 2) & 1) * 0x97;
		const uint8_t g = ((data >> 3) & 1) * 0x21 + ((data >> 4) & 1) * 0x47 + ((data >> 5) & 1) * 0x97;
		const uint8_t b = ((data >> 6) & 1) * 0x51 + ((data >> 7) & 1) * 0xae;
		const uint8_t a = (i == 0) ? 0 : 0xff;
		_palette.push_back({ .red = r, .green = g, .blue = b, .alpha=a });
	}

	for (uint16_t i = 0; i < 64; ++i)
	{
		palette_t palette;
		const uint8_t data1 = videorom[0x2020 + 4 * i];
		const uint8_t data2 = videorom[0x2021 + 4 * i];
		const uint8_t data3 = videorom[0x2022 + 4 * i];
		const uint8_t data4 = videorom[0x2023 + 4 * i];
		palette.push_back(_palette[data1 & 0x3f]);
		palette.push_back(_palette[data2 & 0x3f]);
		palette.push_back(_palette[data3 & 0x3f]);
		palette.push_back(_palette[data4 & 0x3f]);

		_lookup.push_back(palette);
	}
	delete[] videorom;
}

void Pacman::init(ae::display::RasterDisplay* raster)
{
	_cpu = xprocessors::Cpu::create("Z80");
	_display = raster;
	_tilemap = new tilemap::TileMap(288, 224);
	_rom = new uint8_t[0x4000];

	size_t offset = 0;
	for (const auto& rom : _roms) {
		if (rom.start == 0)
			offset += rom.rom.read(_rom + offset);
	}

	_ram = new uint8_t[0x1000]{ 0 };
	_spritesxy = new uint8_t[0x10]{ 0 };

	_controller = ae::controller::ArcadeController::create();
	initVideoRom();

	_cpu->read([this](const uint16_t p) { return this->read(p); });
	_cpu->write([this](const uint16_t p, const uint8_t v) { this->write(p, v); });
	_cpu->in([this](const uint8_t) { return 0; });
	_cpu->out([this](const uint8_t p, const uint8_t v) { if (p == 0) _interrupt_vector = v; });
}

uint8_t Pacman::read(const uint16_t address) const
{
	if (address < 0x4000)
		return _rom[address];
	if (address < 0x5000)
		return _ram[address - 0x4000];

	if (address == 0x5000)
		return _port0.get();
	if (address == 0x5040)
		return _port1.get();
	if (address == 0x5080)
		return _port2.get();
	return 0;
}

void Pacman::write(const uint16_t address, const uint8_t value)
{
	uint16_t address_mirror = address & 0x7fff;

	if (address_mirror < 0x4000)
		return;
	if (address_mirror < 0x5000)
	{
		_ram[address_mirror - 0x4000] = value;
		return;
	}
	if (address_mirror == 0x5000)
	{
		_interrupt_enabled = ((value & 1) == 1) ? true : false;
		return;
	}
	if (address_mirror == 0x5003)
	{
		_flip = ((value & 1) == 1) ? true: false;
		return;
	}
	if (address_mirror < 0x5060)
		return;
	if (address_mirror < 0x5070)
	{
		_spritesxy[address_mirror - 0x5060] = value;
		return;
	}
}

uint8_t Pacman::tick()
{
	static uint64_t deltaDisplay{ 0 };
	const uint64_t clockDisplay = 1000 / 60 * _clockPerMs;

	uint64_t previous = _cpu->elapsed_cycles();
	_cpu->executeOne();
	uint64_t deltaClock = _cpu->elapsed_cycles() - previous;

	if (deltaDisplay > clockDisplay) { // 60 Hz
		draw();
		if (_interrupt_enabled)
			_cpu->interrupt(_interrupt_vector);
		deltaDisplay -= clockDisplay;
		_controller->tick(); _port0.tick(*_controller); _port1.tick(*_controller);
	}
	deltaDisplay += deltaClock;

	return static_cast<uint8_t>(deltaClock);
}

void Pacman::drawBackground(const uint8_t priority)
{
	uint16_t address = 0x02;

	// bottom of screen
	for (uint16_t i = 0; i < 56; ++i) {
		const uint8_t tile = _ram[address];
		const uint8_t palette = _ram[address + 0x400] & 0x3f;
		const uint16_t x = (_flip) ? 1 - i / 28 : 34 + i / 28;
		const uint16_t y = (_flip) ? 27 - (i % 28) : i % 28;
		_tilemap->drawTile(*_display, _tiles[tile], x * 8, y * 8, _lookup[palette], (_flip)? true : false, (_flip)? true : false);
		address++;
		if (i == 27) address += 4;
	}
	address += 2;

	// middle of screen
	for (uint16_t i = 0; i < 28 * 32; ++i) {
		const uint8_t tile = _ram[address];
		const uint8_t palette = _ram[address + 0x400] & 0x3f;
		const uint16_t x = (_flip) ? 33 - i % 32 : 2 + i % 32;
		const uint16_t y = (_flip) ? 27 - (i / 32) : i / 32;
		_tilemap->drawTile(*_display, _tiles[tile], x * 8, y * 8, _lookup[palette], (_flip) ? true : false, (_flip) ? true : false);
		address++;
	}

	address += 2;
	// top of screen
	for (uint16_t i = 0; i < 56; ++i) {
		const uint8_t tile = _ram[address];
		const uint8_t palette = _ram[address + 0x400] & 0x3f;
		const uint16_t x = (_flip) ? 35 - i / 28 : i / 28;
		const uint16_t y = (_flip) ? 27 - (i % 28) : i % 28;
		_tilemap->drawTile(*_display, _tiles[tile], x * 8, y * 8, _lookup[palette], (_flip) ? true : false, (_flip) ? true : false);
		address++;
		if (i == 27) address += 4;
	}
}

void Pacman::drawSprites()
{
	// sprites
	for (int i = 7; i >= 0; --i) {
		const uint8_t spriteflips = _ram[0x0ff0 + 2 * i];
		const uint8_t palette = _ram[0x0ff1 + 2 * i] & 0x3f;
		const uint16_t x = _spritesxy[2 * i];
		const uint16_t y = _spritesxy[1 + 2 * i];
		_tilemap->drawMaskTile(*_display, _sprites[spriteflips >> 2], 272-y, x-31, _lookup[palette], (spriteflips & 1) > 0, (spriteflips & 2) > 0);
	}
}

void Pacman::draw()
{
	drawBackground(0);
	drawSprites();
	_display->refresh();
}

static ae::RegistryHandler<aos::emulator::GameDriver> pacman{ "pacman", {
	.name = "Pacman",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const vector<aos::emulator::RomConfiguration>& roms) { return std::make_unique<ae::namco::Pacman>(roms, config); },
	.roms = {
		{ 0, 0x1000, 0xc1e6ab10 },
		{ 0, 0x1000, 0x1a6fb2d4 },
		{ 0, 0x1000, 0xbcdd1beb },
		{ 0, 0x1000, 0x817d94e3 },
		{ 1, 0x1000, 0x0c944964 },
		{ 1, 0x1000, 0x958fedf9 },
		{ 1, 0x20, 0x2fc650bd },
		{ 1, 0x100, 0x3eb3a8e4 }
	},
	.configuration = {
		.switches = {{ "coinage", 1, "Coinage", {"Free", "1C/1C", "1C/2C", "2C/1C"} },
					 { "lives", 2, "Lives", {"1", "2", "3", "5"} },
					 { "bonus", 0, "Bonus", {"10000 points", "15000 points", "20000 points", "no"} },
					 { "difficulty", 1, "Difficulty", {"Hard", "Normal"} },
					 { "ghostname", 1, "Ghost names", {"Alternate", "Normal"} },
					 { "rackadvance", 1, "Rackadvance", {"On", "Off"} },
					 { "boardtest", 1, "Board test", {"On", "Off"} },
					 { "cabinet", 1, "Cabinet", {"Table", "Upright"} }
		  }
	}
}};
