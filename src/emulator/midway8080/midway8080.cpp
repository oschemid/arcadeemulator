#include "midway8080.h"
#include "spaceinvaders.h"
#include "file.h"
#include "SDL2/SDL.h"


static ae::emulator::Emulator::registry reg("midway8080", [](const ae::emulator::Game& game) { return std::make_unique<ae::midway8080::Midway8080>(game); });


using namespace ae::midway8080;


Port::Port(const uint8_t v):
	_port{v} {}

void Port::set(uint8_t bit, const string& str)
{
	if (bit > 7)
		throw std::out_of_range("Port::set");
	_definition.push_back({ bit, str });
}

void Port::reset()
{
	_definition.clear();
}

void Port::init(const emulator::Game& game)
{
	for (auto& [bit, str] : _definition)
	{
		try
		{
			_port |= game.settings(str) << bit;
		}
		catch (std::out_of_range)
		{
		}
	}
}

void Port::tick(const ae::controller::ArcadeController& controller)
{
	static std::map<string, std::function<bool(const ae::controller::ArcadeController&)>> _map =
	{
		{ "_COIN", [](const ae::controller::ArcadeController& c) { return c.coin(); }},
		{ "_COIN2", [](const ae::controller::ArcadeController& c) { return c.coin2(); }},
		{ "_START1", [](const ae::controller::ArcadeController& c) { return c.button(ae::controller::ArcadeController::button_control::start1); }},
		{ "_START2", [](const ae::controller::ArcadeController& c) { return c.button(ae::controller::ArcadeController::button_control::start2); }},
		{ "_JOY1_FIRE", [](const ae::controller::ArcadeController& c) { return c.joystick1(ae::controller::ArcadeController::joystick_control::fire); }},
		{ "_JOY1_LEFT", [](const ae::controller::ArcadeController& c) { return c.joystick1(ae::controller::ArcadeController::joystick_control::left); }},
		{ "_JOY1_RIGHT", [](const ae::controller::ArcadeController& c) { return c.joystick1(ae::controller::ArcadeController::joystick_control::right); }},
		{ "_JOY1_DOWN", [](const ae::controller::ArcadeController& c) { return c.joystick1(ae::controller::ArcadeController::joystick_control::down); }},
		{ "_JOY1_UP", [](const ae::controller::ArcadeController& c) { return c.joystick1(ae::controller::ArcadeController::joystick_control::up); }},
		{ "_JOY2_LEFT", [](const ae::controller::ArcadeController& c) { return c.joystick2(ae::controller::ArcadeController::joystick_control::left); }},
		{ "_JOY2_RIGHT", [](const ae::controller::ArcadeController& c) { return c.joystick2(ae::controller::ArcadeController::joystick_control::right); }},
		{ "_JOY2_DOWN", [](const ae::controller::ArcadeController& c) { return c.joystick2(ae::controller::ArcadeController::joystick_control::down); }},
		{ "_JOY2_UP", [](const ae::controller::ArcadeController& c) { return c.joystick2(ae::controller::ArcadeController::joystick_control::up); }},
	};
	for (auto& [bit, str] : _definition)
	{
		if (_map.contains(str)) {
			if (_map[str](controller))
				_port.set(bit);
			else
				_port.reset(bit);
		}
	}
}

uint8_t Port::get() const
{
	return _port.to_ulong();
}

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

extern uint64_t getNanoSeconds(std::chrono::time_point<std::chrono::high_resolution_clock>* start);

void Midway8080::run(ae::display::RasterDisplay* raster)
{
	_raster = raster;
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
				raster->refresh();
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
				_raster->set(CoordX, CoordY, color);
			}
		}
	}
}
