#include "spaceinvaders.h"
#include <fstream>
#include <chrono>
#include "time.h"
#include <iostream>
#include "SDL2/SDL.h"

//#define NK_INCLUDE_FIXED_TYPES
//#define NK_INCLUDE_STANDARD_IO
//#define NK_INCLUDE_STANDARD_VARARGS
//#define NK_INCLUDE_DEFAULT_ALLOCATOR
//#define NK_INCLUDE_VERTEX_BUFFER_OUTPUT
//#define NK_INCLUDE_FONT_BAKING
//#define NK_INCLUDE_DEFAULT_FONT
//#define NK_IMPLEMENTATION
//#define NK_SDL_RENDERER_IMPLEMENTATION

//#include "nuklear.h"
//#include "nuklear_sdl_renderer.h"

//nk_context* ctx;

#include "ui/ui.h"

ae::spaceinvaders::spaceinvaders() :
    memory(),
	cpu(&memory)
{
}


bool ae::spaceinvaders::init()
{
    memory.load("roms/spaceinvaders/invaders.h", 0);
    memory.load("roms/spaceinvaders/invaders.g", 0x0800);
    memory.load("roms/spaceinvaders/invaders.f", 0x1000);
    memory.load("roms/spaceinvaders/invaders.e", 0x1800);

//	SDL_SetRenderDrawColor(MainRenderer, 0x00, 0x00, 0x00, 0x00);
//	SDL_RenderClear(MainRenderer);

	ae::ui::createDisplay(224, 256);
//	ctx = nk_sdl_init(MainWindow, MainRenderer);
//	{struct nk_font_atlas* atlas;
//	nk_sdl_font_stash_begin(&atlas);
//	nk_sdl_font_stash_end(); }
	return true;
}

uint64_t getNanoSeconds(std::chrono::time_point<std::chrono::high_resolution_clock>* start) {
	auto diff = std::chrono::high_resolution_clock::now() - *start;
	return duration_cast<std::chrono::nanoseconds>(diff).count();
}

void ae::spaceinvaders::updateDisplay() {
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
	
//	if (nk_begin(ctx, "Demo", nk_rect(50, 50, 230, 250),
//				 NK_WINDOW_BORDER | NK_WINDOW_MOVABLE | NK_WINDOW_SCALABLE |
//				 NK_WINDOW_MINIMIZABLE | NK_WINDOW_TITLE))
//	{
//		nk_layout_row_dynamic(ctx, 30, 4);
//		if (nk_)
//		if (nk_option_label(ctx, "3 ships", ships == 3)) ships = 3;
//		if (nk_option_label(ctx, "4 ships", ships == 4)) ships = 4;
//		if (nk_option_label(ctx, "5 ships", ships == 5)) ships = 5;
//		if (nk_option_label(ctx, "6 ships", ships == 6)) ships = 6;
//	}
//	nk_end(ctx);
//	nk_sdl_render(NK_ANTI_ALIASING_ON);
	ae::ui::updateDisplay(Pixels);
	ae::ui::refresh();
}
void ae::spaceinvaders::run()
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
				ClockCount += cpu.executeOne();
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
				interrupt = cpu.interrupt(2);
			}
			else
				interrupt = cpu.interrupt(1);

			if (interrupt)
				DrawFull = 1 - DrawFull;
		}
		if (CurrentTime - LastInput > 1000000000 / 30 || LastInput > CurrentTime) { // 30 Hz - Manage Events
			LastInput = CurrentTime;
//			nk_input_begin(ctx);
//			while (SDL_PollEvent(&ev)) {
//				if (evt.type == SDL_QUIT) goto cleanup;
//				nk_sdl_handle_event(&ev);
//			}
//			nk_input_end(ctx);
			while (SDL_PollEvent(&ev)) {
			}
			cpu.inPort[1] &= 0b10001000;
			cpu.inPort[2] &= 0b00000000;
			switch (ships) {
			case 3:
				cpu.inPort[2] |= 0b00;
				break;
			case 4:
				cpu.inPort[2] |= 0b01;
				break;
			case 5:
				cpu.inPort[2] |= 0b10;
				break;
			case 6:
				cpu.inPort[2] |= 0b11;
				break;
			}
			if (Keyboard[SDL_SCANCODE_RETURN])
				cpu.inPort[1] |= 1;
			if (Keyboard[SDL_SCANCODE_1])
				cpu.inPort[1] |= 4;
			if (Keyboard[SDL_SCANCODE_2])
				cpu.inPort[1] |= 2;
			if (Keyboard[SDL_SCANCODE_LEFT]) {
				cpu.inPort[1] |= 0x20;
				cpu.inPort[2] |= 0x20;
			}
			if (Keyboard[SDL_SCANCODE_RIGHT]) {
				cpu.inPort[1] |= 0x40;
				cpu.inPort[2] |= 0x40;
			}
			if (Keyboard[SDL_SCANCODE_SPACE]) {
				cpu.inPort[1] |= 0x10;
				cpu.inPort[2] |= 0x10;
			}
		}
	}
}