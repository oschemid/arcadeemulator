#include "taito8080.h"
#include "spaceinvaders.h"


static ae::emulator::RegistryHandler reg("taito8080", [] { return std::make_unique<ae::taito8080::Taito8080>(); });


using namespace ae::taito8080;


Cartridge::Cartridge() :
	_memory{ nullptr }
{}

Cartridge::~Cartridge()
{
	if (_memory)
		delete[] _memory;
}

UCartridge Cartridge::create(const string& name)
{
	return Registry<UCartridge>::instance().create(name);
}

Taito8080::Taito8080() :
	_cpu{ nullptr },
	_cartridge{ nullptr },
	_src{ nullptr }
{
}

ae::emulator::SystemInfo Taito8080::getSystemInfo() const
{
	return ae::emulator::SystemInfo{
		.geometry = {.width = 224, .height = 256}
	};
}

void Taito8080::init(const json& settings)
{
	_cpu = xprocessors::Cpu::create("i8080");
	_src = new uint32_t[224 * 256];

	string cartridge;
	settings.at("roms").at("id").get_to(cartridge);
	_cartridge = Cartridge::create(cartridge);
	_cartridge->init(settings);
	_cpu->read([this](const uint16_t p) { return _cartridge->read(p); });
	_cpu->write([this](const uint16_t p, const uint8_t v) { return _cartridge->write(p, v); });
	_cpu->in([this](const uint8_t p) { return _cartridge->in(p); });
	_cpu->out([this](const uint8_t p, const uint8_t v) { return _cartridge->out(p, v); });
}

extern uint64_t getNanoSeconds(std::chrono::time_point<std::chrono::high_resolution_clock>* start);

void Taito8080::run(ae::gui::RasterDisplay* raster)
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
				_cartridge->updateDisplay(_src);
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
		}
	}
}