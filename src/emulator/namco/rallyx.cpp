#include "rallyx.h"
#include "SDL2/SDL.h"
#include "file.h"
#include "registry.h"
#include <iostream>



using namespace ae::namco;

static const aos::string archive = "roms/namco/rallyx.zip";

static std::vector<uint16_t> xoffset8 = { 8,8,8,8,0,0,0,0 };
static std::vector<uint16_t> yoffset8 = { 0,1,2,3,4,5,6,7 };
static std::vector<uint16_t> xoffset16 = { 8,8,8,8,16,16,16,16,24,24,24,24,0,0,0,0 };
static std::vector<uint16_t> yoffset16 = { 0,1,2,3,4,5,6,7,32,33,34,35,36,37,38,39 };


RallyX::RallyX(const aos::emulator::GameConfiguration& game)
{
	_clockPerMs = 3072;
	_port0.set(1, "_JOY1_FIRE");
	_port0.set(2, "_JOY1_LEFT");
	_port0.set(3, "_JOY1_RIGHT");
	_port0.set(4, "_JOY1_DOWN");
	_port0.set(5, "_JOY1_UP");
	_port0.set(6, "_START1");
	_port0.set(7, "_COIN");
	_port0.init(game);

	_port2.set(0, "service");
	_port2.init(game);
}

RallyX::~RallyX()
{
}

aos::emulator::SystemInfo RallyX::getSystemInfo() const
{
	return aos::emulator::SystemInfo{
		.geometry = {.width = 288, .height = 224}
	};
}

void RallyX::initVideoRom()
{
	uint8_t* videorom = new uint8_t[0x1120];
	filemanager::readRoms(archive, { {0,"8e"}, {0x1000,"rx1-1.11n"}, {0x1020,"rx1-7.8p"} }, videorom);
	_tiles = ae::tilemap::decodeTiles(256, 8, videorom, xoffset8, yoffset8);
	_sprites = ae::tilemap::decodeTiles(0x40, 16, videorom, xoffset16, yoffset16);
	for (uint8_t i = 0; i < 32; ++i)
	{
		const uint8_t data = videorom[0x1000 + i];
		const uint8_t r = (data & 1) * 0x21 + ((data >> 1) & 1) * 0x47 + ((data >> 2) & 1) * 0x97;
		const uint8_t g = ((data >> 3) & 1) * 0x21 + ((data >> 4) & 1) * 0x47 + ((data >> 5) & 1) * 0x97;
		const uint8_t b = ((data >> 6) & 1) * 0x21 + ((data >> 7) & 1) * 0x47;
		const uint8_t a = (i == 0) ? 0 : 0xff;
		_palette.push_back({ .red = r, .green = g, .blue = b, .alpha=a });
	}
	for (uint16_t i = 0; i < 64; ++i)
	{
		palette_t palette;
		const uint8_t data1 = videorom[0x1020 + 4 * i];
		const uint8_t data2 = videorom[0x1021 + 4 * i];
		const uint8_t data3 = videorom[0x1022 + 4 * i];
		const uint8_t data4 = videorom[0x1023 + 4 * i];
		palette.push_back(_palette[data1 & 0x3f]);
		palette.push_back(_palette[data2 & 0x3f]);
		palette.push_back(_palette[data3 & 0x3f]);
		palette.push_back(_palette[data4 & 0x3f]);

		_lookup.push_back(palette);
	}
	delete[] videorom;
}

void RallyX::init(ae::display::RasterDisplay* raster)
{
	_cpu = xprocessors::Cpu::create("Z80");
	_display = raster;
	_tilemap = new tilemap::TileMap(288, 224);
	_rom = new uint8_t[0x4000];
	filemanager::readRoms(archive, {{0,"1b"}, {0,"rallyxn.1e"}, {0,"rallyxn.1h"}, {0,"rallyxn.1k"}}, _rom);

	_controller = ae::controller::ArcadeController::create();
	initVideoRom();

	_videoram = new uint8_t[0x1000];
	_ram = new uint8_t[0x0800];
	_radarattr = new uint8_t[0x0f];

	_cpu->read([this](const uint16_t p) { return this->read(p); });
	_cpu->write([this](const uint16_t p, const uint8_t v) { this->write(p, v); });
	_cpu->in([this](const uint8_t) { return 0; });
	_cpu->out([this](const uint8_t p, const uint8_t v) { if (p == 0) _interrupt_vector = v; });
}

uint8_t RallyX::read(const uint16_t address) const
{
	if (address < 0x4000)
		return _rom[address];
	if ((address >= 0x8000) && (address < 0x9000))
		return _videoram[address - 0x8000];
	if ((address >= 0x9800) && (address < 0xA000))
		return _ram[address - 0x9800];
	if (address == 0xa000) 
		return (~_port0.get())&0xff; //input
	if (address == 0xa080)
		return 0xff; //input + upright
	if (address == 0xa100)
		return _port2.get();
	return 0;
	throw std::runtime_error("Error");
}
void RallyX::write(const uint16_t address, const uint8_t value)
{
	if (address < 0x8000)
		return;
	if (address < 0x9000)
	{
		// ???
		_videoram[address - 0x8000] = value;
		return;
	}
	if (address < 0x9800)
		return;
	if (address < 0xa000)
	{
		_ram[address - 0x9800] = value;
		return;
	}
	if (address < 0xa010)
	{
		_radarattr[address & 0x000f] = value;
		return;
	}
	switch (address)
	{
	case 0xa181:
		_interrupt_enabled = ((value & 1) == 1) ? true : false;
		break;
	case 0xa130:
		_scrollX = value;
		break;
	case 0xa140:
		_scrollY = value;
		break;
	}
//	throw std::runtime_error("Error");
}

uint8_t RallyX::tick()
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
		_controller->tick(); _port0.tick(*_controller);;
	}
	deltaDisplay += deltaClock;

	return static_cast<uint8_t>(deltaClock);
}

void RallyX::drawBackground(const uint8_t priority)
{
	int16_t scrollX = 3 - _scrollX;
	int16_t scrollY = -_scrollY - 16;

	for (uint16_t offset = 0; offset < 0x400; ++offset)
	{
		uint8_t tile = _videoram[0x400 + offset];
		uint8_t color = _videoram[0xc00 + offset];
		if ((color & 0x20) >> 5 != priority)
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

		_tilemap->drawTile(*_display, _tiles[tile], x, y, _lookup[color], xFlip>0, yFlip>0);
		_tilemap->drawTile(*_display, _tiles[tile], x-256, y, _lookup[color], xFlip>0, yFlip>0);
	}
}

void RallyX::drawSidebar()
{
	for (uint16_t my = 0; my < 32; my++)
	{
		for (uint16_t mx = 0; mx < 8; mx++)
		{
			uint16_t offset = mx + (my << 5);
			uint8_t tile = _videoram[offset];
			uint8_t color = _videoram[0x800 + offset];
			uint8_t xFlip = ~color & 0x40;
			uint8_t yFlip = color & 0x80;
			color &= 0x3f;

			int16_t x = mx * 8-32;
			if (x < 0) x += 64;
			x += 224;
			int16_t y = my * 8-16;
			
			_tilemap->drawTile(*_display, _tiles[tile], x, y, _lookup[color], xFlip > 0, yFlip > 0);
		}
	}
}

void RallyX::drawSprites()
{
	for (uint16_t offset = 0x14; offset < 0x20; offset+=2)
	{
		uint8_t tile = (_videoram[offset] & 0xfc) >> 2;
		uint8_t xFlip = _videoram[offset] & 1;
		uint8_t yFlip = _videoram[offset] & 2;
		uint8_t color = _videoram[0x800 + offset+1]& 0x3f;
		int16_t x = _videoram[offset + 1] + ((_videoram[0x800 + offset + 1] & 0x80) << 1) - 1;
		int16_t y = 241 - _videoram[0x800 + offset] - 1 - 16;

		_tilemap->drawMaskTile(*_display, _sprites[tile], x, y, _lookup[color], xFlip > 0, yFlip > 0);
	}
}

void RallyX::draw()
{
	drawBackground(0);
	drawSprites();
	drawBackground(1);
	drawSidebar();
	_display->refresh();
}

static ae::RegistryHandler<aos::emulator::GameDriver> rallyx{ "rallyx", {
	.name = "RallyX",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config) { return std::make_unique<ae::namco::RallyX>(config); },
	.configuration = {
		.switches = {{ "service", 1, "Service", {"False", "True"} }
		  }
	}
}};
