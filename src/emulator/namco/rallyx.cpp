#include "rallyx.h"
#include "registry.h"


using namespace aos::namco;


RallyX::RallyX(const vector<aos::emulator::RomConfiguration>& roms, const aos::emulator::GameConfiguration& game) :
	PacmanSystem(roms, game,
		RallyXGpu::create())
{
	_port0.set(1, "_JOY1_FIRE");
	_port0.set(2, "_JOY1_LEFT");
	_port0.set(3, "_JOY1_RIGHT");
	_port0.set(4, "_JOY1_DOWN");
	_port0.set(5, "_JOY1_UP");
	_port0.set(6, "_START1");
	_port0.set(7, "_COIN");
	_port0.init(game);

	_port2 = 0xc0;
	_port2.set(0, "service");
	_port2.init(game);
}

RallyX::~RallyX()
{
}

void RallyX::mapping()
{
	_mmu.map(0, 0x3fff).name("cpu").rom();
	_mmu.map(0x8000, 0x8fff).readfn([this](const uint16_t a) { return _gpu->readVRAM(a); }).writefn([this](const uint16_t a, const uint8_t v) { _gpu->writeVRAM(a, v); });
	_mmu.map(0x9800, 0x9fff).ram();
	_mmu.map(0xa000, 0xa000).readfn([this](const uint16_t) { return (~_port0.get()) & 0xff; });
	_mmu.map(0xa080, 0xa080).readfn([this](const uint16_t) { return 0xff; });
	_mmu.map(0xa100, 0xa100).readfn([this](const uint16_t) { return _port2.get(); });
	_mmu.map(0xa000, 0xa00f).writefn([this](const uint16_t a, const uint8_t v) { _radarattr[a & 0x000f] = v; });
	_mmu.map(0xa181, 0xa181).writefn([this](const uint16_t, const uint8_t value) { _interrupt_enabled = ((value & 1) == 1) ? true : false; });
	_mmu.map(0xa130, 0xa130).writefn([this](const uint16_t, const uint8_t value) { _gpu->setScrollX(value); });
	_mmu.map(0xa140, 0xa140).writefn([this](const uint16_t, const uint8_t value) { _gpu->setScrollY(value); });
}


static aos::RegistryHandler<aos::emulator::GameDriver> rallyx{ "rallyx", {
	.name = "RallyX",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<aos::namco::RallyX>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x1000, 0x5882700d },
		{ "cpu", 0, 0x1000, 0xed1eba2b },
		{ "cpu", 0, 0x1000, 0x4f98dd1c },
		{ "cpu", 0, 0x1000, 0x9aacccf0 },
		{ "video", 1, 0x1000, 0x277c1de5 },
		{ "palette", 1, 0x20, 0xc7865434 },
		{ "palette", 1, 0x100, 0x834d4fda }
	},
	.configuration = {
		.switches = {{ "service", 1, "Service", {"False", "True"} }
		  }
	}
}};
