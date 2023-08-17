#include "registry.h"
#include "pacmansystem.h"
#include "gpu/pacmangpu.h"


using aos::namco::PacmanSystem;
using aos::namco::PacmanGpu;


class Pacman : public PacmanSystem<PacmanGpu>
	{
	public:
		Pacman(const vector<aos::emulator::RomConfiguration>& roms,
			const aos::emulator::GameConfiguration& game) :
			PacmanSystem(roms, game,
				PacmanGpu::create({
					.orientation = geometry_t::rotation_t::ROT90,
					.tileModel = PacmanGpu::Configuration::TileModel::PACMAN,
					.romModel = PacmanGpu::Configuration::RomModel::PACMAN,
					.spriteAddress = 0xff0
				}))
		{
			_port0.set(0, "_JOY1_UP", true);
			_port0.set(1, "_JOY1_LEFT", true);
			_port0.set(2, "_JOY1_RIGHT", true);
			_port0.set(3, "_JOY1_DOWN", true);
			_port0.set(4, "rackadvance");
			_port0.set(5, "_COIN", true);
			_port0.set(6, "_COIN2", true);
			_port0.set(7, "_COIN3", true);
			_port0.init(game);

			_port1.set(0, "_JOY1_UP", true);
			_port1.set(1, "_JOY1_LEFT", true);
			_port1.set(2, "_JOY1_RIGHT", true);
			_port1.set(3, "_JOY1_DOWN", true);
			_port1.set(4, "boardtest");
			_port1.set(5, "_START1", true);
			_port1.set(6, "_START2", true);
			_port1.set(7, "cabinet");
			_port1.init(game);

			_port2.set(0, "coinage");
			_port2.set(2, "lives");
			_port2.set(4, "bonus");
			_port2.set(6, "difficulty");
			_port2.set(7, "ghostname");
			_port2.init(game);
		}

		void mapping() override
		{
			_mmu.map(0, 0x3fff, 0x7fff, "cpu").rom();
			_mmu.map(0x4000, 0x4fff, 0x7fff).readfn([this](const uint16_t a) { return _gpu->readVRAM(a); }).writefn([this](const uint16_t a, const uint8_t v) { _gpu->writeVRAM(a, v); });
			_mmu.map(0x5000, 0x503f, 0x7fff).readfn([this](const uint16_t) { return _port0.get(); });
			_mmu.map(0x5040, 0x507f, 0x7fff).readfn([this](const uint16_t) { return _port1.get(); });
			_mmu.map(0x5080, 0x50bf, 0x7fff).readfn([this](const uint16_t) { return _port2.get(); });
			_mmu.map(0x5000, 0x5000, 0x7fff).writefn([this](const uint16_t, const uint8_t value) { _interrupt_enabled = ((value & 1) == 1) ? true : false; });
			_mmu.map(0x5003, 0x5003, 0x7fff).writefn([this](const uint16_t, const uint8_t value) { _gpu->flip(((value & 1) == 1) ? true : false); });
			_mmu.map(0x5040, 0X505f, 0x7fff).writefn([this](const uint16_t address, const uint8_t value) { _wsg.write(address, value); });
			_mmu.map(0x5060, 0x506f, 0x7fff).writefn([this](const uint16_t address, const uint8_t value) { _gpu->writeSpritePos(address, value); });
		}
	};


static aos::emulator::GameConfiguration pacman_configuration = {
		.switches = {{ "coinage", 1, "Coinage", {"Free", "1C/1C", "1C/2C", "2C/1C"} },
					 { "lives", 2, "Lives", {"1", "2", "3", "5"} },
					 { "bonus", 0, "Bonus", {"10000 points", "15000 points", "20000 points", "no"} },
					 { "difficulty", 1, "Difficulty", {"Hard", "Normal"} },
					 { "ghostname", 1, "Ghost names", {"Alternate", "Normal"} },
					 { "rackadvance", 1, "Rackadvance", {"On", "Off"} },
					 { "boardtest", 1, "Board test", {"On", "Off"} },
					 { "cabinet", 1, "Cabinet", {"Table", "Upright"} }
		  }
};


static aos::RegistryHandler<aos::emulator::GameDriver> puckman{ "puckman", {
	.name = "Pacman",
	.version = "Puck Man (Japan set 1)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x0800, 0xf36e88ab },
		{ "cpu", 0, 0x0800, 0x618bd9b3 },
		{ "cpu", 0, 0x0800, 0x7d177853 },
		{ "cpu", 0, 0x0800, 0xd3e8914c },
		{ "cpu", 0, 0x0800, 0x6bf4f625 },
		{ "cpu", 0, 0x0800, 0xa948ce83 },
		{ "cpu", 0, 0x0800, 0xb6289b26 },
		{ "cpu", 0, 0x0800, 0x17a88c13 },
		{ "video", 0, 0x0800, 0x2066a0b7 },
		{ "video", 0, 0x0800, 0x3591b89d },
		{ "video", 0, 0x0800, 0x9e39323a },
		{ "video", 0, 0x0800, 0x1b1d9096 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
}};


static aos::RegistryHandler<aos::emulator::GameDriver> puckmanb{ "puckmanb", {
	.name = "Pacman",
	.version = "Puck Man (bootleg set 1)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x1000, 0xfee263b3 },
		{ "cpu", 0, 0x1000, 0x39d1fc83 },
		{ "cpu", 0, 0x1000, 0x02083b03 },
		{ "cpu", 0, 0x1000, 0x7a36fe55 },
		{ "video", 0, 0x1000, 0x0c944964 },
		{ "video", 0, 0x1000, 0x958fedf9 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
}};


static aos::RegistryHandler<aos::emulator::GameDriver> puckmanf{ "puckmanf", {
	.name = "Pacman",
	.version = "Puck Man (speedup hack)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x1000, 0xfee263b3 },
		{ "cpu", 0, 0x1000, 0x51b38db9 },
		{ "cpu", 0, 0x1000, 0x02083b03 },
		{ "cpu", 0, 0x1000, 0x7a36fe55 },
		{ "video", 0, 0x1000, 0x0c944964 },
		{ "video", 0, 0x1000, 0x958fedf9 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
}};


static aos::RegistryHandler<aos::emulator::GameDriver> puckmanh{ "puckmanh", {
	.name = "Pacman",
	.version = "Puck Man (bootleg set 2)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x1000, 0x5fe8610a },
		{ "cpu", 0, 0x1000, 0x61d38c6c },
		{ "cpu", 0, 0x1000, 0x4e7ef99f },
		{ "cpu", 0, 0x1000, 0x8939ddd2 },
		{ "video", 0, 0x800, 0x2229ab07 },
		{ "video", 0, 0x800, 0x3591b89d },
		{ "video", 0, 0x800, 0x9e39323a },
		{ "video", 0, 0x800, 0x1b1d9096 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
}};


static aos::RegistryHandler<aos::emulator::GameDriver> puckmod{ "puckmod", {
	.name = "Pacman",
	.version = "Puck Man (Japan set 2)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x1000, 0xfee263b3 },
		{ "cpu", 0, 0x1000, 0x39d1fc83 },
		{ "cpu", 0, 0x1000, 0x02083b03 },
		{ "cpu", 0, 0x1000, 0x7d98d5f5 },
		{ "video", 0, 0x1000, 0x0c944964 },
		{ "video", 0, 0x1000, 0x958fedf9 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
}};


static aos::RegistryHandler<aos::emulator::GameDriver> pacman{ "pacman", {
	.name = "Pacman",
	.version = "Pac-Man (Midway)",
	.main_version = true,
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x1000, 0xc1e6ab10 },
		{ "cpu", 0, 0x1000, 0x1a6fb2d4 },
		{ "cpu", 0, 0x1000, 0xbcdd1beb },
		{ "cpu", 0, 0x1000, 0x817d94e3 },
		{ "video", 0, 0x1000, 0x0c944964 },
		{ "video", 0, 0x1000, 0x958fedf9 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
}};


static aos::RegistryHandler<aos::emulator::GameDriver> pacmanso{ "pacmanso", {
	.name = "Pacman",
	.version = "Pac-Man (SegaSA / Sonic)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x0800, 0x76dbed21 },
		{ "cpu", 0, 0x0800, 0x965bb9b2 },
		{ "cpu", 0, 0x0800, 0x7d177853 },
		{ "cpu", 0, 0x0800, 0xd3e8914c },
		{ "cpu", 0, 0x0800, 0xa5af382c },
		{ "cpu", 0, 0x0800, 0xa948ce83 },
		{ "cpu", 0, 0x0800, 0xcd03135a },
		{ "cpu", 0, 0x0800, 0xfb397ced },
		{ "video", 0, 0x0800, 0x2ee076d2 },
		{ "video", 0, 0x0800, 0x3591b89d },
		{ "video", 0, 0x0800, 0x9e39323a },
		{ "video", 0, 0x0800, 0x1b1d9096 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
}};


static aos::RegistryHandler<aos::emulator::GameDriver> pacmanvg{ "pacmanvg", {
	.name = "Pacman",
	.version = "Pac-Man (Video Game SA)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x0800, 0x76dbed21 },
		{ "cpu", 0, 0x0800, 0x965bb9b2 },
		{ "cpu", 0, 0x0800, 0x7d177853 },
		{ "cpu", 0, 0x0800, 0xd3e8914c },
		{ "cpu", 0, 0x0800, 0xa5af382c },
		{ "cpu", 0, 0x0800, 0xa948ce83 },
		{ "cpu", 0, 0x0800, 0x7c42d9be },
		{ "cpu", 0, 0x0800, 0x68a7300d },
		{ "video", 0, 0x0800, 0x2229ab07 },
		{ "video", 0, 0x0800, 0x3591b89d },
		{ "video", 0, 0x0800, 0x9e39323a },
		{ "video", 0, 0x0800, 0x1b1d9096 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> pacmanf{ "pacmanf", {
	.name = "Pacman",
	.version = "Pac-Man (Midway, speedup)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x1000, 0xc1e6ab10 },
		{ "cpu", 0, 0x1000, 0x720dc3ee },
		{ "cpu", 0, 0x1000, 0xbcdd1beb },
		{ "cpu", 0, 0x1000, 0x817d94e3 },
		{ "video", 0, 0x1000, 0x0c944964 },
		{ "video", 0, 0x1000, 0x958fedf9 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> pacmod{ "pacmod", {
	.name = "Pacman",
	.version = "Pac-Man (Midway, harder)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x1000, 0x3b2ec270 },
		{ "cpu", 0, 0x1000, 0x1a6fb2d4 },
		{ "cpu", 0, 0x1000, 0x18811780 },
		{ "cpu", 0, 0x1000, 0x5c96a733 },
		{ "video", 0, 0x1000, 0x299fb17a },
		{ "video", 0, 0x1000, 0x958fedf9 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> pacmanjpm{ "pacmanjpm", {
	.name = "Pacman",
	.version = "Pac-Man (JPM bootleg)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x800, 0x2c0fa0ab },
		{ "cpu", 0, 0x800, 0xafeca2f1 },
		{ "cpu", 0, 0x800, 0x7d177853 },
		{ "cpu", 0, 0x800, 0xd3e8914c },
		{ "cpu", 0, 0x800, 0x9045a44c },
		{ "cpu", 0, 0x800, 0x93f344c5 },
		{ "cpu", 0, 0x800, 0x258580a2 },
		{ "cpu", 0, 0x800, 0xb4d7ee8c },
		{ "video", 0, 0x800, 0x2066a0b7 },
		{ "video", 0, 0x800, 0x3591b89d },
		{ "video", 0, 0x800, 0x9e39323a },
		{ "video", 0, 0x800, 0x1b1d9096 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> newpuc2{ "newpuc2", {
	.name = "Pacman",
	.version = "Newpuc2 (set 1)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x800, 0x69496a98 },
		{ "cpu", 0, 0x800, 0x158fc01c },
		{ "cpu", 0, 0x800, 0x7d177853 },
		{ "cpu", 0, 0x800, 0x70810ccf },
		{ "cpu", 0, 0x800, 0x81719de8 },
		{ "cpu", 0, 0x800, 0x3f250c58 },
		{ "cpu", 0, 0x800, 0xe6675736 },
		{ "cpu", 0, 0x800, 0x1f81e765 },
		{ "video", 0, 0x800, 0x2066a0b7 },
		{ "video", 0, 0x800, 0x777c70d3 },
		{ "video", 0, 0x800, 0xca8c184c },
		{ "video", 0, 0x800, 0x7dc75a81 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> newpuc2b{ "newpuc2b", {
	.name = "Pacman",
	.version = "Newpuc2 (set 2)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x800, 0x9d027c4a },
		{ "cpu", 0, 0x800, 0x158fc01c },
		{ "cpu", 0, 0x800, 0x7d177853 },
		{ "cpu", 0, 0x800, 0x70810ccf },
		{ "cpu", 0, 0x800, 0xf5e4b2b1 },
		{ "cpu", 0, 0x800, 0x3f250c58 },
		{ "cpu", 0, 0x800, 0xf068e009 },
		{ "cpu", 0, 0x800, 0x1fadcc2f },
		{ "video", 0, 0x800, 0x2066a0b7 },
		{ "video", 0, 0x800, 0x777c70d3 },
		{ "video", 0, 0x800, 0xca8c184c },
		{ "video", 0, 0x800, 0x7dc75a81 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> newpuckx{ "newpuckx", {
	.name = "Pacman",
	.version = "New Puck-X",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x1000, 0xa8ae23c5 },
		{ "cpu", 0, 0x1000, 0x1a6fb2d4 },
		{ "cpu", 0, 0x1000, 0x197443f8 },
		{ "cpu", 0, 0x1000, 0x2e64a3ba },
		{ "video", 0, 0x1000, 0x0c944964 },
		{ "video", 0, 0x1000, 0x958fedf9 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> pacheart{ "pacheart", {
	.name = "Pacman",
	.version = "Pac-Man (Hearts)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x800, 0xd844b679 },
		{ "cpu", 0, 0x800, 0xb9152a38 },
		{ "cpu", 0, 0x800, 0x7d177853 },
		{ "cpu", 0, 0x800, 0x842d6574 },
		{ "cpu", 0, 0x800, 0x9045a44c },
		{ "cpu", 0, 0x800, 0x888f3c3e },
		{ "cpu", 0, 0x800, 0xf5265c10 },
		{ "cpu", 0, 0x800, 0x1a21a381 },
		{ "video", 0, 0x800, 0xc62bbabf },
		{ "video", 0, 0x800, 0x3591b89d },
		{ "video", 0, 0x800, 0xca8c184c },
		{ "video", 0, 0x800, 0x1b1d9096 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> bucaner{ "bucaner", {
	.name = "Pacman",
	.version = "Buccaneer (set 1)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x800, 0x2c0fa0ab },
		{ "cpu", 0, 0x800, 0xafeca2f1 },
		{ "cpu", 0, 0x800, 0x6b53ada9 },
		{ "cpu", 0, 0x800, 0x35f3ca84 },
		{ "cpu", 0, 0x800, 0x9045a44c },
		{ "cpu", 0, 0x800, 0x888f3c3e },
		{ "cpu", 0, 0x800, 0x292de161 },
		{ "cpu", 0, 0x800, 0x884af858 },
		{ "video", 0, 0x800, 0x4060c077 },
		{ "video", 0, 0x800, 0xe3861283 },
		{ "video", 0, 0x800, 0x09f66dec },
		{ "video", 0, 0x800, 0x653314e7 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> bucanera{ "bucanera", {
	.name = "Pacman",
	.version = "Buccaneer (set 2)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x800, 0x2c0fa0ab },
		{ "cpu", 0, 0x800, 0xafeca2f1 },
		{ "cpu", 0, 0x800, 0x6b53ada9 },
		{ "cpu", 0, 0x800, 0x35f3ca84 },
		{ "cpu", 0, 0x800, 0x9045a44c },
		{ "cpu", 0, 0x800, 0x888f3c3e },
		{ "cpu", 0, 0x800, 0x292de161 },
		{ "cpu", 0, 0x800, 0xe037834d },
		{ "video", 0, 0x800, 0xf814796f },
		{ "video", 0, 0x800, 0xe3861283 },
		{ "video", 0, 0x800, 0x09f66dec },
		{ "video", 0, 0x800, 0x653314e7 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> hangly{ "hangly", {
	.name = "Pacman",
	.version = "Hangly-Man (set 1)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x1000, 0x5fe8610a },
		{ "cpu", 0, 0x1000, 0x73726586 },
		{ "cpu", 0, 0x1000, 0x4e7ef99f },
		{ "cpu", 0, 0x1000, 0x7f4147e6 },
		{ "video", 0, 0x1000, 0x0c944964 },
		{ "video", 0, 0x1000, 0x958fedf9 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> hangly2{ "hangly2", {
	.name = "Pacman",
	.version = "Hangly-Man (set 2)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x1000, 0x5fe8610a },
		{ "cpu", 0, 0x800, 0x5ba228bb },
		{ "cpu", 0, 0x800, 0xbaf5461e },
		{ "cpu", 0, 0x1000, 0x4e7ef99f },
		{ "cpu", 0, 0x800, 0x51305374 },
		{ "cpu", 0, 0x800, 0x427c9d4d },
		{ "video", 0, 0x1000, 0x299fb17a },
		{ "video", 0, 0x1000, 0x958fedf9 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> hangly3{ "hangly3", {
	.name = "Pacman",
	.version = "Hangly-Man (set 3)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x800, 0x9d027c4a },
		{ "cpu", 0, 0x800, 0x194c7189 },
		{ "cpu", 0, 0x800, 0x5ba228bb },
		{ "cpu", 0, 0x800, 0xbaf5461e },
		{ "cpu", 0, 0x800, 0x08419c4a },
		{ "cpu", 0, 0x800, 0xab74b51f },
		{ "cpu", 0, 0x800, 0x5039b082 },
		{ "cpu", 0, 0x800, 0x931770d7 },
		{ "video", 0, 0x800, 0x5f4be3cc },
		{ "video", 0, 0x800, 0x3591b89d },
		{ "video", 0, 0x800, 0x9e39323a },
		{ "video", 0, 0x800, 0x1b1d9096 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> baracuda{ "baracuda", {
	.name = "Pacman",
	.version = "Barracuda",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x1000, 0x5fe8610a },
		{ "cpu", 0, 0x1000, 0x61d38c6c },
		{ "cpu", 0, 0x1000, 0x4e7ef99f },
		{ "cpu", 0, 0x1000, 0x55e86c2b },
		{ "video", 0, 0x800, 0x3fc4030c },
		{ "video", 0, 0x800, 0xea7fba5e },
		{ "video", 0, 0x800, 0xf3e9c9d5 },
		{ "video", 0, 0x800, 0x133d720d },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> popeyeman{ "popeyeman", {
	.name = "Pacman",
	.version = "Popeye-Man",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x800, 0x9d027c4a },
		{ "cpu", 0, 0x800, 0x194c7189 },
		{ "cpu", 0, 0x800, 0x5ba228bb },
		{ "cpu", 0, 0x800, 0xbaf5461e },
		{ "cpu", 0, 0x800, 0x08419c4a },
		{ "cpu", 0, 0x800, 0xab74b51f },
		{ "cpu", 0, 0x800, 0x5039b082 },
		{ "cpu", 0, 0x800, 0x931770d7 },
		{ "video", 0, 0x800, 0xb569c4c1 },
		{ "video", 0, 0x800, 0x3591b89d },
		{ "video", 0, 0x800, 0x014fb5a4 },
		{ "video", 0, 0x800, 0x21b91c64 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> crockman{ "crockman", {
	.name = "Pacman",
	.version = "Crock-Man",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x800, 0x2c0fa0ab },
		{ "cpu", 0, 0x800, 0xafeca2f1 },
		{ "cpu", 0, 0x800, 0x7d177853 },
		{ "cpu", 0, 0x800, 0xd3e8914c },
		{ "cpu", 0, 0x800, 0x9045a44c },
		{ "cpu", 0, 0x800, 0x93f344c5 },
		{ "cpu", 0, 0x800, 0xbed4a077 },
		{ "cpu", 0, 0x800, 0x800be41e },
		{ "video", 0, 0x800, 0xa10218c4 },
		{ "video", 0, 0x800, 0x3591b89d },
		{ "video", 0, 0x800, 0x9e39323a },
		{ "video", 0, 0x800, 0x1b1d9096 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> crockmanf{ "crockmanf", {
	.name = "Pacman",
	.version = "Crock-Man (bootleg)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x800, 0x2c0fa0ab },
		{ "cpu", 0, 0x800, 0xafeca2f1 },
		{ "cpu", 0, 0x800, 0x7d177853 },
		{ "cpu", 0, 0x800, 0xd3e8914c },
		{ "cpu", 0, 0x800, 0x9045a44c },
		{ "cpu", 0, 0x800, 0x93f344c5 },
		{ "cpu", 0, 0x800, 0xbed4a077 },
		{ "cpu", 0, 0x800, 0x800be41e },
		{ "video", 0, 0x800, 0x581d0c11 },
		{ "video", 0, 0x800, 0x3591b89d },
		{ "video", 0, 0x800, 0x9e39323a },
		{ "video", 0, 0x800, 0x1b1d9096 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> joyman{ "joyman", {
	.name = "Pacman",
	.version = "Joyman",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x800, 0xd844b679 },
		{ "cpu", 0, 0x800, 0xab9c8f29 },
		{ "cpu", 0, 0x800, 0x7d177853 },
		{ "cpu", 0, 0x800, 0xb3c8d32e },
		{ "cpu", 0, 0x800, 0x9045a44c },
		{ "cpu", 0, 0x800, 0x888f3c3e },
		{ "cpu", 0, 0x800, 0x00b553f8 },
		{ "cpu", 0, 0x800, 0x5d5ce992 },
		{ "video", 0, 0x800, 0x39b557bc },
		{ "video", 0, 0x800, 0x33e0289e },
		{ "video", 0, 0x800, 0x338771a6 },
		{ "video", 0, 0x800, 0xf4f0add5 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> ctrpllrp{ "ctrpllrp", {
	.name = "Pacman",
	.version = "Caterpillar Pacman",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x800, 0x9d027c4a },
		{ "cpu", 0, 0x800, 0xf39846d3 },
		{ "cpu", 0, 0x800, 0xafa149a8 },
		{ "cpu", 0, 0x800, 0xbaf5461e },
		{ "cpu", 0, 0x800, 0x6bb282a1 },
		{ "cpu", 0, 0x800, 0xfa2140f5 },
		{ "cpu", 0, 0x800, 0x86c91e0e },
		{ "cpu", 0, 0x800, 0x3d28134e },
		{ "video", 0, 0x800, 0x1c4617be },
		{ "video", 0, 0x800, 0x46f72fef },
		{ "video", 0, 0x800, 0xba9ec199 },
		{ "video", 0, 0x800, 0x41c09655 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> pacmanfm{ "pacmanfm", {
	.name = "Pacman",
	.version = "Pac Man (FAMARE SA)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x800, 0xf36e88ab },
		{ "cpu", 0, 0x800, 0x618bd9b3 },
		{ "cpu", 0, 0x800, 0x7d177853 },
		{ "cpu", 0, 0x800, 0xd3e8914c },
		{ "cpu", 0, 0x800, 0x6bf4f625 },
		{ "cpu", 0, 0x800, 0xa948ce83 },
		{ "cpu", 0, 0x800, 0xb6289b26 },
		{ "cpu", 0, 0x800, 0x17a88c13 },
		{ "video", 0, 0x800, 0x7a7b48b3 },
		{ "video", 0, 0x800, 0x3591b89d },
		{ "video", 0, 0x800, 0x9e39323a },
		{ "video", 0, 0x800, 0x1b1d9096 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> pacmanug{ "pacmanug", {
	.name = "Pacman",
	.version = "Pac Man (U.Games)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<Pacman>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x800, 0xf36e88ab },
		{ "cpu", 0, 0x800, 0x618bd9b3 },
		{ "cpu", 0, 0x800, 0x7d177853 },
		{ "cpu", 0, 0x800, 0xd3e8914c },
		{ "cpu", 0, 0x800, 0x6bf4f625 },
		{ "cpu", 0, 0x800, 0xa948ce83 },
		{ "cpu", 0, 0x800, 0xb6289b26 },
		{ "cpu", 0, 0x800, 0x17a88c13 },
		{ "video", 0, 0x800, 0xdc9f2a7b },
		{ "video", 0, 0x800, 0x3591b89d },
		{ "video", 0, 0x800, 0x9e39323a },
		{ "video", 0, 0x800, 0x1b1d9096 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


/*
GAME(1980, pacmanpe, puckman, pacman, pacmanpe, pacman_state, empty_init, ROT90, "bootleg (Petaco SA)", "Come Come (Petaco SA bootleg of Puck Man)", MACHINE_SUPPORTS_SAVE) // might have a speed-up button, check
GAME(1980, pacuman, puckman, pacman, pacuman, pacman_state, empty_init, ROT90, "bootleg (Recreativos Franco S.A.)", "Pacu-Man (Spanish bootleg of Puck Man)", MACHINE_SUPPORTS_SAVE) // common bootleg in Spain, code is shifted a bit compared to the Puck Man sets. Title & Manufacturer info from cabinet/PCB, not displayed ingame
GAME(1981, piranha, puckman, piranha, mspacman, pacman_state, init_eyes, ROT90, "GL (US Billiards license)", "Piranha", MACHINE_SUPPORTS_SAVE)
GAME(1981, piranhao, puckman, piranha, mspacman, pacman_state, init_eyes, ROT90, "GL (US Billiards license)", "Piranha (older)", MACHINE_SUPPORTS_SAVE)
GAME(1981, mspacmab3, puckman, piranha, mspacman, pacman_state, init_eyes, ROT90, "bootleg", "Ms. Pac-Man (bootleg, set 3)", MACHINE_SUPPORTS_SAVE)
GAME(1981, abscam, puckman, piranha, mspacman, pacman_state, init_eyes, ROT90, "GL (US Billiards license)", "Abscam", MACHINE_SUPPORTS_SAVE)
GAME(1981, piranhah, puckman, pacman, mspacman, pacman_state, empty_init, ROT90, "hack", "Piranha (hack)", MACHINE_SUPPORTS_SAVE)
GAME(1981, titanpac, puckman, piranha, mspacman, pacman_state, init_eyes, ROT90, "hack (NSM)", "Titan (Pac-Man hack)", MACHINE_SUPPORTS_SAVE)
*/