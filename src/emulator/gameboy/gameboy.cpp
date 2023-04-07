#include "gameboy.h"
#include <fstream>
#include <chrono>
#include "time.h"
#include <iostream>
#include "SDL2/SDL.h"


static ae::emulator::Emulator::registry reg("gameboy", [](const ae::emulator::Game& game) { return std::make_unique<ae::gameboy::Gameboy>(game); });


ae::gameboy::Gameboy::Gameboy(const emulator::Game& game) :
	cpu(nullptr),
	_game(game)
{
	_clockPerMs = 4194;
}

ae::emulator::SystemInfo ae::gameboy::Gameboy::getSystemInfo() const
{
	return ae::emulator::SystemInfo{
		.geometry = {.width = 160, .height = 144}
	};
}

void ae::gameboy::Gameboy::init(ae::display::RasterDisplay* raster)
{
	cpu = xprocessors::Cpu::create("lr35902");
	_bootrom = std::make_shared<BootRom>(string("roms/gameboy/bootroms/dmg_rom.bin"));
	_cartridge = std::shared_ptr<Mbc>(Mbc::create(_game.romsfile()));
	_mmu = std::make_unique<Mmu>(_bootrom, _cartridge);
	_mmu->registerIoCallback([this](const uint8_t io, const uint8_t v) { _apu.callback(io, v); });
	_mmu->map(MemoryMap::REGISTER_SB, [this](const uint16_t) { return _serial.getRegister(MemoryMap::REGISTER_SB); },
		[this](const uint16_t, const uint8_t v) { _serial.setRegister(MemoryMap::REGISTER_SB, v); });
	_mmu->map(MemoryMap::REGISTER_SC, [this](const uint16_t) { return _serial.getRegister(MemoryMap::REGISTER_SC); },
		[this](const uint16_t, const uint8_t v) { _serial.setRegister(MemoryMap::REGISTER_SC, v); });
	_mmu->map(MemoryMap::REGISTER_LCDC, MemoryMap::REGISTER_LY, [this](const uint16_t a) { return _ppu.getRegister(static_cast<MemoryMap>(a)); },
		[this](const uint16_t a, const uint8_t v) { _ppu.setRegister(static_cast<MemoryMap>(a), v); });
	_mmu->map(MemoryMap::REGISTER_WY, MemoryMap::REGISTER_WX, [this](const uint16_t a) { return _ppu.getRegister(static_cast<MemoryMap>(a)); },
		[this](const uint16_t a, const uint8_t v) { _ppu.setRegister(static_cast<MemoryMap>(a), v); });

	_mmu->map(MemoryMap::VRAM, [this](const uint16_t p) { return _ppu.readVRAM(p); },
		[this](const uint16_t p, const uint8_t v) { _ppu.writeVRAM(p, v); });
	_mmu->map(MemoryMap::OAM, [this](const uint16_t p) { return _ppu.readOAM(p); },
		[this](const uint16_t p, const uint8_t v) { _ppu.writeOAM(p, v); });
	_serial.write([this](const uint16_t p, const uint8_t v) { return _mmu->write(p, v, Mmu::origin::cpu); });
	_serial.read([this](const uint16_t p) { return _mmu->read(p, Mmu::origin::cpu); });

	cpu->read([this](const uint16_t p) { return _mmu->read(p, Mmu::origin::cpu); });
	cpu->write([this](const uint16_t p, const uint8_t v) { return _mmu->write(p, v, Mmu::origin::cpu); });
	_ppu.read([this](const uint16_t p) { return _mmu->read(p, Mmu::origin::ppu); });
	_ppu.write([this](const uint16_t p, const uint8_t v) { return _mmu->write(p, v, Mmu::origin::ppu); });
	_apu.in([this](const uint8_t p) { return _mmu->in(p, Mmu::origin::apu); });
	_apu.out([this](const uint8_t p, const uint8_t v) { return _mmu->out(p, v, Mmu::origin::apu); });

	_apu.init();
	_ppu.init(raster);
}

uint8_t ae::gameboy::Gameboy::tick()
{
	uint64_t previous = cpu->elapsed_cycles();
	cpu->executeOne();
	uint64_t deltaClock = cpu->elapsed_cycles() - previous;

	for (uint8_t i = 0; i < deltaClock; i++) {
		_mmu->tick();
		_apu.tick();
		_ppu.executeOne();
		_serial.tick();
	}
	return deltaClock;
}
