#include "pacmansystem.h"
#include "gpu/pacmangpu.h"
#include "registry.h"


namespace aos::namco
{
	class Ponpoko : public PacmanSystem<PacmanGpu>
	{
	public:
		Ponpoko(const vector<aos::emulator::RomConfiguration>& roms,
			const aos::emulator::GameConfiguration& game) :
			PacmanSystem(roms, game,
				PacmanGpu::create({
				.orientation = geometry_t::rotation_t::NONE,
				.tileModel = PacmanGpu::Configuration::TileModel::PONPOKO,
				.romModel = PacmanGpu::Configuration::RomModel::PACMAN,
				.spriteAddress = 0xff0 }
				))
		{
			_port0.set(0, "_JOY1_UP", false);
			_port0.set(1, "_JOY1_LEFT", false);
			_port0.set(2, "_JOY1_RIGHT", false);
			_port0.set(3, "_JOY1_DOWN", false);
			_port0.set(4, "_JOY1_FIRE", false);
			_port0.set(5, "_COIN", true);
			_port0.set(6, "_COIN2", true);
			_port0.set(7, "_COIN3", true);
			_port0.init(game);

			_port1 = 0x80;
			_port1.set(0, "_JOY1_UP", false);
			_port1.set(1, "_JOY1_LEFT", false);
			_port1.set(2, "_JOY1_RIGHT", false);
			_port1.set(3, "_JOY1_DOWN", false);
			_port1.set(4, "_JOY1_FIRE", false);
			_port1.set(5, "_START1", false);
			_port1.set(6, "_START2", false);
			_port1.init(game);

			_port2.set(0, "bonus");
			_port2.set(4, "lives");
			_port2.set(6, "cabinet");
			_port2.init(game);

			_port3.set(0, "coinage");
			_port3.set(6, "demosound");
			_port3.init(game);
		}

	protected:
		void mapping() override
		{
			_mmu.map(0, 0x3fff, "cpu").rom();
			_mmu.map(0x4000, 0x4fff).readfn([this](const uint16_t a) { return _gpu->readVRAM(a); }).writefn([this](const uint16_t a, const uint8_t v) { _gpu->writeVRAM(a, v); });
			_mmu.map(0x5000, 0x503f).readfn([this](const uint16_t) { return _port0.get(); });
			_mmu.map(0x5040, 0x507f).readfn([this](const uint16_t) { return _port1.get(); });
			_mmu.map(0x5080, 0x50bf).readfn([this](const uint16_t) { return _port2.get(); });
			_mmu.map(0x50c0, 0x50ff).readfn([this](const uint16_t) { return _port3.get(); });
			_mmu.map(0x5000, 0x5000).writefn([this](const uint16_t, const uint8_t value) { _interrupt_enabled = ((value & 1) == 1) ? true : false; });
			_mmu.map(0x5003, 0x5003).writefn([this](const uint16_t, const uint8_t value) { _gpu->flip(((value & 1) == 1) ? true : false); });
			_mmu.map(0x5040, 0X505f).writefn([this](const uint16_t address, const uint8_t value) { _wsg.write(address, value); });
			_mmu.map(0x5060, 0x506f).writefn([this](const uint16_t address, const uint8_t value) { _gpu->writeSpritePos(address, value); });
			_mmu.map(0x8000, 0xbfff, "cpu2").rom();
		}
	};
}


static aos::RegistryHandler<aos::emulator::GameDriver> ponpoko{ "ponpoko", {
	.name = "Ponpoko",
	.version = "Sigma",
	.main_version = true,
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<aos::namco::Ponpoko>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x1000, 0xffa3c004 },
		{ "cpu", 0, 0x1000, 0x4a496866 },
		{ "cpu", 0, 0x1000, 0x17da6ca3 },
		{ "cpu", 0, 0x1000, 0x9d39a565 },
		{ "cpu2", 0, 0x1000, 0x54ca3d7d },
		{ "cpu2", 0, 0x1000, 0x3055c7e0 },
		{ "cpu2", 0, 0x1000, 0x3cbe47ca },
		{ "cpu2", 0, 0x1000, 0x04b63fc6 },
		{ "video", 0, 0x1000, 0xb73e1a06 },
		{ "video", 0, 0x1000, 0x62069b5d },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = {
		.switches = {{ "coinage", 1, "Coinage", {"Free", "A 1/1 B 1/1", "A 2/1 B 2/1", "A 1/2 B 1/2", "A 3/1 B 3/1", "A 1/1 B 2/3", "A 1/1 B 4/5", "A 2/1 B 1/3",
												 "A 1/1 B 1/5", "A 1/1 B 1/6", "A 1/1 B 1/3", "A 2/1 B 1/5", "A 2/1 B 1/6", "A 2/1 1/1", "A 3/1 1/2", "A 3/1 B 1/4"}},
					 { "lives", 2, "Lives", {"2", "3", "4", "5"} },
					 { "bonus", 1, "Bonus", {"no", "10000 points", "30000 points", "50000 points"} },
					 { "cabinet", 1, "Cabinet", {"Cocktail", "Upright"} },
					 { "demosound", 0, "Demo Sounds", { "Off", "On"} }
		  }
	}
} };

static aos::RegistryHandler<aos::emulator::GameDriver> ponpokov{ "ponpokov", {
	.name = "Ponpoko",
	.version = "Venture Line",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<aos::namco::Ponpoko>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x1000, 0x49077667 },
		{ "cpu", 0, 0x1000, 0x5101781a },
		{ "cpu", 0, 0x1000, 0xd790ed22 },
		{ "cpu", 0, 0x1000, 0x4e449069 },
		{ "cpu2", 0, 0x1000, 0x54ca3d7d },
		{ "cpu2", 0, 0x1000, 0x3055c7e0 },
		{ "cpu2", 0, 0x1000, 0x3cbe47ca },
		{ "cpu2", 0, 0x1000, 0xb39be27d },
		{ "video", 0, 0x1000, 0xb73e1a06 },
		{ "video", 0, 0x1000, 0x62069b5d },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = {
		.switches = {{ "coinage", 1, "Coinage", {"Free", "A 1/1 B 1/1", "A 2/1 B 2/1", "A 1/2 B 1/2", "A 3/1 B 3/1", "A 1/1 B 2/3", "A 1/1 B 4/5", "A 2/1 B 1/3",
												 "A 1/1 B 1/5", "A 1/1 B 1/6", "A 1/1 B 1/3", "A 2/1 B 1/5", "A 2/1 B 1/6", "A 2/1 1/1", "A 3/1 1/2", "A 3/1 B 1/4"}},
					 { "lives", 2, "Lives", {"2", "3", "4", "5"} },
					 { "bonus", 1, "Bonus", {"no", "10000 points", "30000 points", "50000 points"} },
					 { "cabinet", 1, "Cabinet", {"Cocktail", "Upright"} },
					 { "demosound", 0, "Demo Sounds", { "Off", "On"} }
		  }
	}
} };

static aos::RegistryHandler<aos::emulator::GameDriver> candory{ "candory", {
	.name = "Ponpoko",
	.version = "Candory bootleg",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<aos::namco::Ponpoko>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x1000, 0xffa3c004 },
		{ "cpu", 0, 0x1000, 0x4a496866 },
		{ "cpu", 0, 0x1000, 0x17da6ca3 },
		{ "cpu", 0, 0x1000, 0x9d39a565 },
		{ "cpu2", 0, 0x1000, 0x54ca3d7d },
		{ "cpu2", 0, 0x1000, 0x3055c7e0 },
		{ "cpu2", 0, 0x1000, 0x3cbe47ca },
		{ "cpu2", 0, 0x1000, 0x04b63fc6 },
		{ "video", 0, 0x1000, 0x7d16bdff },
		{ "video", 0, 0x1000, 0xe08ac188 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = {
		.switches = {{ "coinage", 1, "Coinage", {"Free", "A 1/1 B 1/1", "A 2/1 B 2/1", "A 1/2 B 1/2", "A 3/1 B 3/1", "A 1/1 B 2/3", "A 1/1 B 4/5", "A 2/1 B 1/3",
												 "A 1/1 B 1/5", "A 1/1 B 1/6", "A 1/1 B 1/3", "A 2/1 B 1/5", "A 2/1 B 1/6", "A 2/1 1/1", "A 3/1 1/2", "A 3/1 B 1/4"}},
					 { "lives", 2, "Lives", {"2", "3", "4", "5"} },
					 { "bonus", 1, "Bonus", {"no", "10000 points", "30000 points", "50000 points"} },
					 { "cabinet", 1, "Cabinet", {"Cocktail", "Upright"} },
					 { "demosound", 0, "Demo Sounds", { "Off", "On"} }
		  }
	}
} };