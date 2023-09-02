#include "pacmansystem.h"
#include "gpu/pacmangpu.h"
#include "registry.h"


namespace aos::namco
{
	class Alibaba : public PacmanSystem<PacmanGpu>
	{
	public:
		Alibaba(const vector<aos::emulator::RomConfiguration>& roms,
			const aos::emulator::GameConfiguration& game) :
			PacmanSystem(roms, game,
				PacmanGpu::create({
				.orientation = geometry_t::rotation_t::ROT90,
				.tileModel = PacmanGpu::Configuration::TileModel::PACMAN,
				.romModel = PacmanGpu::Configuration::RomModel::PACMAN,
				.spriteAddress = 0xef0 }
				))
		{
			_port0.set(0, "_JOY1_UP", true);
			_port0.set(1, "_JOY1_LEFT", true);
			_port0.set(2, "_JOY1_RIGHT", true);
			_port0.set(3, "_JOY1_DOWN", true);
			_port0.set(4, "rackadvance");
			_port0.set(5, "_COIN", true);
			_port0.set(6, "_JOY1_FIRE", true);
			_port0.set(7, "_COIN2", true);
			_port0.init(game);

			_port1.set(0, "_JOY1_UP", true);
			_port1.set(1, "_JOY1_LEFT", true);
			_port1.set(2, "_JOY1_RIGHT", true);
			_port1.set(3, "_JOY1_DOWN", true);
			_port1.set(4, "_JOY1_FIRE", true);
			_port1.set(5, "_START1", true);
			_port1.set(6, "_START2", true);
			_port1.set(7, "cabinet");
			_port1.init(game);

			_port2.set(0, "coinage");
			_port2.set(2, "lives");
			_port2.set(4, "bonus");
			_port2.set(6, "difficulty");
			_port2.set(7, "unknown");
			_port2.init(game);
		}

	protected:
		void mapping() override
		{
			_mmu.map(0, 0x3fff).name("cpu").rom();
			_mmu.map(0x4000, 0x4fff).readfn([this](const uint16_t a) { return _gpu->readVRAM(a); }).writefn([this](const uint16_t a, const uint8_t v) { _gpu->writeVRAM(a, v); });
			_mmu.map(0x5000, 0x503f).readfn([this](const uint16_t) { return _port0.get(); });
			_mmu.map(0x5040, 0x507f).readfn([this](const uint16_t) { return _port1.get(); });
			_mmu.map(0x5080, 0x50bf).readfn([this](const uint16_t) { return _port2.get(); });
			_mmu.map(0x50c0, 0x50c0).readfn([](const uint16_t) { return rand() & 0x0f; });
			_mmu.map(0x50c1, 0x50c1).readfn([this](const uint16_t) { return (_mystery++ >> 10) & 1; });
			_mmu.map(0x5003, 0x5003).writefn([this](const uint16_t, const uint8_t value) { _gpu->flip(((value & 1) == 1) ? true : false); });
			_mmu.map(0x5040, 0X504f).writefn([this](const uint16_t address, const uint8_t value) { _wsg.write(address, value); });
			_mmu.map(0x5050, 0x505f).writefn([this](const uint16_t address, const uint8_t value) { _gpu->writeSpritePos(address, value); });
			_mmu.map(0x5060, 0X506f).writefn([this](const uint16_t address, const uint8_t value) { _wsg.write(0x10 | address, value); });
			_mmu.map(0x50c2, 0x50c2).writefn([this](const uint16_t, const uint8_t value) { _interrupt_enabled = ((value & 1) == 1) ? true : false; });
			_mmu.map(0x8000, 0x8fff).name("cpu2").rom();
			_mmu.map(0x9000, 0x9fff).ram();
			_mmu.map(0xa000, 0xa7ff).name("cpu3").rom();
		}

		uint16_t _mystery{ 0 };
	};
}


static aos::RegistryHandler<aos::emulator::GameDriver> alibaba{ "alibaba", {
	.name = "Alibaba & 40 Thieves",
	.version = "Sega",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<aos::namco::Alibaba>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x1000, 0x38d701aa },
		{ "cpu", 0, 0x1000, 0x3d0e35f3 },
		{ "cpu", 0, 0x1000, 0x823bee89 },
		{ "cpu", 0, 0x1000, 0x474d032f },
		{ "cpu2", 0, 0x1000, 0x5ab315c1 },
		{ "cpu3", 0, 0x0800, 0x438d0357 },
		{ "video", 0, 0x0800, 0x85bcb8f8 },
		{ "video", 0, 0x0800, 0x38e50862 },
		{ "video", 0, 0x0800, 0xb5715c86 },
		{ "video", 0, 0x0800, 0x713086b3 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = {
		.switches = {{ "coinage", 1, "Coinage", {"Free", "1C 1C", "1C 2C", "2C 1C"}},
					 { "lives", 2, "Lives", {"1", "2", "3", "5"} },
					 { "bonus", 1, "Bonus", {"10000", "15000", "20000", "None"} },
					 { "cabinet", 1, "Cabinet", {"Cocktail", "Upright"} },
					 { "difficulty", 0, "Rack Advance", { "Normal", "Hard"} },
					 { "unknown", 0, "Rack Advance", { "Off", "On"} },
					 { "rackadvance", 1, "Rack Advance", { "On", "Off"} }
		  }
	}
} };
