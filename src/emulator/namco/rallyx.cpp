#include "rallyx.h"
#include "SDL2/SDL.h"
#include "file.h"


using namespace ae::namco;

static const ae::string archive = "roms/namco/rallyx.zip";


static ae::emulator::Emulator::registry reg("rallyx", [](const ae::emulator::Game& game) { return std::make_unique<ae::namco::RallyX>(game); });


RallyX::RallyX(const emulator::Game&)
{
	_clockPerMs = 3072;
}

RallyX::~RallyX()
{
}

ae::emulator::SystemInfo RallyX::getSystemInfo() const
{
	return ae::emulator::SystemInfo{
		.geometry = {.width = 256, .height = 256}
	};
}

void RallyX::initVideoRom()
{
	uint8_t* videorom = new uint8_t[0x1120];
	filemanager::readRoms(archive, { {0,"8e"}, {0x1000,"rx1-1.11n"}, {0x1020,"rx1-7.8p"} }, videorom);
	_tiles = ae::tilemap::decodeTiles(256, 8, videorom);
	
	for (uint8_t i = 0; i < 32; ++i)
	{
		const uint8_t data = videorom[0x1000 + i];
		const uint8_t r = (data & 1) * 0x21 + ((data >> 1) & 1) * 0x47 + ((data >> 2) & 1) * 0x97;
		const uint8_t g = ((data >> 3) & 1) * 0x21 + ((data >> 4) & 1) * 0x47 + ((data >> 5) & 1) * 0x97;
		const uint8_t b = ((data >> 6) & 1) * 0x21 + ((data >> 7) & 1) * 0x47;
		_palette.push_back({ .red = r, .green = g, .blue = b });
	}
	for (uint16_t i = 0; i < 256; ++i)
	{
		const uint8_t data = videorom[0x1020 + i];
		_lookup.push_back(_palette[data & 0x3f]);
	}
	delete[] videorom;
}

void RallyX::init(ae::display::RasterDisplay* raster)
{
	_cpu = xprocessors::Cpu::create("Z80");
	_display = raster;
	_rom = new uint8_t[0x4000];
	filemanager::readRoms(archive, {{0,"1b"}, {0,"rallyxn.1e"}, {0,"rallyxn.1h"}, {0,"rallyxn.1k"}}, _rom);

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
		return 0xff; //input
	if (address == 0xa080)
		return 0xfe; //input
	if (address == 0xa100)
		return 0; //dip
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
//	throw std::runtime_error("Error");
}

uint8_t RallyX::tick()
{
	static uint64_t deltaDisplay{ 0 };
	const uint64_t clockDisplay = 1000 / 60 * _clockPerMs;

	uint64_t previous = _cpu->elapsed_cycles();
	_cpu->executeOne();
	uint8_t deltaClock = _cpu->elapsed_cycles() - previous;

	if (deltaDisplay > clockDisplay) { // 60 Hz
		draw();
		_cpu->interrupt(_interrupt_vector);
		deltaDisplay -= clockDisplay;
	}
	deltaDisplay += deltaClock;

	return deltaClock;
}

void RallyX::drawBackground()
{
	for (uint16_t offset = 0; offset < 0x400; ++offset)
	{
		uint8_t tile = _videoram[0x400 + offset];
		uint8_t color = _videoram[0xc00 + offset];
		uint8_t xFlip = ~color & 0x40;
		uint8_t yFlip = color & 0x80;
		color &= 0x3f;

		int16_t x = (offset % 32) * 8;
		int16_t y = (offset / 32) * 8;

		for(int8_t sx=0;sx<8;++sx)
			for (int8_t sy = 0; sy < 8; ++sy)
			{
				auto col1 = _tiles[tile].pixel(sx, sy);
				rgb_t col = _lookup[(color<<2) + col1];
				_display->set(x + sx, y + sy, col);
			}
	}
	//INT32 sx, sy, Code, Colour, x, y, xFlip, yFlip;

	//INT32 Displacement = (rallyx) ? 1 : 0;
	//INT32 scrollx = -(xScroll - 3 * Displacement);
	//INT32 scrolly = -(yScroll + 16);

	//for (INT32 offs = 0x3ff; offs >= 0; offs--) {
	//	Code = DrvVideoRam[0x400 + offs];
	//	Colour = DrvVideoRam[0xc00 + offs];

	//	if (((Colour & 0x20) >> 5) != priority) continue;

	//	x += scrollx;
	//	y += scrolly;

	//	if (x < -7) x += 256;
	//	if (y < -7) y += 256;

	//	if (x >= nScreenWidth || y >= nScreenHeight) continue;

	//	if (xFlip) {
	//		if (yFlip) {
	//			Render8x8Tile_FlipXY_Clip(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
	//			Render8x8Tile_FlipXY_Clip(pTransDraw, Code, x - 256, y, Colour, 2, 0, DrvChars);
	//		}
	//		else {
	//			Render8x8Tile_FlipX_Clip(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
	//			Render8x8Tile_FlipX_Clip(pTransDraw, Code, x - 256, y, Colour, 2, 0, DrvChars);
	//		}
	//	}
	//	else {
	//		if (yFlip) {
	//			Render8x8Tile_FlipY_Clip(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
	//			Render8x8Tile_FlipY_Clip(pTransDraw, Code, x - 256, y, Colour, 2, 0, DrvChars);
	//		}
	//		else {
	//			Render8x8Tile_Clip(pTransDraw, Code, x, y, Colour, 2, 0, DrvChars);
	//			Render8x8Tile_Clip(pTransDraw, Code, x - 256, y, Colour, 2, 0, DrvChars);
	//		}
	//	}
	//}
}

void RallyX::draw()
{
	drawBackground();
	_display->refresh();
}
