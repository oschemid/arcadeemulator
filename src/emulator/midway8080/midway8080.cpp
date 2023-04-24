#include "midway8080.h"
#include "spaceinvaders.h"
#include "file.h"
#include "SDL2/SDL.h"


static ae::emulator::Emulator::registry reg("midway8080", [](const ae::emulator::Game& game) { return std::make_unique<ae::midway8080::Midway8080>(game); });


using namespace ae::midway8080;


GameBoard::GameBoard(DisplayOrientation displayOrientation) :
	_displayRotation{ displayOrientation }
{}

GameBoard::~GameBoard()
{
}

GameBoard::Ptr GameBoard::create(const string& name)
{
	return Registry<GameBoard::Ptr,std::function<GameBoard::Ptr()>>::instance().create(name)();
}

Midway8080::Midway8080(const emulator::Game& game) :
	_game(game)
{
	_cartridge = GameBoard::create(game.hardware());
	_clockPerMs = 1997;
}

Midway8080::~Midway8080()
{
	if (_memory)
		delete[] _memory;
}

ae::emulator::SystemInfo Midway8080::getSystemInfo() const
{
	if (_cartridge->displayOrientation() == GameBoard::DisplayOrientation::Horizontal)
		return ae::emulator::SystemInfo{
			.geometry = {.width = 256, .height = 224}
		};
	return ae::emulator::SystemInfo{
		.geometry = {.width = 224, .height = 256}
	};
}

void Midway8080::init(ae::display::RasterDisplay* raster)
{
	_cpu = xprocessors::Cpu::create("i8080");
	_raster = raster;
	const bool romExtended = _cartridge->romExtended();
	_memory = new uint8_t[(romExtended)? 0x6000 :0x4000]{ 0 };

	_cartridge->init(_game);

	string path = "roms/midway8080/"+_game.romsfile();
	auto files = _cartridge->romFiles();
	filemanager::readRoms(path, files, _memory);

	if (romExtended)
		_cpu->read([this](const uint16_t p) { return _memory[p & 0x7fff]; });
	else
		_cpu->read([this](const uint16_t p) { return _memory[p & 0x3fff]; });
	_cpu->write([this](const uint16_t p, const uint8_t v) { if ((p&0x3fff)>0x1fff) _memory[p&0x3fff] = v; });

	_cpu->in([this](const uint8_t p) { return _cartridge->in(p); });
	_cpu->out([this](const uint8_t p, const uint8_t v) { return _cartridge->out(p, v); });
}

uint8_t Midway8080::tick()
{
	static uint64_t deltaDisplay{ 0 };
	static uint64_t deltaController{ 0 };
	static uint8_t draw{ 0 };
	uint64_t previous = _cpu->elapsed_cycles();
	_cpu->executeOne();
	uint8_t deltaClock = _cpu->elapsed_cycles() - previous;

	if (deltaDisplay > 16641) { // 120 Hz
		if (draw) {
			updateDisplay();
			_raster->refresh();
			_cpu->interrupt(2);
		}
		else
			_cpu->interrupt(1);
		draw = 1 - draw;
		deltaDisplay -= 16641;
	}
	deltaDisplay += deltaClock;

	if (deltaController > 66566) { // 30 Hz
		_cartridge->controllerTick();
		deltaController -= 66566;
	}
	deltaController += deltaClock;
	return deltaClock;
}

void Midway8080::updateDisplay()
{
	for (int x = 0; x < 224; x++) {
		for (int y = 0; y < 256; y += 8) {
			uint8_t VRAMByte = _memory[0x2400 + (x << 5) + (y >> 3)];

			for (int bit = 0; bit < 8; bit++) {
				uint8_t CoordX;
				uint8_t CoordY;
				if (_cartridge->displayOrientation() == GameBoard::DisplayOrientation::Vertical) {
					CoordX = x;
					CoordY = (256 - 1 - (y + bit));
				}
				else {
					CoordX = y + bit;
					CoordY = x;
				}

				rgb_t color = _cartridge->color(CoordX, CoordY, (VRAMByte >> bit) & 1);
				_raster->set(CoordX, CoordY, color);
			}
		}
	}
}
