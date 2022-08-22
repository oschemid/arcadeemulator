#include "pacman.h"
#include <fstream>
#include <chrono>
#include "time.h"
#include <iostream>
#include "SDL2/SDL.h"
#include "../ui/ui.h"


ae::machine::Pacman::Pacman() :
	memory(nullptr),
	cpu(nullptr),
	display(nullptr),
	interrupt_enabled(false),
	sound_enabled(false),
	flip_screen(false)
{
}

bool ae::machine::Pacman::writeMemory(const uint16_t p, const uint8_t v) {
	if (p < 0x5000)
		return memory->write(p, v);
	if ((p >= 0x50C0) && (p <= 0x50FF))
		// Watchdog
		return true;
	switch (p) {
	case 0x5000:
		interrupt_enabled = ((v & 1) == 1) ? true : false;
		break;
	case 0x5001:
		sound_enabled = ((v & 1) == 1) ? true : false;
		break;
	case 0X5002:
	case 0X5004:
	case 0x5005:
	case 0x5006:
	case 0x5007:
		break;
	case 0x5003:
		flip_screen = ((v & 1) == 1) ? true : false;
		break;
	default:
		return false;
	}
	return true;
}
bool ae::machine::Pacman::init()
{
	if (!display) {
		display = Display::create();
		display->setSize(224, 256);
		display->registerCallback([this](uint16_t* p) { return this->updateDisplay(p); });
		display->init();
	}

	cpu = Cpu::create("Z80");
	memory = newMemory(0x5000);
	loadMemory();

	cpu->in([this](const uint8_t p) { return 0; });
	cpu->out([this](const uint8_t p, const uint8_t v) { if (p == 0) interrupt_vector = v; return; });
	cpu->read([this](const uint16_t p) { return memory->read(p); });
	cpu->write([this](const uint16_t p, const uint8_t v) { return this->writeMemory(p, v); });
	return true;
}


void ae::machine::Pacman::updateDisplay(uint16_t* pixels) {
}

extern uint64_t getNanoSeconds(std::chrono::time_point<std::chrono::high_resolution_clock>* start);

bool ae::machine::Pacman::run()
{
	auto StartTime = std::chrono::high_resolution_clock::now();

	uint64_t CurrentTime = 0;
	uint64_t LastDraw = 0;
	uint8_t DrawFull = 0;
	uint64_t LastInput = 0;
	uint64_t LastThrottle = 0;
	uint64_t LastDisplay = 0;
	uint32_t ClocksPerMS = 3720;
	uint64_t ClockCompensation = 0;
	uint64_t ClockCount = 0;
	SDL_Event ev;

	const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

	while (0 == 0) {
		CurrentTime = getNanoSeconds(&StartTime);
		if (CurrentTime - LastThrottle < 1000000) {		// 1ms
			if (ClockCount < ClocksPerMS + ClockCompensation)
				ClockCount += cpu->executeOne();
		}
		else {
			ClockCompensation += ClocksPerMS * (CurrentTime - LastThrottle) / 1000000;
			LastThrottle = CurrentTime;
		}
		if (CurrentTime - LastDisplay > 1000000000) {
			float a = (float)ClockCount / (CurrentTime / 1000);
			std::cout << a << std::endl;
			LastDisplay = CurrentTime;
		}
		if (CurrentTime - LastDraw > 1000000000 / 60 || LastDraw > CurrentTime) {
			LastDraw = CurrentTime;

			bool interrupt = false;
			if (interrupt_enabled)
				interrupt = cpu->interrupt(interrupt_vector);
		}
		/*		if (CurrentTime - LastInput > 1000000000 / 30 || LastInput > CurrentTime) { // 30 Hz - Manage Events
					LastInput = CurrentTime;
					while (SDL_PollEvent(&ev)) {
					}
					if (Keyboard[SDL_SCANCODE_ESCAPE]) {
						return true;
					}
				}*/
	}
}

void ae::machine::Pacman::loadMemory() {
	memory->map(0, 0x3fff, ae::IMemory::type::ROM);
	memory->map(0x4000, 0x4fff, ae::IMemory::type::RAM);
	memory->load(0, "roms/pacman/pacman.6e");
	memory->load(0x1000, "roms/pacman/pacman.6f");
	memory->load(0x2000, "roms/pacman/pacman.6h");
	memory->load(0x3000, "roms/pacman/pacman.6j");
}
