#include "gameboy.h"
#include <iostream>


//static aos::RegistryHandler<aos::emulator::GameDriver> bombjack{ "bombjack", {
//	.name = "Bomb Jack",
//	.emulator = "gameboy",
//	.creator = [](const aos::emulator::GameConfiguration& config, const vector<aos::emulator::RomConfiguration>&) { return std::make_unique<ae::gameboy::Gameboy>("roms/gameboy/bombjack.gb"); },
//}};
//static aos::RegistryHandler<aos::emulator::GameDriver> boxxle{ "boxxle", {
//	.name = "Boxxle",
//	.emulator = "gameboy",
//	.creator = [](const aos::emulator::GameConfiguration& config, const vector<aos::emulator::RomConfiguration>&) { return std::make_unique<ae::gameboy::Gameboy>("roms/gameboy/boxxle.gb"); },
//}};

aos::gameboy::Gameboy::Gameboy(const string rom) :
	_rom(rom)
{
}

json aos::gameboy::Gameboy::getRequirements() const
{
	json requirements{
		{"display", {
			{"type", "display"},
			{"width", 160},
			{"height", 144},
			{"rotation", "NONE"} }
		}
	};
	return requirements;
}

void aos::gameboy::Gameboy::init(map<string, Device::SharedPtr> devices)
{
	_bootrom = std::make_shared<BootRom>(string("roms/gameboy/bootroms/dmg_rom.bin"));
	_cartridge = std::shared_ptr<Mbc>(Mbc::create(_rom));
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

	_ppu.read([this](const uint16_t p) { return _mmu->read(p, Mmu::origin::ppu); });
	_ppu.write([this](const uint16_t p, const uint8_t v) { return _mmu->write(p, v, Mmu::origin::ppu); });
	_apu.in([this](const uint8_t p) { return _mmu->in(p, Mmu::origin::apu); });
	_apu.out([this](const uint8_t p, const uint8_t v) { return _mmu->out(p, v, Mmu::origin::apu); });

	_apu.init();

	auto raster = static_cast<aos::device::RasterDisplay*>(devices["display"].get());
	_ppu.init(raster);
}

void aos::gameboy::Gameboy::run()
{
	_clock.reset();
	int tick = 0;

	while (0 == 0)
	{
		if (_clock.tickable())
		{
			_cpu.tick();
			uint8_t current_if = _mmu->read(0xff0f, aos::gameboy::Mmu::origin::cpu);
			_mmu->tick();
			_apu.tick();
			_ppu.executeOne();
			_serial.tick();

			const uint8_t pins = _cpu.getControlPins();
			if (pins & xprocessors::cpu::sm83::PIN_RD)
			{
				const uint16_t address = _cpu.getAddressBus();
				_cpu.setDataBus(_mmu->read(address, Mmu::origin::cpu));
			}
			if (pins & xprocessors::cpu::sm83::PIN_WR)
			{
				const uint16_t address = _cpu.getAddressBus();
				_mmu->write(address, _cpu.getDataBus(), Mmu::origin::cpu);
			}
			uint8_t next_if = _mmu->read(0xff0f, aos::gameboy::Mmu::origin::cpu);
			if ((current_if != next_if)&&(next_if>0))
				_cpu.interrupt(next_if);
		}
	}
}
