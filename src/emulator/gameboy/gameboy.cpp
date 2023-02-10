#include "gameboy.h"
#include <fstream>
#include <chrono>
#include "time.h"
#include <iostream>
#include "SDL2/SDL.h"


static ae::emulator::RegistryHandler reg("gameboy", [] { return std::make_unique<ae::gameboy::Gameboy>(); });


ae::gameboy::Gameboy::Gameboy() :
	cpu(nullptr)
{
}

ae::emulator::SystemInfo ae::gameboy::Gameboy::getSystemInfo() const
{
	return ae::emulator::SystemInfo{
		.geometry = {.width = 160, .height = 144}
	};
}

void ae::gameboy::Gameboy::init(const json& settings)
{
	cpu = xprocessors::Cpu::create("lr35902");
	_bootrom = std::make_shared<BootRom>(string("roms/gameboy/bootroms/dmg_rom.bin"));
	_cartridge = std::shared_ptr<Mbc>(Mbc::create(settings.at("roms")));
	_mmu = std::make_unique<Mmu>(_bootrom, _cartridge);
	_mmu->registerIoCallback([this](const uint8_t io, const uint8_t v) { _apu.callback(io, v); });
	_mmu->map(MemoryMap::REGISTER_SB, [this](const uint16_t p) { return _serial.in(p); },
		[this](const uint16_t p, const uint8_t v) { _serial.out(p, v); });
	_mmu->map(MemoryMap::REGISTER_SC, [this](const uint16_t p) { return _serial.in(p); },
		[this](const uint16_t p, const uint8_t v) { _serial.out(p, v); });
	_mmu->map(MemoryMap::VRAM, [this](const uint16_t p) { return _ppu.readVRAM(p); },
		[this](const uint16_t p, const uint8_t v) { _ppu.writeVRAM(p, v); });
	_serial.write([this](const uint16_t p, const uint8_t v) { return _mmu->write(p, v, Mmu::origin::cpu); });

	cpu->read([this](const uint16_t p) { return _mmu->read(p, Mmu::origin::cpu); });
	cpu->write([this](const uint16_t p, const uint8_t v) { return _mmu->write(p, v, Mmu::origin::cpu); });
	_ppu.read([this](const uint16_t p) { return _mmu->read(p, Mmu::origin::ppu); });
	_ppu.write([this](const uint16_t p, const uint8_t v) { return _mmu->write(p, v, Mmu::origin::ppu); });
	_apu.in([this](const uint8_t p) { return _mmu->in(p, Mmu::origin::apu); });
	_apu.out([this](const uint8_t p, const uint8_t v) { return _mmu->out(p, v, Mmu::origin::apu); });

	_apu.init();
}

extern uint64_t getNanoSeconds(std::chrono::time_point<std::chrono::high_resolution_clock>* start);

void ae::gameboy::Gameboy::run(ae::gui::RasterDisplay* raster)
{
	_ppu.init(raster);

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
					_serial.tick();
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
			if (Keyboard[SDL_SCANCODE_ESCAPE]) {
				return;
			}
		}
	}
}
