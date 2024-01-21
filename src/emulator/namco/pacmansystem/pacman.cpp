#include "registry.h"
#include "../pacmansystem.h"
#include "../gpu/pacmangpu.h"

#include "rom_library.h"
#include "configuration.h"


using aos::namco::PacmanSystem;
using aos::namco::PacmanGpu;
using aos::emulator::GameDriver;


//
// Pacman System - Pacman
//
// Configuration:
//   * ROM 0x0000 - 0x4000
//   * Ports 0 & 1
//   * DipSwitch 0
//
class Pacman : public PacmanSystem<PacmanGpu>
{
	public:
		Pacman(const aos::mmu::RomMappings& roms,
			const aos::emulator::GameConfiguration& game) :
			PacmanSystem(roms, game,
				PacmanGpu::create({
					.orientation = geometry_t::rotation_t::ROT90,
					.tileModel = PacmanGpu::Configuration::TileModel::PACMAN,
					.spriteAddress = 0xff0
				}))
		{
			_port2.set(0, "dsw1-0");
			_port2.set(2, "dsw1-2");
			_port2.set(3, "dsw1-3");
			_port2.set(4, "dsw1-4");
			_port2.set(5, "dsw1-5");
			_port2.set(6, "dsw1-6");
			_port2.set(7, "dsw1-7");
			_port2.init(game);
		}

		void mapping() override
		{
			if (_romDecoder)
				_mmu.map(0, 0x3fff).mirror(0x7fff).name("cpu").rom().decodefn([this](uint8_t* m, const size_t s) { _romDecoder("cpu", m, s); });
			else
				_mmu.map(0, 0x3fff).mirror(0x7fff).name("cpu").rom();
			_mmu.map(0x4000, 0x4fff).mirror(0x7fff).readfn([this](const uint16_t a) { return _gpu->readVRAM(a); }).writefn([this](const uint16_t a, const uint8_t v) { _gpu->writeVRAM(a, v); });
			_mmu.map(0x5000, 0x503f).mirror(0x7fff).readfn([this](const uint16_t) { return _port0.get(); });
			_mmu.map(0x5040, 0x507f).mirror(0x7fff).readfn([this](const uint16_t) { return _port1.get(); });
			_mmu.map(0x5080, 0x50bf).mirror(0x7fff).readfn([this](const uint16_t) { return _port2.get(); });
			_mmu.map(0x5000, 0x5000).mirror(0x7fff).writefn([this](const uint16_t, const uint8_t value) { this->enableInterrupts(((value & 1) == 1) ? true : false); });
			_mmu.map(0x5003, 0x5003).mirror(0x7fff).writefn([this](const uint16_t, const uint8_t value) { _gpu->flip(((value & 1) == 1) ? true : false); });
			_mmu.map(0x5040, 0X505f).mirror(0x7fff).writefn([this](const uint16_t address, const uint8_t value) { _wsg.write(address & 0xff, value); });
			_mmu.map(0x5060, 0x506f).mirror(0x7fff).writefn([this](const uint16_t address, const uint8_t value) { _gpu->writeSpritePos(address, value); });
		}
};

static std::unique_ptr<Pacman> pacman_init(const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
	// Port 0 - JOYSTICK 0x0f CHEAT 0x10 COIN 0x20
	auto emul = std::make_unique<Pacman>(roms, config);
	emul->port0().joystick1().coin();
	emul->port0().set(4, "_CHEAT", true);
	emul->port1().starts();
	return emul;
}
static std::unique_ptr<Pacman> pacman_init_coininverted(const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
	// Port 0 - JOYSTICK 0x0f CHEAT 0x10 COIN 0x40
	auto emul = std::make_unique<Pacman>(roms, config);
	emul->port0().joystick1().coin(6);
	emul->port0().set(4, "_CHEAT", true);
	emul->port1().starts();
	return emul;
}
static std::unique_ptr<Pacman> jumpshot_init(const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
	// Port 0 - JOYSTICK 0x0f COIN 0x20
	// Port 1 - JOYSTICK 0x0f FIRE 0X60
	auto emul = std::make_unique<Pacman>(roms, config);
	emul->port0().joystick1().coin();
	emul->port1().joystick2().fire1(5).fire2(6);
	emul->romDecodingFn(jumpshot_decodeRom);
	return emul;
}
static std::unique_ptr<Pacman> piranha_init(const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
	// pacman + decoding
	auto emul = pacman_init(config, roms);
	emul->romDecodingFn(eyes_decodeRom);
	emul->interruptDecodingFn([](const uint8_t v) { return (v == 0xfa) ? 0x78 : v; });
	return emul;
}

static std::unique_ptr<Pacman> eyes_init(const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
	// Port 0 - JOYSTICK 0x0f COIN 0x20
	// Port 1 - FIRE 0x10
	auto emul = std::make_unique<Pacman>(roms, config);
	emul->port0().joystick1().coin();
	emul->port1().starts().fire1();
	emul->romDecodingFn(eyes_decodeRom);
	return emul;
}
static std::unique_ptr<Pacman> eyes_nodecode_init(const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
	// Port 0 - JOYSTICK 0x0f COIN 0x20
	// Port 1 - FIRE 0x10
	auto emul = std::make_unique<Pacman>(roms, config);
	emul->port0().joystick1().coin();
	emul->port1().starts().fire1();
	return emul;
}

static aos::RegistryHandler<GameDriver> puckman{ "puckman", {
	.name = "Pacman",
	.version = "Puck Man (Japan set 1)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_800_f36e88ab },
		{ "cpu", rom_cpu_800_618bd9b3 },
		{ "cpu", rom_cpu_800_7d177853 },
		{ "cpu", rom_cpu_800_d3e8914c },
		{ "cpu", rom_cpu_800_6bf4f625 },
		{ "cpu", rom_cpu_800_a948ce83 },
		{ "cpu", rom_cpu_800_b6289b26 },
		{ "cpu", rom_cpu_800_17a88c13 },
		{ "video", rom_video_800_2066a0b7 },
		{ "video", rom_video_800_3591b89d },
		{ "video", rom_video_800_9e39323a },
		{ "video", rom_video_800_1b1d9096 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
}};
static aos::RegistryHandler<GameDriver> puckmanb{ "puckmanb", {
	.name = "Pacman",
	.version = "Puck Man (bootleg set 1)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_1k_fee263b3 },
		{ "cpu", rom_cpu_1k_39d1fc83 },
		{ "cpu", rom_cpu_1k_02083b03 },
		{ "cpu", rom_cpu_1k_7a36fe55 },
		{ "video", rom_video_1k_0c944964 },
		{ "video", rom_video_1k_958fedf9 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
}};
static aos::RegistryHandler<GameDriver> puckmanf{ "puckmanf", {
	.name = "Pacman",
	.version = "Puck Man (speedup hack)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_1k_fee263b3 },
		{ "cpu", rom_cpu_1k_51b38db9 },
		{ "cpu", rom_cpu_1k_02083b03 },
		{ "cpu", rom_cpu_1k_7a36fe55 },
		{ "video", rom_video_1k_0c944964 },
		{ "video", rom_video_1k_958fedf9 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
}};
static aos::RegistryHandler<GameDriver> puckmanh{ "puckmanh", {
	.name = "Pacman",
	.version = "Puck Man (bootleg set 2)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_1k_5fe8610a },
		{ "cpu", rom_cpu_1k_61d38c6c },
		{ "cpu", rom_cpu_1k_4e7ef99f },
		{ "cpu", rom_cpu_1k_8939ddd2 },
		{ "video", rom_video_800_2229ab07 },
		{ "video", rom_video_800_3591b89d },
		{ "video", rom_video_800_9e39323a },
		{ "video", rom_video_800_1b1d9096 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
}};
static aos::RegistryHandler<GameDriver> puckmod{ "puckmod", {
	.name = "Pacman",
	.version = "Puck Man (Japan set 2)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_1k_fee263b3 },
		{ "cpu", rom_cpu_1k_39d1fc83 },
		{ "cpu", rom_cpu_1k_02083b03 },
		{ "cpu", rom_cpu_1k_7d98d5f5 },
		{ "video", rom_video_1k_0c944964 },
		{ "video", rom_video_1k_958fedf9 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
}};
static aos::RegistryHandler<GameDriver> pacman{ "pacman", {
	.name = "Pacman",
	.version = "Pac-Man (Midway)",
	.main_version = true,
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_1k_c1e6ab10 },
		{ "cpu", rom_cpu_1k_1a6fb2d4 },
		{ "cpu", rom_cpu_1k_bcdd1beb },
		{ "cpu", rom_cpu_1k_817d94e3 },
		{ "video", rom_video_1k_0c944964 },
		{ "video", rom_video_1k_958fedf9 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
}};
static aos::RegistryHandler<GameDriver> pacmanso{ "pacmanso", {
	.name = "Pacman",
	.version = "Pac-Man (SegaSA / Sonic)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_800_76dbed21 },
		{ "cpu", rom_cpu_800_965bb9b2 },
		{ "cpu", rom_cpu_800_7d177853 },
		{ "cpu", rom_cpu_800_d3e8914c },
		{ "cpu", rom_cpu_800_a5af382c },
		{ "cpu", rom_cpu_800_a948ce83 },
		{ "cpu", rom_cpu_800_cd03135a },
		{ "cpu", rom_cpu_800_fb397ced },
		{ "video", rom_video_800_2ee076d2 },
		{ "video", rom_video_800_3591b89d },
		{ "video", rom_video_800_9e39323a },
		{ "video", rom_video_800_1b1d9096 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
}};
static aos::RegistryHandler<GameDriver> pacmanvg{ "pacmanvg", {
	.name = "Pacman",
	.version = "Pac-Man (Video Game SA)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_800_76dbed21 },
		{ "cpu", rom_cpu_800_965bb9b2 },
		{ "cpu", rom_cpu_800_7d177853 },
		{ "cpu", rom_cpu_800_d3e8914c },
		{ "cpu", rom_cpu_800_a5af382c },
		{ "cpu", rom_cpu_800_a948ce83 },
		{ "cpu", rom_cpu_800_7c42d9be },
		{ "cpu", rom_cpu_800_68a7300d },
		{ "video", rom_video_800_2229ab07 },
		{ "video", rom_video_800_3591b89d },
		{ "video", rom_video_800_9e39323a },
		{ "video", rom_video_800_1b1d9096 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> pacmanf{ "pacmanf", {
	.name = "Pacman",
	.version = "Pac-Man (Midway, speedup)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_1k_c1e6ab10 },
		{ "cpu", rom_cpu_1k_720dc3ee },
		{ "cpu", rom_cpu_1k_bcdd1beb },
		{ "cpu", rom_cpu_1k_817d94e3 },
		{ "video", rom_video_1k_0c944964 },
		{ "video", rom_video_1k_958fedf9 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> pacmod{ "pacmod", {
	.name = "Pacman",
	.version = "Pac-Man (Midway, harder)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_1k_3b2ec270 },
		{ "cpu", rom_cpu_1k_1a6fb2d4 },
		{ "cpu", rom_cpu_1k_18811780 },
		{ "cpu", rom_cpu_1k_5c96a733 },
		{ "video", rom_video_1k_299fb17a },
		{ "video", rom_video_1k_958fedf9 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> pacmanjpm{ "pacmanjpm", {
	.name = "Pacman",
	.version = "Pac-Man (JPM bootleg)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_800_2c0fa0ab },
		{ "cpu", rom_cpu_800_afeca2f1 },
		{ "cpu", rom_cpu_800_7d177853 },
		{ "cpu", rom_cpu_800_d3e8914c },
		{ "cpu", rom_cpu_800_9045a44c },
		{ "cpu", rom_cpu_800_93f344c5 },
		{ "cpu", rom_cpu_800_258580a2 },
		{ "cpu", rom_cpu_800_b4d7ee8c },
		{ "video", rom_video_800_2066a0b7 },
		{ "video", rom_video_800_3591b89d },
		{ "video", rom_video_800_9e39323a },
		{ "video", rom_video_800_1b1d9096 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> newpuc2{ "newpuc2", {
	.name = "Pacman",
	.version = "Newpuc2 (set 1)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_800_69496a98 },
		{ "cpu", rom_cpu_800_150fc01c },
		{ "cpu", rom_cpu_800_7d177853 },
		{ "cpu", rom_cpu_800_70810ccf },
		{ "cpu", rom_cpu_800_81719de8 },
		{ "cpu", rom_cpu_800_3f250c58 },
		{ "cpu", rom_cpu_800_e6675736 },
		{ "cpu", rom_cpu_800_1f81e765 },
		{ "video", rom_video_800_2066a0b7 },
		{ "video", rom_video_800_777c70d3 },
		{ "video", rom_video_800_ca8c184c },
		{ "video", rom_video_800_7dc75a81 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> newpuc2b{ "newpuc2b", {
	.name = "Pacman",
	.version = "Newpuc2 (set 2)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_800_9d027c4a },
		{ "cpu", rom_cpu_800_150fc01c },
		{ "cpu", rom_cpu_800_7d177853 },
		{ "cpu", rom_cpu_800_70810ccf },
		{ "cpu", rom_cpu_800_f5e4b2b1 },
		{ "cpu", rom_cpu_800_3f250c58 },
		{ "cpu", rom_cpu_800_f068e009 },
		{ "cpu", rom_cpu_800_1fadcc2f },
		{ "video", rom_video_800_2066a0b7 },
		{ "video", rom_video_800_777c70d3 },
		{ "video", rom_video_800_ca8c184c },
		{ "video", rom_video_800_7dc75a81 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> newpuckx{ "newpuckx", {
	.name = "Pacman",
	.version = "New Puck-X",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_1k_a8ae23c5 },
		{ "cpu", rom_cpu_1k_1a6fb2d4 },
		{ "cpu", rom_cpu_1k_197443f8 },
		{ "cpu", rom_cpu_1k_2e64a3ba },
		{ "video", rom_video_1k_0c944964 },
		{ "video", rom_video_1k_958fedf9 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> pacheart{ "pacheart", {
	.name = "Pacman",
	.version = "Pac-Man (Hearts)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_800_d844b679 },
		{ "cpu", rom_cpu_800_b9152a38 },
		{ "cpu", rom_cpu_800_7d177853 },
		{ "cpu", rom_cpu_800_842d6574 },
		{ "cpu", rom_cpu_800_9045a44c },
		{ "cpu", rom_cpu_800_888f3c3e },
		{ "cpu", rom_cpu_800_f5265c10 },
		{ "cpu", rom_cpu_800_1a21a381 },
		{ "video", rom_video_800_c62bbabf },
		{ "video", rom_video_800_3591b89d },
		{ "video", rom_video_800_ca8c184c },
		{ "video", rom_video_800_1b1d9096 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> bucaner{ "bucaner", {
	.name = "Pacman",
	.version = "Buccaneer (set 1)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_800_2c0fa0ab },
		{ "cpu", rom_cpu_800_afeca2f1 },
		{ "cpu", rom_cpu_800_6b53ada9 },
		{ "cpu", rom_cpu_800_35f3ca84 },
		{ "cpu", rom_cpu_800_9045a44c },
		{ "cpu", rom_cpu_800_888f3c3e },
		{ "cpu", rom_cpu_800_292de161 },
		{ "cpu", rom_cpu_800_884af858 },
		{ "video", rom_video_800_4060c077 },
		{ "video", rom_video_800_e3861283 },
		{ "video", rom_video_800_09f66dec },
		{ "video", rom_video_800_653314e7 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> bucanera{ "bucanera", {
	.name = "Pacman",
	.version = "Buccaneer (set 2)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_800_2c0fa0ab },
		{ "cpu", rom_cpu_800_afeca2f1 },
		{ "cpu", rom_cpu_800_6b53ada9 },
		{ "cpu", rom_cpu_800_35f3ca84 },
		{ "cpu", rom_cpu_800_9045a44c },
		{ "cpu", rom_cpu_800_888f3c3e },
		{ "cpu", rom_cpu_800_292de161 },
		{ "cpu", rom_cpu_800_e037834d },
		{ "video", rom_video_800_f814796f },
		{ "video", rom_video_800_e3861283 },
		{ "video", rom_video_800_09f66dec },
		{ "video", rom_video_800_653314e7 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> hangly{ "hangly", {
	.name = "Pacman",
	.version = "Hangly-Man (set 1)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_1k_5fe8610a },
		{ "cpu", rom_cpu_1k_73726586 },
		{ "cpu", rom_cpu_1k_4e7ef99f },
		{ "cpu", rom_cpu_1k_7f4147e6 },
		{ "video", rom_video_1k_0c944964 },
		{ "video", rom_video_1k_958fedf9 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> hangly2{ "hangly2", {
	.name = "Pacman",
	.version = "Hangly-Man (set 2)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_1k_5fe8610a },
		{ "cpu", rom_cpu_800_5ba228bb },
		{ "cpu", rom_cpu_800_baf5461e },
		{ "cpu", rom_cpu_1k_4e7ef99f },
		{ "cpu", rom_cpu_800_51305374 },
		{ "cpu", rom_cpu_800_427c9d4d },
		{ "video", rom_video_1k_299fb17a },
		{ "video", rom_video_1k_958fedf9 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> hangly3{ "hangly3", {
	.name = "Pacman",
	.version = "Hangly-Man (set 3)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_800_9d027c4a },
		{ "cpu", rom_cpu_800_194c7189 },
		{ "cpu", rom_cpu_800_5ba228bb },
		{ "cpu", rom_cpu_800_baf5461e },
		{ "cpu", rom_cpu_800_08419c4a },
		{ "cpu", rom_cpu_800_ab74b51f },
		{ "cpu", rom_cpu_800_5039b082 },
		{ "cpu", rom_cpu_800_931770d7 },
		{ "video", rom_video_800_5f4be3cc },
		{ "video", rom_video_800_3591b89d },
		{ "video", rom_video_800_9e39323a },
		{ "video", rom_video_800_1b1d9096 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> baracuda{ "baracuda", {
	.name = "Pacman",
	.version = "Barracuda",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_1k_5fe8610a },
		{ "cpu", rom_cpu_1k_61d38c6c },
		{ "cpu", rom_cpu_1k_4e7ef99f },
		{ "cpu", rom_cpu_1k_55e86c2b },
		{ "video", rom_video_800_3fc4030c },
		{ "video", rom_video_800_ea7fba5e },
		{ "video", rom_video_800_f3e9c9d5 },
		{ "video", rom_video_800_133d720d },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> popeyeman{ "popeyeman", {
	.name = "Pacman",
	.version = "Popeye-Man",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_800_9d027c4a },
		{ "cpu", rom_cpu_800_194c7189 },
		{ "cpu", rom_cpu_800_5ba228bb },
		{ "cpu", rom_cpu_800_baf5461e },
		{ "cpu", rom_cpu_800_08419c4a },
		{ "cpu", rom_cpu_800_ab74b51f },
		{ "cpu", rom_cpu_800_5039b082 },
		{ "cpu", rom_cpu_800_931770d7 },
		{ "video", rom_video_800_b569c4c1 },
		{ "video", rom_video_800_3591b89d },
		{ "video", rom_video_800_014fb5a4 },
		{ "video", rom_video_800_21b91c64 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> crockman{ "crockman", {
	.name = "Pacman",
	.version = "Crock-Man",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_800_2c0fa0ab },
		{ "cpu", rom_cpu_800_afeca2f1 },
		{ "cpu", rom_cpu_800_7d177853 },
		{ "cpu", rom_cpu_800_d3e8914c },
		{ "cpu", rom_cpu_800_9045a44c },
		{ "cpu", rom_cpu_800_93f344c5 },
		{ "cpu", rom_cpu_800_bed4a077 },
		{ "cpu", rom_cpu_800_800be41e },
		{ "video", rom_video_800_a10218c4 },
		{ "video", rom_video_800_3591b89d },
		{ "video", rom_video_800_9e39323a },
		{ "video", rom_video_800_1b1d9096 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> crockmanf{ "crockmanf", {
	.name = "Pacman",
	.version = "Crock-Man (bootleg)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_800_2c0fa0ab },
		{ "cpu", rom_cpu_800_afeca2f1 },
		{ "cpu", rom_cpu_800_7d177853 },
		{ "cpu", rom_cpu_800_d3e8914c },
		{ "cpu", rom_cpu_800_9045a44c },
		{ "cpu", rom_cpu_800_93f344c5 },
		{ "cpu", rom_cpu_800_bed4a077 },
		{ "cpu", rom_cpu_800_800be41e },
		{ "video", rom_video_800_581d0c11 },
		{ "video", rom_video_800_3591b89d },
		{ "video", rom_video_800_9e39323a },
		{ "video", rom_video_800_1b1d9096 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> joyman{ "joyman", {
	.name = "Pacman",
	.version = "Joyman",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_800_d844b679 },
		{ "cpu", rom_cpu_800_ab9c8f29 },
		{ "cpu", rom_cpu_800_7d177853 },
		{ "cpu", rom_cpu_800_b3c8d32e },
		{ "cpu", rom_cpu_800_9045a44c },
		{ "cpu", rom_cpu_800_888f3c3e },
		{ "cpu", rom_cpu_800_00b553f8 },
		{ "cpu", rom_cpu_800_5d5ce992 },
		{ "video", rom_video_800_39b557bc },
		{ "video", rom_video_800_33e0289e },
		{ "video", rom_video_800_338771a6 },
		{ "video", rom_video_800_f4f0add5 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> ctrpllrp{ "ctrpllrp", {
	.name = "Pacman",
	.version = "Caterpillar Pacman",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_800_9d027c4a },
		{ "cpu", rom_cpu_800_f39846d3 },
		{ "cpu", rom_cpu_800_afa149a8 },
		{ "cpu", rom_cpu_800_baf5461e },
		{ "cpu", rom_cpu_800_6bb282a1 },
		{ "cpu", rom_cpu_800_fa2140f5 },
		{ "cpu", rom_cpu_800_86c91e0e },
		{ "cpu", rom_cpu_800_3d28134e },
		{ "video", rom_video_800_1c4617be },
		{ "video", rom_video_800_46f72fef },
		{ "video", rom_video_800_ba9ec199 },
		{ "video", rom_video_800_41c09655 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> pacmanfm{ "pacmanfm", {
	.name = "Pacman",
	.version = "Pac Man (FAMARE SA)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_800_f36e88ab },
		{ "cpu", rom_cpu_800_618bd9b3 },
		{ "cpu", rom_cpu_800_7d177853 },
		{ "cpu", rom_cpu_800_d3e8914c },
		{ "cpu", rom_cpu_800_6bf4f625 },
		{ "cpu", rom_cpu_800_a948ce83 },
		{ "cpu", rom_cpu_800_b6289b26 },
		{ "cpu", rom_cpu_800_17a88c13 },
		{ "video", rom_video_800_7a7b48b3 },
		{ "video", rom_video_800_3591b89d },
		{ "video", rom_video_800_9e39323a },
		{ "video", rom_video_800_1b1d9096 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> pacmanug{ "pacmanug", {
	.name = "Pacman",
	.version = "Pac Man (U.Games)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_800_f36e88ab },
		{ "cpu", rom_cpu_800_618bd9b3 },
		{ "cpu", rom_cpu_800_7d177853 },
		{ "cpu", rom_cpu_800_d3e8914c },
		{ "cpu", rom_cpu_800_6bf4f625 },
		{ "cpu", rom_cpu_800_a948ce83 },
		{ "cpu", rom_cpu_800_b6289b26 },
		{ "cpu", rom_cpu_800_17a88c13 },
		{ "video", rom_video_800_dc9f2a7b },
		{ "video", rom_video_800_3591b89d },
		{ "video", rom_video_800_9e39323a },
		{ "video", rom_video_800_1b1d9096 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };
static aos::RegistryHandler<GameDriver> pacmanpe{ "pacmanpe", {
	.name = "Pacman",
	.version = "Come Come",
	.emulator = "namco",
	.creator = pacman_init_coininverted,
	.roms = {
		{ "cpu", rom_cpu_800_183d235a },
		{ "cpu", rom_cpu_800_2771c530 },
		{ "cpu", rom_cpu_800_7d177853 },
		{ "cpu", rom_cpu_800_d3e8914c },
		{ "cpu", rom_cpu_800_cc92abb1 },
		{ "cpu", rom_cpu_800_8810b38e },
		{ "cpu", rom_cpu_800_a02ce07f },
		{ "cpu", rom_cpu_800_34a9566d },
		{ "video", rom_video_800_2229ab07 },
		{ "video", rom_video_800_3591b89d },
		{ "video", rom_video_800_9e39323a },
		{ "video", rom_video_800_1b1d9096 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacmanpe_configuration
} };
static aos::RegistryHandler<GameDriver> pacuman{ "pacuman", {
	.name = "Pacman",
	.version = "Pacu-Man",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_800_ec8c1ed8 },
		{ "cpu", rom_cpu_800_40edaf56 },
		{ "cpu", rom_cpu_800_db4f702b },
		{ "cpu", rom_cpu_800_28f7257d },
		{ "cpu", rom_cpu_800_212acb41 },
		{ "cpu", rom_cpu_800_e73dd1b9 },
		{ "cpu", rom_cpu_800_908a2cb2 },
		{ "cpu", rom_cpu_800_cf4ba26c },
		{ "video", rom_video_800_2066a0b7 },
		{ "video", rom_video_800_3591b89d },
		{ "video", rom_video_800_9e39323a },
		{ "video", rom_video_800_1b1d9096 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacuman_configuration
} };
static aos::RegistryHandler<GameDriver> piranhah{ "piranhah", {
	.name = "Pacman",
	.version = "Piranha (hack)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_1k_bc5ad024 },
		{ "cpu", rom_cpu_1k_1a6fb2d4 },
		{ "cpu", rom_cpu_1k_473c379d },
		{ "cpu", rom_cpu_1k_63fbf895 },
		{ "video", rom_video_800_3fc4030c },
		{ "video", rom_video_800_30b9a010 },
		{ "video", rom_video_800_f3e9c9d5 },
		{ "video", rom_video_800_133d720d },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mspacman_configuration
} };
static aos::RegistryHandler<GameDriver> pacmansp{ "pacmansp", {
	.name = "Pacman",
	.version = "Puck Man (spanish)",
	.emulator = "namco",
	.creator = pacman_init,
	.roms = {
		{ "cpu", rom_cpu_8k_f2404b4d, {.size = 0x4000 } },
		{ "video", rom_video_2k_7a75b696, {.size = 0x0800 } },
		{ "video", rom_video_2k_7a75b696, {.start = 0x1000, .size = 0x0800 } },
		{ "video", rom_video_2k_7a75b696, {.start = 0x800, .size = 0x0800 } },
		{ "video", rom_video_2k_7a75b696, {.start = 0x1800, .size = 0x0800 } },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacmansp_configuration
} };
static aos::RegistryHandler<GameDriver> piranha{ "piranha", {
	.name = "Pacman",
	.version = "Piranha",
	.emulator = "namco",
	.creator = piranha_init,
	.roms = {
		{ "cpu", rom_cpu_800_69a3e6ea },
		{ "cpu", rom_cpu_800_245e753f },
		{ "cpu", rom_cpu_800_62cb6954 },
		{ "cpu", rom_cpu_800_cb0700bc },
		{ "cpu", rom_cpu_800_843fbfe5 },
		{ "cpu", rom_cpu_800_73084d5e },
		{ "cpu", rom_cpu_800_4cdf6704 },
		{ "cpu", rom_cpu_800_b86fedb3 },
		{ "video", rom_video_800_0f19eb28 },
		{ "video", rom_video_800_5f8bdabe },
		{ "video", rom_video_800_d19399fb },
		{ "video", rom_video_800_cfb4403d },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_08c9447b },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mspacman_configuration
} };
static aos::RegistryHandler<GameDriver> piranhao{ "piranhao", {
	.name = "Pacman",
	.version = "Piranha (older)",
	.emulator = "namco",
	.creator = piranha_init,
	.roms = {
		{ "cpu", rom_cpu_800_c6ce1bfc },
		{ "cpu", rom_cpu_800_a2655a33 },
		{ "cpu", rom_cpu_800_62cb6954 },
		{ "cpu", rom_cpu_800_cb0700bc },
		{ "cpu", rom_cpu_800_843fbfe5 },
		{ "cpu", rom_cpu_800_73084d5e },
		{ "cpu", rom_cpu_800_9363a4d1 },
		{ "cpu", rom_cpu_800_2769979c },
		{ "video", rom_video_800_94eb7563 },
		{ "video", rom_video_800_a3606973 },
		{ "video", rom_video_800_84165a2c },
		{ "video", rom_video_800_2699ba9e },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_08c9447b },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mspacman_configuration
} };
static aos::RegistryHandler<GameDriver> abscam{ "abscam", {
	.name = "Pacman",
	.version = "Abscam",
	.emulator = "namco",
	.creator = piranha_init,
	.roms = {
		{ "cpu", rom_cpu_800_0b102302 },
		{ "cpu", rom_cpu_800_3116a8ec },
		{ "cpu", rom_cpu_800_bc0281e0 },
		{ "cpu", rom_cpu_800_428ee2e8 },
		{ "cpu", rom_cpu_800_e05d46ad },
		{ "cpu", rom_cpu_800_3ae9a8cb },
		{ "cpu", rom_cpu_800_b39eb940 },
		{ "cpu", rom_cpu_800_16cf1c67 },
		{ "video", rom_video_800_61daabe5 },
		{ "video", rom_video_800_81d50c98 },
		{ "video", rom_video_800_a3bd1613 },
		{ "video", rom_video_800_9d802b68 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_1605b324 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mspacman_configuration
} };
static aos::RegistryHandler<GameDriver> titanpac{ "titanpac", {
	.name = "Pacman",
	.version = "Titan",
	.emulator = "namco",
	.creator = piranha_init,
	.roms = {
		{ "cpu", rom_cpu_800_5538c288 },
		{ "cpu", rom_cpu_800_095f5a5f },
		{ "cpu", rom_cpu_800_8117a6a2 },
		{ "cpu", rom_cpu_800_cb0700bc },
		{ "cpu", rom_cpu_800_060e514e },
		{ "cpu", rom_cpu_800_9209882a },
		{ "cpu", rom_cpu_800_2c8c7299 },
		{ "cpu", rom_cpu_800_ff943d70 },
		{ "video", rom_video_800_412e723e },
		{ "video", rom_video_800_87d28931 },
		{ "video", rom_video_800_3be1601b },
		{ "video", rom_video_800_f773cb8b },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_b67a0c10 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mspacman_configuration
} };
static aos::RegistryHandler<GameDriver> pacplus{ "pacplus", {
	.name = "Pacman Plus",
	.version = "Pac-Man Plus",
	.main_version = true,
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
		auto emul = pacman_init(config, roms);
		emul->romDecodingFn(pacplus_decodeRom);
		return emul;
	},
	.roms = {
		{ "cpu", rom_cpu_1k_d611ef68 },
		{ "cpu", rom_cpu_1k_c7207556 },
		{ "cpu", rom_cpu_1k_ae379430 },
		{ "cpu", rom_cpu_1k_5a6dff7b },
		{ "video", rom_video_1k_022c35da },
		{ "video", rom_video_1k_4de65cdd },
		{ "palette", rom_palette_20_063dd53a },
		{ "palette", rom_palette_100_e271a166 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = pacman_configuration
} };

static aos::RegistryHandler<GameDriver> jumpshot{ "jumpshot", {
	.name = "Jump Shot",
	.version = "Jump Shot (Bally Midway)",
	.main_version = true,
	.emulator = "namco",
	.creator = jumpshot_init,
	.roms = {
		{ "cpu", rom_cpu_1k_f00def9a },
		{ "cpu", rom_cpu_1k_f70deae2 },
		{ "cpu", rom_cpu_1k_894d6f68 },
		{ "cpu", rom_cpu_1k_f15a108a },
		{ "video", rom_video_1k_d9fa90f5 },
		{ "video", rom_video_1k_2ec711c1 },
		{ "palette", rom_palette_20_872b42f3 },
		{ "palette", rom_palette_100_0399f39f },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = jumpshot_configuration
} };
static aos::RegistryHandler<GameDriver> jumpshotp{ "jumpshotp", {
	.name = "Jump Shot",
	.version = "Jump Shot (Engineering sample)",
	.emulator = "namco",
	.creator = jumpshot_init,
	.roms = {
		{ "cpu", rom_cpu_1k_acc5e15e },
		{ "cpu", rom_cpu_1k_62b48ba4 },
		{ "cpu", rom_cpu_1k_7c9b5e30 },
		{ "cpu", rom_cpu_1k_9f0c39f6 },
		{ "video", rom_video_1k_d9fa90f5 },
		{ "video", rom_video_1k_2ec711c1 },
		{ "palette", rom_palette_20_872b42f3 },
		{ "palette", rom_palette_100_0399f39f },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = jumpshotp_configuration
} };

static aos::RegistryHandler<GameDriver> eyes{ "eyes", {
	.name = "Eyes",
	.version = "Eyes (US set 1)",
	.main_version = true,
	.emulator = "namco",
	.creator = eyes_init,
	.roms = {
		{ "cpu", rom_cpu_1k_3b09ac89 },
		{ "cpu", rom_cpu_1k_97096855 },
		{ "cpu", rom_cpu_1k_731e294e },
		{ "cpu", rom_cpu_1k_22f7a719 },
		{ "video", rom_video_1k_d6af0030 },
		{ "video", rom_video_1k_a42b5201 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_d8d78829 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = eyes_configuration
} };
static aos::RegistryHandler<GameDriver> eyes2{ "eyes2", {
	.name = "Eyes",
	.version = "Eyes (US set 2)",
	.emulator = "namco",
	.creator = eyes_init,
	.roms = {
		{ "cpu", rom_cpu_1k_2cda7185 },
		{ "cpu", rom_cpu_1k_b9fe4f59 },
		{ "cpu", rom_cpu_1k_d618ba66 },
		{ "cpu", rom_cpu_1k_cf038276 },
		{ "video", rom_video_1k_03b1b4c7 },
		{ "video", rom_video_1k_a42b5201 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_d8d78829 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = eyes_configuration
} };
static aos::RegistryHandler<GameDriver> eyesb{ "eyesb", {
	.name = "Eyes",
	.version = "Eyes (bootleg set 1)",
	.emulator = "namco",
	.creator = eyes_init,
	.roms = {
		{ "cpu", rom_cpu_800_339d279a },
		{ "cpu", rom_cpu_800_1b68a61d },
		{ "cpu", rom_cpu_800_d4f9aaf8 },
		{ "cpu", rom_cpu_800_6b41bb80 },
		{ "cpu", rom_cpu_800_748e0e48 },
		{ "cpu", rom_cpu_800_7b7f4a74 },
		{ "cpu", rom_cpu_800_367a3884 },
		{ "cpu", rom_cpu_800_2baaadae },
		{ "video", rom_video_800_342c0653 },
		{ "video", rom_video_800_99af4b30 },
		{ "video", rom_video_800_b247b82c },
		{ "video", rom_video_800_aaa7a537 },
		{ "palette", rom_palette_20_2c3cc909 },
		{ "palette", rom_palette_100_d8d78829 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = eyes_configuration
} };
static aos::RegistryHandler<GameDriver> eyeszac{ "eyeszac", {
	.name = "Eyes",
	.version = "Eyes (Italy)",
	.emulator = "namco",
	.creator = eyes_init,
	.roms = {
		{ "cpu", rom_cpu_1k_e555b265 },
		{ "cpu", rom_cpu_1k_d6d73eb5 },
		{ "cpu", rom_cpu_1k_604c940c },
		{ "cpu", rom_cpu_1k_acc9cd8b },
		{ "video", rom_video_1k_d6af0030 },
		{ "video", rom_video_1k_a42b5201 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_d8d78829 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = eyes_configuration
} };
static aos::RegistryHandler<GameDriver> eyeszacb{ "eyeszacb", {
	.name = "Eyes",
	.version = "Eyes (bootleg set 2)",
	.emulator = "namco",
	.creator = eyes_nodecode_init,
	.roms = {
		{ "cpu", rom_cpu_800_a4a9d7a0 },
		{ "cpu", rom_cpu_800_c32b3f73 },
		{ "cpu", rom_cpu_800_195b9473 },
		{ "cpu", rom_cpu_800_292886cb },
		{ "cpu", rom_cpu_800_ff94b015 },
		{ "cpu", rom_cpu_800_9271c58c },
		{ "cpu", rom_cpu_800_965cf32b },
		{ "cpu", rom_cpu_800_c254e92e },
		{ "video", rom_video_800_59dce22e },
		{ "video", rom_video_800_aaa7a537 },
		{ "video", rom_video_800_1969792b },
		{ "video", rom_video_800_99af4b30 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_d8d78829 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = eyes_configuration
} };

// Doesn't work
//static aos::RegistryHandler<GameDriver> shootbul{ "shootbul", {
//	.name = "Shoot the Bull",
//	.version = "Midway",
//	.main_version = true,
//	.emulator = "namco",
//	.creator = [](const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) { return std::make_unique<Pacman>(roms, config, jumpshot_decodeRom); },
//	.roms = {
//		{ "cpu", rom_cpu_1k_25daa5e9 },
//		{ "cpu", rom_cpu_1k_92144044 },
//		{ "cpu", rom_cpu_1k_43b7f99d },
//		{ "cpu", rom_cpu_1k_bc4d3bbf },
//		{ "video", rom_video_1k_07c6c5aa },
//		{ "video", rom_video_1k_eaec6837 },
//		{ "palette", rom_palette_20_ec578b98 },
//		{ "palette", rom_palette_100_81a6b30f },
//		{ "sound", rom_sound_100_a9cc86bf }
//	},
//	.configuration = pacman_configuration
//} };

static aos::RegistryHandler<GameDriver> mrtnt{ "mrtnt", {
	.name = "Mr. TNT",
	.version = "Techstar",
	.main_version = true,
	.emulator = "namco",
	.creator = eyes_init,
	.roms = {
		{ "cpu", rom_cpu_1k_0e836586 },
		{ "cpu", rom_cpu_1k_779c4c5b },
		{ "cpu", rom_cpu_1k_ad6fc688 },
		{ "cpu", rom_cpu_1k_d77557b3 },
		{ "video", rom_video_1k_3038cc0e },
		{ "video", rom_video_1k_97634d8b },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mrtnt_configuration
} };
static aos::RegistryHandler<GameDriver> gorkans{ "gorkans", {
	.name = "Mr. TNT",
	.version = "Gorkans (Techstar)",
	.emulator = "namco",
	.creator = eyes_nodecode_init,
	.roms = {
		{ "cpu", rom_cpu_800_55100b18 },
		{ "cpu", rom_cpu_800_b5c604bf },
		{ "cpu", rom_cpu_800_b8c6def4 },
		{ "cpu", rom_cpu_800_4602c840 },
		{ "cpu", rom_cpu_800_21412a62 },
		{ "cpu", rom_cpu_800_a013310b },
		{ "cpu", rom_cpu_800_122969b2 },
		{ "cpu", rom_cpu_800_f2524b11 },
		{ "video", rom_video_800_39cd0dbc },
		{ "video", rom_video_800_33d52535 },
		{ "video", rom_video_800_4b6b7970 },
		{ "video", rom_video_800_ed70bb3c },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mrtnt_configuration
} };

static aos::RegistryHandler<GameDriver> eggor{ "eggor", {
	.name = "Eggor",
	.version = "Tecko",
	.emulator = "namco",
	.creator = eyes_init,
	.roms = {
		{ "cpu", rom_cpu_800_818ed154 },
		{ "cpu", rom_cpu_800_a4b21d93 },
		{ "cpu", rom_cpu_800_5d7a23ed },
		{ "cpu", rom_cpu_800_e9dbca8d },
		{ "cpu", rom_cpu_800_4318ab85 },
		{ "cpu", rom_cpu_800_03214d7f },
		{ "cpu", rom_cpu_800_dc805be4 },
		{ "cpu", rom_cpu_800_f9ae204b },
		{ "video", rom_video_800_96ad8626 },
		{ "video", rom_video_800_cc324017 },
		{ "video", rom_video_800_7c97f513 },
		{ "video", rom_video_800_2e930602 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mrtnt_configuration
} };


//static aos::RegistryHandler<GameDriver> sprglbpg{ "sprglbpg", {
//	.name = "Super Glob",
//	.version = "Super Glob (German bootleg)",
//	.emulator = "namco",
//	.creator = [](const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
//		auto emul = std::make_unique<Pacman>(roms, config);
//		emul->port1().set(4, "_JOY1_FIRE", true);
//		return emul;
//	},
//	.roms = {
//		{ "cpu", rom_cpu_1k_a2df2073 },
//		{ "cpu", rom_cpu_1k_3d2c22d9 },
//		{ "cpu", rom_cpu_1k_a252047f },
//		{ "cpu", rom_cpu_1k_7efa81f1 },
//		{ "video", rom_video_2k_e54f484d },
//		{ "palette", rom_palette_20_1f617527 },
//		{ "palette", rom_palette_100_28faa769 },
//		{ "sound", rom_sound_100_c29dea27 }
//	},
//	.configuration = mrtnt_configuration
//} };

/*

GAME( 1985, porky,    0,        porky,    porky,    pacman_state,  init_porky,    ROT90,  "Shinkai Inc. (Magic Electronics Inc. license)", "Porky", MACHINE_SUPPORTS_SAVE )


GAME(1992, mschamp, mspacman, mschamp, mschamp, pacman_state, init_mschamp, ROT90, "hack", "Ms. Pacman Champion Edition / Zola-Puc Gal", MACHINE_SUPPORTS_SAVE) // Rayglo version
GAME(1995, mschamps, mspacman, mschamp, mschamp, pacman_state, init_mschamp, ROT90, "hack", "Ms. Pacman Champion Edition / Super Zola-Puc Gal", MACHINE_SUPPORTS_SAVE)

GAME( 1984, sprglbpg, suprglob, pacman,   theglobp, epospm_state,  empty_init,    ROT90,  "bootleg (Software Labor)", "Super Glob (Pac-Man hardware) (German bootleg)",             MACHINE_SUPPORTS_SAVE )


//          rom       parent    machine   inp       state          init


GAME( 1992, mspactwin,    0,        mspactwin, mspactwin, mspactwin_state, init_mspactwin, ROT90,  "hack (Susilu)",   "Ms Pac Man Twin (Argentina)",            MACHINE_SUPPORTS_SAVE )

GAME( 1989, clubpacm,     0,        clubpacm,  clubpacm,  clubpacm_state,  empty_init,     ROT90,  "hack (Miky SRL)", "Pacman Club / Club Lambada (Argentina)", MACHINE_SUPPORTS_SAVE )
GAME( 1990, clubpacma,    clubpacm, clubpacm,  clubpacma, clubpacm_state,  init_clubpacma, ROT90,  "hack (Miky SRL)", "Pacman Club (Argentina)",                MACHINE_SUPPORTS_SAVE )

GAME( 1981, crush,    0,        korosuke, maketrax, pacman_state,  init_maketrax, ROT90,  "Alpha Denshi Co. / Kural Samno Electric, Ltd.", "Crush Roller (set 1)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1981, crush2,   crush,    crush2,   maketrax, pacman_state,  empty_init,    ROT90,  "Alpha Denshi Co. / Kural Esco Electric, Ltd.",  "Crush Roller (set 2)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1981, crush3,   crush,    korosuke, maketrax, pacman_state,  init_maketrax, ROT90,  "Alpha Denshi Co. / Kural Electric, Ltd.",       "Crush Roller (set 3)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1981, crush4,   crush,    crush2,   maketrax, pacman_state,  init_eyes,     ROT90,  "Alpha Denshi Co. / Kural Electric, Ltd.",       "Crush Roller (set 4)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1981, crush5,   crush,    crush4,   crush4,   pacman_state,  empty_init,    ROT90,  "Alpha Denshi Co. / Kural TWT",                  "Crush Roller (set 5)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1981, maketrax, crush,    korosuke, maketrax, pacman_state,  init_maketrax, ROT270, "Alpha Denshi Co. / Kural (Williams license)",   "Make Trax (US set 1)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1981, maketrxb, crush,    korosuke, maketrax, pacman_state,  init_maketrax, ROT270, "Alpha Denshi Co. / Kural (Williams license)",   "Make Trax (US set 2)",                   MACHINE_SUPPORTS_SAVE )
GAME( 1981, korosuke, crush,    korosuke, korosuke, pacman_state,  init_maketrax, ROT90,  "Alpha Denshi Co. / Kural Electric, Ltd.",       "Korosuke Roller (Japan)",                MACHINE_SUPPORTS_SAVE ) // ADK considers it a sequel?
GAME( 1981, crushrlf, crush,    crush2,   maketrax, pacman_state,  empty_init,    ROT90,  "bootleg",                                       "Crush Roller (Famare SA PCB)",           MACHINE_SUPPORTS_SAVE )
GAME( 1981, crushbl,  crush,    crush2,   maketrax, pacman_state,  empty_init,    ROT90,  "bootleg",                                       "Crush Roller (bootleg set 1)",           MACHINE_SUPPORTS_SAVE )
GAME( 1981, crushbl2, crush,    korosuke, mbrush,   pacman_state,  init_mbrush,   ROT90,  "bootleg",                                       "Crush Roller (bootleg set 2)",           MACHINE_SUPPORTS_SAVE )
GAME( 1981, crushbl3, crush,    korosuke, mbrush,   pacman_state,  init_maketrax, ROT90,  "bootleg",                                       "Crush Roller (bootleg set 3)",           MACHINE_SUPPORTS_SAVE )
GAME( 1981, crushs,   crush,    crushs,   crushs,   pacman_state,  empty_init,    ROT90,  "bootleg (Sidam)",                               "Crush Roller (bootleg set 4)",           MACHINE_SUPPORTS_SAVE ) // Sidam PCB, no Sidam text
GAME( 1981, mbrush,   crush,    korosuke, mbrush,   pacman_state,  init_mbrush,   ROT90,  "bootleg (Olympia)",                             "Magic Brush (bootleg of Crush Roller)",  MACHINE_SUPPORTS_SAVE )
GAME( 1981, paintrlr, crush,    crush2,   paintrlr, pacman_state,  empty_init,    ROT90,  "bootleg",                                       "Paint Roller (bootleg of Crush Roller)", MACHINE_SUPPORTS_SAVE )
GAME( 1984, painter,  crush,    crush2,   paintrlr, pacman_state,  empty_init,    ROT90,  "hack (Monshine Ent. Co.)",                      "Painter (hack of Crush Roller)",         MACHINE_IMPERFECT_GRAPHICS | MACHINE_SUPPORTS_SAVE ) // currently shows Paintei due to bad sprite ROM


GAME( 1983, birdiy,   0,        birdiy,   birdiy,   pacman_state,  empty_init,    ROT270, "Mama Top", "Birdiy", MACHINE_NO_COCKTAIL | MACHINE_SUPPORTS_SAVE )


GAME( 1981, nmouse,   0,        nmouse,   nmouse,   pacman_state,  init_eyes,     ROT90,  "Amenip (Palcom Queen River)", "Naughty Mouse (set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1981, nmouseb,  nmouse,   nmouse,   nmouse,   pacman_state,  init_eyes,     ROT90,  "Amenip Nova Games Ltd.",      "Naughty Mouse (set 2)", MACHINE_SUPPORTS_SAVE )


GAME( 1982, dremshpr, 0,        dremshpr, dremshpr, pacman_state,  empty_init,    ROT270, "Sanritsu", "Dream Shopper", MACHINE_SUPPORTS_SAVE )

GAME( 1983, vanvan,   0,        vanvan,   vanvan,   pacman_state,  empty_init,    ROT270, "Sanritsu",                     "Van-Van Car",                  MACHINE_SUPPORTS_SAVE )
GAME( 1983, vanvank,  vanvan,   vanvan,   vanvank,  pacman_state,  empty_init,    ROT270, "Sanritsu (Karateco license?)", "Van-Van Car (Karateco set 1)", MACHINE_SUPPORTS_SAVE ) // or bootleg?
GAME( 1983, vanvanb,  vanvan,   vanvan,   vanvank,  pacman_state,  empty_init,    ROT270, "Sanritsu (Karateco license?)", "Van-Van Car (Karateco set 2)", MACHINE_SUPPORTS_SAVE ) // "

GAME( 1983, bwcasino, 0,        acitya,   bwcasino, epospm_state,  empty_init,    ROT90,  "Epos Corporation", "Boardwalk Casino",     MACHINE_SUPPORTS_SAVE )
GAME( 1983, acitya,   bwcasino, acitya,   acitya,   epospm_state,  empty_init,    ROT90,  "Epos Corporation", "Atlantic City Action", MACHINE_SUPPORTS_SAVE )

GAME( 1983, theglobp, suprglob, theglobp, theglobp, epospm_state,  empty_init,    ROT90,  "Epos Corporation",         "The Glob (Pac-Man hardware)",                                MACHINE_SUPPORTS_SAVE )
GAME( 1983, sprglobp, suprglob, theglobp, theglobp, epospm_state,  empty_init,    ROT90,  "Epos Corporation",         "Super Glob (Pac-Man hardware)",                              MACHINE_SUPPORTS_SAVE )
GAME( 1984, beastfp,  suprglob, theglobp, theglobp, epospm_state,  empty_init,    ROT90,  "Epos Corporation",         "Beastie Feastie (Pac-Man conversion)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1984, eeekkp,   eeekk,    eeekkp,   eeekkp,   epospm_state,  empty_init,    ROT90,  "Epos Corporation",         "Eeekk! (Pac-Man conversion)",                                MACHINE_SUPPORTS_SAVE )

GAME( 1984, drivfrcp, 0,        drivfrcp, drivfrcp, pacman_state,  init_drivfrcp, ROT90,  "Shinkai Inc. (Magic Electronics Inc. license)", "Driving Force (Pac-Man conversion)", MACHINE_SUPPORTS_SAVE )

GAME( 1985, 8bpm,     8ballact, _8bpm,    8bpm,     pacman_state,  init_8bpm,     ROT90,  "Seatongrove UK, Ltd. (Magic Electronics USA license)", "Eight Ball Action (Pac-Man conversion)", MACHINE_SUPPORTS_SAVE )


GAME( 1986, rocktrv2, 0,        rocktrv2, rocktrv2, pacman_state,  init_rocktrv2, ROT90,  "Triumph Software Inc.", "MTV Rock-N-Roll Trivia (Part 2)", MACHINE_SUPPORTS_SAVE )

GAME( 1986, bigbucks, 0,        bigbucks, bigbucks, pacman_state,  empty_init,    ROT90,  "Dynasoft Inc.", "Big Bucks", MACHINE_SUPPORTS_SAVE )

GAME( 1983, numcrash, 0,        numcrash, numcrash, pacman_state,  empty_init,    ROT90,  "Hanshin Goraku / Peni", "Number Crash", MACHINE_SUPPORTS_SAVE ) // "Peni soft" related?

GAME( 1985, cannonbp, 0,        cannonbp, cannonbp, pacman_state,  empty_init,    ROT90,  "Novomatic", "Cannon Ball (Pac-Man Hardware)", MACHINE_WRONG_COLORS | MACHINE_SUPPORTS_SAVE )

GAME( 1999, superabc, 0,        superabc, superabc, pacman_state,  init_superabc, ROT90,  "hack (Two-Bit Score)", "Super ABC (Pac-Man multigame kit, Sep. 03 1999)", MACHINE_SUPPORTS_SAVE )
GAME( 1999, superabco,superabc, superabc, superabc, pacman_state,  init_superabc, ROT90,  "hack (Two-Bit Score)", "Super ABC (Pac-Man multigame kit, Mar. 08 1999)", MACHINE_SUPPORTS_SAVE )

GAME( 1981, pengojpm, pengo,    pengojpm, pengojpm, pacman_state,  empty_init,    ROT90,  "bootleg",               "Pengo (bootleg on Pac-Man hardware, set 1)",             MACHINE_SUPPORTS_SAVE ) // conversion of pacmanjpm board with wire mods
GAME( 1981, pengopac, pengo,    pengojpm, pengojpm, pacman_state,  empty_init,    ROT90,  "bootleg",               "Pengo (bootleg on Pac-Man hardware, set 2)",             MACHINE_SUPPORTS_SAVE ) // different conversion?
GAME( 1982, pengomc1, pengo,    pengojpm, pengojpm, pacman_state,  init_pengomc1, ROT90,  "bootleg (Marti Colls)", "Pengo (Marti Colls bootleg on Pac-Man hardware, set 1)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, pengomc2, pengo,    pengojpm, pengojpm, pacman_state,  empty_init,    ROT90,  "bootleg (Marti Colls)", "Pengo (Marti Colls bootleg on Pac-Man hardware, set 2)", MACHINE_SUPPORTS_SAVE )
GAME( 1982, pinguinos,pengo,    pengojpm, pengojpm, pacman_state,  empty_init,    ROT90,  "bootleg (Aincar)",      "Pinguinos (Spanish bootleg on Pac-Man hardware)",        MACHINE_SUPPORTS_SAVE )*/