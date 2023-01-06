#include "gameboy.h"
#include <fstream>
#include <chrono>
#include "time.h"
#include <iostream>
#include "SDL2/SDL.h"
#include "../ui/ui.h"


ae::gameboy::Gameboy::Gameboy() :
	cpu(nullptr)
{
}

bool ae::gameboy::Gameboy::init()
{
	cpu = xprocessors::Cpu::create("lr35902");
	_bootrom = std::make_shared<BootRom>(string("roms/gameboy/bootroms/dmg_rom.bin"));
	_cartridge = std::shared_ptr<Mbc>(Mbc::create("roms/gameboy/tetris.gb"));
	_mmu = std::make_unique<Mmu>(_bootrom, _cartridge);
	_mmu->registerIoCallback([this](const uint8_t io, const uint8_t v) { _apu.callback(io, v); });

	cpu->read([this](const uint16_t p) { return _mmu->read(p, Mmu::origin::cpu); });
	cpu->write([this](const uint16_t p, const uint8_t v) { return _mmu->write(p, v, Mmu::origin::cpu); });
	_ppu.read([this](const uint16_t p) { return _mmu->read(p, Mmu::origin::ppu); });
	_ppu.write([this](const uint16_t p, const uint8_t v) { return _mmu->write(p, v, Mmu::origin::ppu); });
	_apu.in([this](const uint8_t p) { return _mmu->in(p, Mmu::origin::apu); });
	_apu.out([this](const uint8_t p, const uint8_t v) { return _mmu->out(p, v, Mmu::origin::apu); });

	_apu.init();
	return true;
}


extern uint64_t getNanoSeconds(std::chrono::time_point<std::chrono::high_resolution_clock>* start);

bool ae::gameboy::Gameboy::run()
{
	auto StartTime = std::chrono::high_resolution_clock::now();

	uint64_t CurrentTime = 0;
	uint64_t LastDraw = 0;
	uint8_t DrawFull = 0;
	uint64_t LastInput = 0;
	uint64_t LastThrottle = 0;
	uint64_t LastDisplay = 0;
	uint32_t ClocksPerMS = 4194304 / 1000;
	uint64_t ClockCompensation = 0;
	uint64_t ClockCount = 0;
	SDL_Event ev;

	const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);
	bool stopped = false;

	while (0 == 0) {
		CurrentTime = getNanoSeconds(&StartTime);
		if (CurrentTime - LastThrottle < 1000000) {		// 1ms
			if (ClockCount < ClocksPerMS + ClockCompensation) {
				uint64_t previousClockCount = ClockCount;
				if (!stopped)
					cpu->executeOne();
				ClockCount = cpu->elapsed_cycles();
				for (uint8_t i = 0; i < ClockCount - previousClockCount; i++) {
					_mmu->tick();
					_apu.tick();
					_ppu.executeOne();
				}
			}
		}
		else {
			ClockCompensation += ClocksPerMS * (CurrentTime - LastThrottle) / 1000000;
			LastThrottle = CurrentTime;
		}
		if (CurrentTime - LastInput > 1000000000 / 30 || LastInput > CurrentTime) { // 30 Hz - Manage Events
			LastInput = CurrentTime;
			while (SDL_PollEvent(&ev)) {
			}
			if (Keyboard[SDL_SCANCODE_F1]) {
				_ppu.switchDisplayVram();
			}
			if (Keyboard[SDL_SCANCODE_ESCAPE]) {
				return true;
			}
		}
	}
}
