#include "midway8080.h"
#include "spaceinvaders.h"
#include "file.h"


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
	_src{ nullptr },
	_game(game)
{
	_cartridge = GameBoard::create(game.hardware());
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

void Midway8080::init()
{
	_cpu = xprocessors::Cpu::create("i8080");
	const bool romExtended = _cartridge->romExtended();
	_memory = new uint8_t[(romExtended)? 0x6000 :0x4000]{ 0 };
	_src = new uint32_t[224 * 256];

	_cartridge->init(_game);

	string path = "roms/midway8080/"+_game.romsfile();
	auto files = _cartridge->romFiles();
	filemanager::readRoms(path, files, _memory);

	if (romExtended)
		_cpu->read([this](const uint16_t p) { return _memory[p&0x7fff]; });
	else
		_cpu->read([this](const uint16_t p) { return _memory[p & 0x3fff]; });
	_cpu->write([this](const uint16_t p, const uint8_t v) { if ((p&0x3fff)>0x1fff) _memory[p&0x3fff] = v; });

	_cpu->in([this](const uint8_t p) { return _cartridge->in(p); });
	_cpu->out([this](const uint8_t p, const uint8_t v) { return _cartridge->out(p, v); });
}

extern uint64_t getNanoSeconds(std::chrono::time_point<std::chrono::high_resolution_clock>* start);

void Midway8080::run(ae::gui::RasterDisplay* raster)
{
	auto StartTime = std::chrono::high_resolution_clock::now();

	uint64_t CurrentTime = 0;
	uint64_t LastDraw = 0;
	uint8_t DrawFull = 0;
	uint64_t LastInput = 0;
	uint64_t LastThrottle = 0;
	uint64_t LastDisplay = 0;
	uint32_t ClocksPerMS = 1997;
	uint64_t ClockCompensation = 0;
	uint64_t ClockCount = 0;
	SDL_Event ev;

	const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

	while (0 == 0) {
		CurrentTime = getNanoSeconds(&StartTime);
		if (CurrentTime - LastThrottle < 1000000) {		// 1ms
			if (ClockCount < ClocksPerMS + ClockCompensation)
				ClockCount += _cpu->executeOne();
		}
		else {
			ClockCompensation += ClocksPerMS * (CurrentTime - LastThrottle) / 1000000;
			LastThrottle = CurrentTime;
		}
		if (CurrentTime - LastDraw > 1000000000 / 120 || LastDraw > CurrentTime) { // 120 Hz - Manage Screen (Half screen in a cycle, then end screen in another)
			LastDraw = CurrentTime;

			bool interrupt = false;
			if (DrawFull) {
				updateDisplay();
				raster->refresh((uint8_t*)_src);
				interrupt = _cpu->interrupt(2);
			}
			else
				interrupt = _cpu->interrupt(1);
			DrawFull = 1 - DrawFull;
		}
		if (CurrentTime - LastInput > 1000000000 / 30 || LastInput > CurrentTime) { // 30 Hz - Manage Events
			LastInput = CurrentTime;
			while (SDL_PollEvent(&ev)) {
			}
			if (Keyboard[SDL_SCANCODE_ESCAPE]) {
				return;
			}
			_cartridge->controllerTick();
		}
	}
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
				if (_cartridge->displayOrientation() == GameBoard::DisplayOrientation::Vertical) {
					_src[CoordY * 224 + CoordX] = 0xff000000 + (color.blue << 16) + (color.green << 8) + color.red;
				}
				else
				{
					_src[CoordY * 256 + CoordX] = 0xff000000 + (color.blue << 16) + (color.green << 8) + color.red;
				}
			}
		}
	}
}
