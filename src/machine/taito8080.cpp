#include "taito8080.h"
#include <fstream>
#include <chrono>
#include "time.h"
#include <iostream>
#include "SDL2/SDL.h"
#include "../ui/ui.h"

const ae::Layout::zones ae::machine::Taito8080::invaders_layout = {
		{{255,255,255}, {0,0,224,260}},
		{{32, 255, 32}, {0,184,224,240}},
		{{32,255,32}, {16,240,134,260}},
		{{255,32,32}, {0,32,224,62}}
};


ae::machine::Taito8080::Taito8080(const size_t memSize,
								  const ae::Layout::zones zones) :
	memory(nullptr),
	cpu(nullptr),
	display(nullptr),
	layout(nullptr),
	shift0(0),
	shift1(0),
	_memorySize(memSize),
	_zones(zones),
	shift_offset(0)
{
}

void ae::machine::Taito8080::out(const uint8_t port, const uint8_t value) {
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

bool ae::machine::Taito8080::init()
{
	if (!display) {
		display = Display::create();
		display->setSize(224, 256);
		display->registerCallback([this](uint16_t* p) { return this->updateDisplay(p); });
		display->init();
	}
	if ((!layout) && (_zones.size() > 0)) {
		layout = Layout::create();
		layout->setSize(224, 256);
		layout->setZones(_zones);
		layout->init();
	}

	cpu = Cpu::create("i8080");
	memory = newMemory(_memorySize);
	loadMemory();

	cpu->in([this](const uint8_t p) { return in(p); });
	cpu->out([this](const uint8_t p, const uint8_t v) { out(p, v); });
	cpu->read([this](const uint16_t p) { return memory->read(p); });
	cpu->write([this](const uint16_t p, const uint8_t v) { return memory->write(p, v); });
	return true;
}

uint64_t getNanoSeconds(std::chrono::time_point<std::chrono::high_resolution_clock>* start) {
	auto diff = std::chrono::high_resolution_clock::now() - *start;
	return duration_cast<std::chrono::nanoseconds>(diff).count();
}

void ae::machine::Taito8080::updateDisplay(uint16_t* pixels) {
	uint32_t ColorToDraw = 0xffff;

	for (int x = 0; x < 224; x++) {
		for (int y = 0; y < 256; y += 8) {
			uint8_t VRAMByte = memory->read(0x2400 + (x << 5) + (y >> 3));

			for (int bit = 0; bit < 8; bit++) {
				ColorToDraw = 0x0000;

				if (((VRAMByte >> bit) & 1)) {
					ColorToDraw = 0xffff;
				}

				uint8_t CoordX = x;
				uint8_t CoordY = (256 - 1 - (y + bit));
				pixels[CoordY * 224 + CoordX] = ColorToDraw;
			}
		}
	}
}

bool ae::machine::Taito8080::run()
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
				SDL_Renderer* renderer = ae::ui::getRenderer();
				SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
				SDL_RenderClear(renderer);

				SDL_Rect rect;
				rect.x = 512 - 224;
				rect.y = 384 - 256;
				rect.w = 224 * 2;
				rect.h = 256 * 2;

				display->update(rect);
				if (layout)
					layout->update(rect);
				SDL_RenderPresent(renderer);
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
