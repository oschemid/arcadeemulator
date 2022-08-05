#include "spaceinvaders.h"
#include <fstream>
#include <chrono>
#include "time.h"
#include <iostream>
#include "SDL2/SDL.h"
#include "../ui/ui.h"


ae::machine::SpaceInvaders::SpaceInvaders() :
	memory(nullptr),
	cpu(nullptr),
	shift0(0),
	shift1(0),
	ships("Ships"),
	extraShip("Extra ship")
{
	ships.addAlias(0, "3");
	ships.addAlias(1, "4");
	ships.addAlias(2, "5");
	ships.addAlias(3, "6");
	extraShip.addAlias(0, "at 1500");
	extraShip.addAlias(1, "at 1000");
}


const uint8_t ae::machine::SpaceInvaders::in1() {
	uint8_t port = 0b10001000;

	const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

	if (Keyboard[SDL_SCANCODE_RETURN])
		port |= 1;
	if (Keyboard[SDL_SCANCODE_2])
		port |= 2;
	if (Keyboard[SDL_SCANCODE_1])
		port |= 4;
	if (Keyboard[SDL_SCANCODE_SPACE])
		port |= 0x10;
	if (Keyboard[SDL_SCANCODE_LEFT])
		port |= 0x20;
	if (Keyboard[SDL_SCANCODE_RIGHT])
		port |= 0x40;
	return port;
}

std::list<ae::IParameter*> ae::machine::SpaceInvaders::getParameters() const {
	return { (ae::IParameter*)&ships,
			 (ae::IParameter*)&extraShip };
}

const uint8_t ae::machine::SpaceInvaders::in2() {
	uint8_t port = 0b00000000;

	const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

	port |= ships.getValue();
	port |= extraShip.getValue() << 3;

	if (Keyboard[SDL_SCANCODE_LEFT])
		port |= 0x20;
	if (Keyboard[SDL_SCANCODE_RIGHT])
		port |= 0x40;
	if (Keyboard[SDL_SCANCODE_SPACE])
		port |= 0x10;
	return port;
}

const uint8_t ae::machine::SpaceInvaders::in(const uint8_t port) {
	switch (port) {
	case 1:
		return in1();
		break;
	case 2:
		return in2();
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
	cpu = ICpu::create("i8080");
	memory = newMemory(0x3fff);
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
			uint8_t VRAMByte = memory->read(0x2400 + x * (256 >> 3) + (y >> 3));

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
			DrawFull = 1 - DrawFull;
		}
		if (CurrentTime - LastInput > 1000000000 / 30 || LastInput > CurrentTime) { // 30 Hz - Manage Events
			LastInput = CurrentTime;
			while (SDL_PollEvent(&ev)) {
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
	memory->map(0, 0x1fff, ae::IMemory::type::ROM);
	memory->map(0x2000, 0x3fff, ae::IMemory::type::RAM);
	memory->load(0, "roms/spaceinvaders/invaders.h");
	memory->load(0x0800, "roms/spaceinvaders/invaders.g");
	memory->load(0x1000, "roms/spaceinvaders/invaders.f");
	memory->load(0x1800, "roms/spaceinvaders/invaders.e");
}

ae::machine::SpaceInvadersTV::SpaceInvadersTV() :
	SpaceInvaders()
{
}

void ae::machine::SpaceInvadersTV::loadMemory() {
	memory->map(0, 0x1fff, ae::IMemory::type::ROM);
	memory->map(0x2000, 0x3fff, ae::IMemory::type::RAM);
	memory->load(0, "roms/spaceinvaders/tv0h.s1");
	memory->load(0x0800, "roms/spaceinvaders/tv02.rp1");
	memory->load(0x1000, "roms/spaceinvaders/tv03.n1");
	memory->load(0x1800, "roms/spaceinvaders/tv04.m1");
}

const uint8_t ae::machine::SpaceInvadersTV::in(const uint8_t port) {
	if (port == 0)
		return 1;
	return ae::machine::SpaceInvaders::in(port);
}

ae::machine::SpaceChaserCV::SpaceChaserCV() :
	SpaceInvaders()
{
}

bool ae::machine::SpaceChaserCV::init()
{
	cpu = ICpu::create("i8080");
	memory = newMemory(0x5fff);
	cpu->link(memory);
	loadMemory();
	ae::ui::createDisplay(224, 256);
	cpu->in([this](const uint8_t p) { return in(p); });
	cpu->out([this](const uint8_t p, const uint8_t v) { out(p, v); });
	return true;
}

const uint8_t ae::machine::SpaceChaserCV::in1() {
	uint8_t port = 0;

	const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

	if (Keyboard[SDL_SCANCODE_RETURN])
		port |= 1;
	if (Keyboard[SDL_SCANCODE_2])
		port |= 2;
	if (Keyboard[SDL_SCANCODE_1])
		port |= 4;
	if (Keyboard[SDL_SCANCODE_DOWN])
		port |= 8;
	if (Keyboard[SDL_SCANCODE_SPACE])
		port |= 0x10;
	if (Keyboard[SDL_SCANCODE_LEFT])
		port |= 0x20;
	if (Keyboard[SDL_SCANCODE_RIGHT])
		port |= 0x40;
	if (Keyboard[SDL_SCANCODE_UP])
		port |= 0x80;
	return port;
}

void ae::machine::SpaceChaserCV::loadMemory() {
	memory->map(0, 0x1FFF, ae::IMemory::type::ROM);
	memory->map(0x2000, 0x3FFF, ae::IMemory::type::RAM);
	memory->map(0x4000, 0x5FFF, ae::IMemory::type::ROM);
	memory->load(0, "roms/spacechaser/1");
	memory->load(0x0400, "roms/spacechaser/2");
	memory->load(0x0800, "roms/spacechaser/3");
	memory->load(0x0C00, "roms/spacechaser/4");
	memory->load(0x1000, "roms/spacechaser/5");
	memory->load(0x1400, "roms/spacechaser/6");
	memory->load(0x1800, "roms/spacechaser/7");
	memory->load(0x1C00, "roms/spacechaser/8");
	memory->load(0x4000, "roms/spacechaser/9");
	memory->load(0x4400, "roms/spacechaser/10");
}

const uint8_t ae::machine::SpaceChaserCV::in(const uint8_t port) {
	if (port == 0)
		return 1;
	return ae::machine::SpaceInvaders::in(port);
}