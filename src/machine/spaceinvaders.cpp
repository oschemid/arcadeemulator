#include "spaceinvaders.h"
#include <fstream>
#include <chrono>
#include "time.h"
#include <iostream>
#include "SDL2/SDL.h"
#include "../ui/ui.h"


ae::machine::SpaceInvaders::SpaceInvaders() :
	memory(),
	cpu(nullptr),
	shift0(0),
	shift1(0)
{
}


const uint8_t ae::machine::SpaceInvaders::in(const uint8_t port) {
	switch (port) {
	case 1:
		return port1;
		break;
	case 2:
		return port2;
	case 3: {
		uint16_t v = (shift1 << 8) | shift0;
		return (uint8_t)((v >> (8 - shift_offset)) & 0xff); }
		  break;
	default:
		return (uint8_t)0;
		break;
	}
}

void ae::machine::SpaceInvaders::out(const uint8_t port, const uint8_t value) {
	switch (port)
	{
	case 2:
		shift_offset = value & 0x7;
		break;
	case 4:
		shift0 = shift1;
		shift1 = value;
		break;
	}
}

bool ae::machine::SpaceInvaders::init()
{
	cpu = newCpu("i8080");
	memory.allocate(0x3fff);
	cpu->link(memory);
	loadMemory();
	ae::ui::createDisplay(224, 256);
	cpu->in([this](const uint8_t p) { return in(p); });
	cpu->out([this](const uint8_t p, const uint8_t v) { out(p, v); });
	return true;
}

uint64_t getNanoSeconds(std::chrono::time_point<std::chrono::high_resolution_clock>* start) {
	auto diff = std::chrono::high_resolution_clock::now() - *start;
	return duration_cast<std::chrono::nanoseconds>(diff).count();
}

void ae::machine::SpaceInvaders::updateDisplay() {
	uint32_t ColorToDraw = 0xffff;

	for (int x = 0; x < 224; x++) {
		for (int y = 0; y < 256; y += 8) {
			uint8_t VRAMByte = memory.read(0x2400 + x * (256 >> 3) + (y >> 3));

			for (int bit = 0; bit < 8; bit++) {
				ColorToDraw = 0x0000;

				if (((VRAMByte >> bit) & 1)) {
					ColorToDraw = 0xffff;
				}

				uint8_t CoordX = x;
				uint8_t CoordY = (256 - 1 - (y + bit));
				Pixels[CoordY * 224 + CoordX] = ColorToDraw;
			}
		}
	}

	ae::ui::updateDisplay(Pixels);
	ae::ui::refresh();
}

bool ae::machine::SpaceInvaders::run()
{
	auto StartTime = std::chrono::high_resolution_clock::now();

	uint64_t CurrentTime = 0;
	uint64_t LastDraw = 0;
	uint8_t DrawFull = 0;
	uint64_t LastInput = 0;
	uint64_t LastThrottle = 0;
	uint64_t LastDisplay = 0;
	uint32_t ClocksPerMS = 2000;
	uint64_t ClockCompensation = 0;
	uint64_t ClockCount = 0;
	SDL_Event ev;

	port1 = 0b10001000;
	port2 = 0b00000000;

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
		if (CurrentTime - LastDraw > 1000000000 / 120 || LastDraw > CurrentTime) { // 120 Hz - Manage Screen (Half screen in a cycle, then end screen in another)
			LastDraw = CurrentTime;

			bool interrupt = false;
			if (DrawFull) {
				updateDisplay();
				interrupt = cpu->interrupt(2);
			}
			else
				interrupt = cpu->interrupt(1);

			//			if (interrupt)
			DrawFull = 1 - DrawFull;
		}
		if (CurrentTime - LastInput > 1000000000 / 30 || LastInput > CurrentTime) { // 30 Hz - Manage Events
			LastInput = CurrentTime;
			while (SDL_PollEvent(&ev)) {
			}
			port1 &= 0b10001000;
			port2 &= 0b00000000;

			switch (ships) {
			case 3:
				port2 |= 0b00;
				break;
			case 4:
				port2 |= 0b01;
				break;
			case 5:
				port2 |= 0b10;
				break;
			case 6:
				port2 |= 0b11;
				break;
			}
			if (Keyboard[SDL_SCANCODE_RETURN])
				port1 |= 1;
			if (Keyboard[SDL_SCANCODE_1])
				port1 |= 4;
			if (Keyboard[SDL_SCANCODE_2])
				port1 |= 2;
			if (Keyboard[SDL_SCANCODE_LEFT]) {
				port1 |= 0x20;
				port2 |= 0x20;
			}
			if (Keyboard[SDL_SCANCODE_RIGHT]) {
				port1 |= 0x40;
				port2 |= 0x40;
			}
			if (Keyboard[SDL_SCANCODE_SPACE]) {
				port1 |= 0x10;
				port2 |= 0x10;
			}
			if (Keyboard[SDL_SCANCODE_ESCAPE]) {
				return true;
			}
		}
	}
}

ae::machine::SpaceInvadersMidway::SpaceInvadersMidway() :
	SpaceInvaders()
{
}

void ae::machine::SpaceInvadersMidway::loadMemory() {
	memory.load("roms/spaceinvaders/invaders.h", 0);
	memory.load("roms/spaceinvaders/invaders.g", 0x0800);
	memory.load("roms/spaceinvaders/invaders.f", 0x1000);
	memory.load("roms/spaceinvaders/invaders.e", 0x1800);
}

ae::machine::SpaceInvadersTV::SpaceInvadersTV() :
	SpaceInvaders()
{
}

void ae::machine::SpaceInvadersTV::loadMemory() {
	memory.load("roms/spaceinvaders/tv0h.s1", 0);
	memory.load("roms/spaceinvaders/tv02.rp1", 0x0800);
	memory.load("roms/spaceinvaders/tv03.n1", 0x1000);
	memory.load("roms/spaceinvaders/tv04.m1", 0x1800);
}

const uint8_t ae::machine::SpaceInvadersTV::in(const uint8_t port) {
	if (port == 0)
		return 1;
	return ae::machine::SpaceInvaders::in(port);
}