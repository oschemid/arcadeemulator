#include "../pacmansystem.h"
#include "../gpu/pacmangpu.h"
#include "registry.h"

#include "rom_library.h"
#include "configuration.h"

using aos::namco::PacmanSystem;
using aos::namco::PacmanGpu;
using aos::emulator::GameDriver;


//
// Pacman System - Woodpecker
//
// Configuration:
//   * ROM 0x0000 - 0x4000
//   * ROM 0x8000 - 0xC000
//   * Ports 0 & 1
//   * DipSwitch 0
//
class Woodpecker : public PacmanSystem<PacmanGpu>
{
public:
	Woodpecker(const vector<aos::mmu::RomMapping>& roms,
		const aos::emulator::GameConfiguration& game) :
		PacmanSystem(roms, game,
			PacmanGpu::create({
			.orientation = geometry_t::rotation_t::ROT90,
			.tileModel = PacmanGpu::Configuration::TileModel::PACMAN,
			.spriteAddress = 0xff0 }
			))
	{
		_port2.set(0, "dsw1-0");
		_port2.set(2, "dsw1-2");
		_port2.set(4, "dsw1-4");
		_port2.set(6, "dsw1-6");
		_port2.set(7);
		_port2.init(game);

		_port3 = 0xff;
		_port3.set(0, "dsw2-0");
		_port3.init(game);
	}

protected:
	void mapping() override
	{
		if (_romDecoder) {
			_mmu.map(0, 0x3fff).name("cpu").rom().decodefn([this](uint8_t* m, const size_t s) { _romDecoder("cpu", m, s); });
			_mmu.map(0x8000, 0xbfff).name("cpu2").rom().decodefn([this](uint8_t* m, const size_t s) { _romDecoder("cpu2", m, s); });
		}
		else {
			_mmu.map(0, 0x3fff).name("cpu").rom();
			_mmu.map(0x8000, 0xbfff).name("cpu2").rom();
		}
		_mmu.map(0x4000, 0x4fff).readfn([this](const uint16_t a) { return _gpu->readVRAM(a); }).writefn([this](const uint16_t a, const uint8_t v) { _gpu->writeVRAM(a, v); });
		_mmu.map(0x5000, 0x503f).readfn([this](const uint16_t) { return _port0.get(); });
		_mmu.map(0x5040, 0x507f).readfn([this](const uint16_t a) { uint8_t value = _port1.get(); return (_portDecoder) ? _portDecoder(a & 0xff, value) : value; });
		_mmu.map(0x5080, 0x50bf).readfn([this](const uint16_t) { return _port2.get(); });
		_mmu.map(0x50c0, 0x50ff).readfn([this](const uint16_t) { return _port3.get(); });
		_mmu.map(0x5000, 0x5000).writefn([this](const uint16_t, const uint8_t value) { this->enableInterrupts(((value & 1) == 1) ? true : false); });
		_mmu.map(0x5003, 0x5003).writefn([this](const uint16_t, const uint8_t value) { static_cast<PacmanGpu*>(&(*_gpu))->flip(((value & 1) == 1) ? true : false); });
		_mmu.map(0x5040, 0X505f).writefn([this](const uint16_t address, const uint8_t value) { _wsg.write(address & 0xff, value); });
		_mmu.map(0x5060, 0x506f).writefn([this](const uint16_t address, const uint8_t value) { static_cast<PacmanGpu*>(&(*_gpu))->writeSpritePos(address, value); });
	}
};
	
static std::unique_ptr<Woodpecker> mspacman_init(const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
	// Port 0 - JOYSTICK 0x0f CHEAT 0x10 COIN 0x20
	auto emul = std::make_unique<Woodpecker>(roms, config);
	emul->port0().joystick1().coin();
	emul->port0().set(4, "_CHEAT", true);
	emul->port1().starts();
	return emul;
}
static std::unique_ptr<Woodpecker> woodpecker_init(const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
	// Port 0 - JOYSTICK 0x0f CHEAT 0x10 COIN 0x20
	auto emul = mspacman_init(config, roms);
	emul->romDecodingFn(woodpecker_decodeRom);
	return emul;
}
static std::unique_ptr<Woodpecker> lizwiz_init(const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
	// Port 0 - JOYSTICK 0x0f COIN 0x20
	auto emul = std::make_unique<Woodpecker>(roms, config);
	emul->port0().joystick1().coin();
	emul->port1().joystick2().fire1().starts().fire2(7);
	return emul;
}
static std::unique_ptr<Woodpecker> ponpoko_init(const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
	// Port 0 - JOYSTICK 0x0f COIN 0x20
	auto emul = std::make_unique<Woodpecker>(roms, config);
	emul->port0().reset(6);
	emul->port0().reset(7);
	emul->port0().joystick1(false).fire1(4, false).coin(5, false);
	emul->port1().joystick1(false).fire1(4, false).starts(false);
	emul->port1().reset(7);
	emul->romDecodingFn(ponpoko_decodeRom);
	emul->_port3 = 0xf1;
	emul->rotateDisplay();
	return emul;
}

static aos::RegistryHandler<aos::emulator::GameDriver> woodpeck{ "woodpeck", {
	.name = "Woodpecker",
	.version = "Amenip (set 1)",
	.main_version = true,
	.emulator = "namco",
	.creator = woodpecker_init,
	.roms = {
		{ "cpu", rom_cpu_1k_37ea66ca },
		{ "cpu2", rom_cpu_1k_cd115dba },
		{ "cpu2", rom_cpu_1k_d40b2321 },
		{ "cpu2", rom_cpu_1k_024092f4 },
		{ "cpu2", rom_cpu_1k_18ef0fc8 },
		{ "video", rom_video_800_15a87f62 },
		{ "video", rom_video_800_ab4abd88 },
		{ "video", rom_video_800_53368498 },
		{ "video", rom_video_800_d7b80a45 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_d8772167 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = woodpecker_configuration
} };
static aos::RegistryHandler<aos::emulator::GameDriver> woodpecka{ "woodpecka", {
	.name = "Woodpecker",
	.version = "Amenip (set 2)",
	.emulator = "namco",
	.creator = woodpecker_init,
	.roms = {
		{ "cpu", rom_cpu_1k_b5ee8bca },
		{ "cpu2", rom_cpu_1k_c5ec2de6 },
		{ "cpu2", rom_cpu_1k_07ea534e },
		{ "cpu2", rom_cpu_1k_a3a3253a },
		{ "cpu2", rom_cpu_1k_6c50546b },
		{ "video", rom_video_1k_0bf52102 },
		{ "video", rom_video_1k_0ed8def8 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_d8772167 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = woodpecker_configuration
} };

static aos::RegistryHandler<aos::emulator::GameDriver> mspacmab{ "mspacmab", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-man (bootleg, set 1)",
	.emulator = "namco",
	.creator = mspacman_init,
	.roms = {
		{ "cpu", rom_cpu_1k_d16b31b7 },
		{ "cpu", rom_cpu_1k_0d32de5e },
		{ "cpu", rom_cpu_1k_1821ee0b },
		{ "cpu", rom_cpu_1k_165a9dd8 },
		{ "cpu2", rom_cpu_1k_8c3e6de6 },
		{ "cpu2", rom_cpu_1k_368cb165 },
		{ "video", rom_video_1k_5c281d01 },
		{ "video", rom_video_1k_615af909 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mspacman_configuration
} };
static aos::RegistryHandler<aos::emulator::GameDriver> mspacmab2{ "mspacmab2", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-man (bootleg, set 2)",
	.emulator = "namco",
	.creator = mspacman_init,
	.roms = {
		{ "cpu", rom_cpu_2k_39ae7b16 },
		{ "cpu", rom_cpu_2k_09d86ef8 },
		{ "cpu2", rom_cpu_2k_9921d46f },
		{ "video", rom_video_1k_5c281d01 },
		{ "video", rom_video_1k_615af909 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = woodpecker_configuration
} };
static aos::RegistryHandler<aos::emulator::GameDriver> mspacmab4{ "mspacmab4", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-man (bootleg, set 4)",
	.emulator = "namco",
	.creator = mspacman_init,
	.roms = {
		{ "cpu", rom_cpu_2k_3ed9d3ca },
		{ "cpu", rom_cpu_2k_988db4af },
		{ "cpu2", rom_cpu_2k_9921d46f },
		{ "video", rom_video_1k_5c281d01 },
		{ "video", rom_video_1k_615af909 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = woodpecker_configuration
} };
static aos::RegistryHandler<aos::emulator::GameDriver> mspacmbmc{ "mspacmbmc", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-man (Marti Colls)",
	.emulator = "namco",
	.creator = mspacman_init,
	.roms = {
		{ "cpu", rom_cpu_1k_d16b31b7 },
		{ "cpu", rom_cpu_1k_0d32de5e },
		{ "cpu", rom_cpu_1k_1821ee0b },
		{ "cpu", rom_cpu_1k_e086219d },
		{ "cpu2", rom_cpu_1k_8c3e6de6 },
		{ "cpu2", rom_cpu_1k_206a9623 },
		{ "video", rom_video_1k_5c281d01 },
		{ "video", rom_video_1k_615af909 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = woodpecker_configuration
} };
static aos::RegistryHandler<aos::emulator::GameDriver> mspacmanlai{ "mspacmanlai", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-man (Leisure and Allied)",
	.emulator = "namco",
	.creator = mspacman_init,
	.roms = {
		{ "cpu", rom_cpu_1k_d16b31b7 },
		{ "cpu", rom_cpu_1k_0d32de5e },
		{ "cpu", rom_cpu_1k_1821ee0b },
		{ "cpu", rom_cpu_1k_e30f2dae },
		{ "cpu2", rom_cpu_1k_8c3e6de6 },
		{ "cpu2", rom_cpu_800_286041cf },
		{ "video", rom_video_800_93933d1d },
		{ "video", rom_video_800_ec7caeba },
		{ "video", rom_video_800_22b0188a },
		{ "video", rom_video_800_50c7477d },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = woodpecker_configuration
} };
static aos::RegistryHandler<aos::emulator::GameDriver> pacgal{ "pacgal", {
	.name = "Ms. Pac-man",
	.version = "Pac-Gal (set 1)",
	.emulator = "namco",
	.creator = mspacman_init,
	.roms = {
		{ "cpu", rom_cpu_1k_d16b31b7 },
		{ "cpu", rom_cpu_1k_0d32de5e },
		{ "cpu", rom_cpu_1k_513f4d5c },
		{ "cpu", rom_cpu_1k_70694c8e },
		{ "cpu2", rom_cpu_1k_8c3e6de6 },
		{ "cpu2", rom_cpu_1k_368cb165 },
		{ "video", rom_video_1k_5c281d01 },
		{ "video", rom_video_800_65a3ee71 },
		{ "video", rom_video_800_50c7477d },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_63efb927 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = woodpecker_configuration
} };
static aos::RegistryHandler<aos::emulator::GameDriver> mspacpls{ "mspacpls", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-man Plus",
	.emulator = "namco",
	.creator = mspacman_init,
	.roms = {
		{ "cpu", rom_cpu_1k_d16b31b7 },
		{ "cpu", rom_cpu_1k_0af09d31 },
		{ "cpu", rom_cpu_1k_1821ee0b },
		{ "cpu", rom_cpu_1k_165a9dd8 },
		{ "cpu2", rom_cpu_1k_e6e06954 },
		{ "cpu2", rom_cpu_1k_3b5db308 },
		{ "video", rom_video_1k_5c281d01 },
		{ "video", rom_video_1k_615af909 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = woodpecker_configuration
} };
static aos::RegistryHandler<aos::emulator::GameDriver> mspacmanbg{ "mspacmanbg", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-man (Greece set 1)",
	.emulator = "namco",
	.creator = mspacman_init,
	.roms = {
		{ "cpu", rom_cpu_8k_97c64918, {.size = 0x4000} },
		{ "cpu2", rom_cpu_8k_97c64918, {.start = 0x4000, .size = 0x4000} },
		{ "video", rom_video_8k_f2c5da43, {.size = 0x0800} },
		{ "video", rom_video_8k_f2c5da43, {.start = 0x1000, .size = 0x0800} },
		{ "video", rom_video_8k_f2c5da43, {.start = 0x0800, .size = 0x0800} },
		{ "video", rom_video_8k_f2c5da43, {.start = 0x1800, .size = 0x0800} },
		{ "palette", rom_palette_20_3545e7e9 },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = woodpecker_configuration
} };
static aos::RegistryHandler<aos::emulator::GameDriver> mspacmanbg2{ "mspacmanbg2", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-man (Greece set 2)",
	.emulator = "namco",
	.creator = mspacman_init,
	.roms = {
		{ "cpu", rom_cpu_8k_e11d4132, {.size = 0x4000} },
		{ "cpu2", rom_cpu_8k_e11d4132, {.start = 0x4000, .size = 0x4000} },
		{ "video", rom_video_4k_8ee4a3b0, {.size = 0x0800} },
		{ "video", rom_video_4k_8ee4a3b0, {.start = 0x1000, .size = 0x0800} },
		{ "video", rom_video_4k_8ee4a3b0, {.start = 0x0800, .size = 0x0800} },
		{ "video", rom_video_4k_8ee4a3b0, {.start = 0x1800, .size = 0x0800} },
		{ "palette", rom_palette_20_3545e7e9 },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = woodpecker_configuration
} };
static aos::RegistryHandler<aos::emulator::GameDriver> mspacmanbgd{ "mspacmanbgd", {
	.name = "Ms. Pac-man",
	.version = "Miss Pukman (Greece Datamat)",
	.emulator = "namco",
	.creator = mspacman_init,
	.roms = {
		{ "cpu", rom_cpu_8k_5bcc195e, {.size = 0x4000} },
		{ "cpu2", rom_cpu_8k_5bcc195e, {.start = 0x4000, .size = 0x4000} },
		{ "video", rom_video_8k_f2c5da43, {.size = 0x0800} },
		{ "video", rom_video_8k_f2c5da43, {.start = 0x1000, .size = 0x0800} },
		{ "video", rom_video_8k_f2c5da43, {.start = 0x0800, .size = 0x0800} },
		{ "video", rom_video_8k_f2c5da43, {.start = 0x1800, .size = 0x0800} },
		{ "palette", rom_palette_20_3545e7e9 },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = woodpecker_configuration
} };
static aos::RegistryHandler<aos::emulator::GameDriver> mspacmanblt{ "mspacmanblt", {
	.name = "Ms. Pac-man",
	.version = "Come-Cocos (Greece Triunvi set 1)",
	.emulator = "namco",
	.creator = mspacman_init,
	.roms = {
		{ "cpu", rom_cpu_8k_d9da2917, {.size = 0x4000} },
		{ "cpu2", rom_cpu_8k_d9da2917, {.start = 0x4000, .size = 0x4000} },
		{ "video", rom_video_2k_e6446f49, {.size = 0x0800} },
		{ "video", rom_video_2k_e6446f49, {.start = 0x1000, .size = 0x0800} },
		{ "video", rom_video_2k_e6446f49, {.start = 0x0800, .size = 0x0800} },
		{ "video", rom_video_2k_e6446f49, {.start = 0x1800, .size = 0x0800} },
		{ "palette", rom_palette_20_3545e7e9 },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = woodpecker_configuration
} };
static aos::RegistryHandler<aos::emulator::GameDriver> mspacmanblt2{ "mspacmanblt2", {
	.name = "Ms. Pac-man",
	.version = "Come-Cocos (Greece Triunvi set 2)",
	.emulator = "namco",
	.creator = mspacman_init,
	.roms = {
		{ "cpu", rom_cpu_8k_763c2abb, {.size = 0x4000} },
		{ "cpu2", rom_cpu_8k_763c2abb, {.start = 0x4000, .size = 0x4000} },
		{ "video", rom_video_8k_f2c5da43, {.size = 0x0800} },
		{ "video", rom_video_8k_f2c5da43, {.start = 0x1000, .size = 0x0800} },
		{ "video", rom_video_8k_f2c5da43, {.start = 0x0800, .size = 0x0800} },
		{ "video", rom_video_8k_f2c5da43, {.start = 0x1800, .size = 0x0800} },
		{ "palette", rom_palette_20_3545e7e9 },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = woodpecker_configuration
} };
static aos::RegistryHandler<GameDriver> mspacmanbcc{ "mspacmanbcc", {
	.name = "Ms. Pac-man",
	.version = "Come-Cocos (Greece Tecnausa)",
	.emulator = "namco",
	.creator = mspacman_init,
	.roms = {
		{ "cpu", rom_cpu_8k_220eccae, {.size = 0x4000} },
		{ "cpu2", rom_cpu_8k_220eccae, {.start = 0x4000, .size = 0x4000} },
		{ "video", rom_video_8k_f2c5da43, {.size = 0x0800} },
		{ "video", rom_video_8k_f2c5da43, {.start = 0x1000, .size = 0x0800} },
		{ "video", rom_video_8k_f2c5da43, {.start = 0x0800, .size = 0x0800} },
		{ "video", rom_video_8k_f2c5da43, {.start = 0x1800, .size = 0x0800} },
		{ "palette", rom_palette_20_3545e7e9 },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = woodpecker_configuration
} };
static aos::RegistryHandler<GameDriver> mspacmanbhe{ "mspacmanbhe", {
	.name = "Ms. Pac-man",
	.version = "Come-Cocos (Greece Herle SA)",
	.emulator = "namco",
	.creator =mspacman_init,
	.roms = {
		{ "cpu", rom_cpu_8k_b256540f, {.size = 0x4000} },
		{ "cpu2", rom_cpu_8k_b256540f, {.start = 0x4000, .size = 0x4000} },
		{ "video", rom_video_4k_8ee4a3b0, {.size = 0x0800} },
		{ "video", rom_video_4k_8ee4a3b0, {.start = 0x1000, .size = 0x0800} },
		{ "video", rom_video_4k_8ee4a3b0, {.start = 0x0800, .size = 0x0800} },
		{ "video", rom_video_4k_8ee4a3b0, {.start = 0x1800, .size = 0x0800} },
		{ "palette", rom_palette_20_3545e7e9 },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = woodpecker_configuration
} };
static aos::RegistryHandler<GameDriver> mspacmanbco{ "mspacmanbco", {
	.name = "Ms. Pac-man",
	.version = "Come-Cocos (Cocamatic)",
	.emulator = "namco",
	.creator = mspacman_init,
	.roms = {
		{ "cpu", rom_cpu_8k_1ba81f43, {.size = 0x4000} },
		{ "cpu2", rom_cpu_8k_1ba81f43, {.start = 0x4000, .size = 0x4000} },
		{ "video", rom_video_8k_f2c5da43, {.size = 0x0800} },
		{ "video", rom_video_8k_f2c5da43, {.start = 0x1000, .size = 0x0800} },
		{ "video", rom_video_8k_f2c5da43, {.start = 0x0800, .size = 0x0800} },
		{ "video", rom_video_8k_f2c5da43, {.start = 0x1800, .size = 0x0800} },
		{ "palette", rom_palette_20_3545e7e9 },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = woodpecker_configuration
} };
static aos::RegistryHandler<GameDriver> mspacmanbi{ "mspacmanbi", {
	.name = "Ms. Pac-man",
	.version = "Come-Cocos (Impeuropex)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
		auto emul = mspacman_init(config, roms);
		emul->romDecodingFn([](const string c, uint8_t* m, const size_t s) {
			if (c == "cpu") {
				m[0x18b5] = 0x4e;
				m[0x197d] = 0xda;
				m[0x1a61] = 0x21;
			}
			if (c == "cpu2") {
				m[0x2c0d] = 0xeb;
			}});
		return emul;
	},
	.roms = {
		{ "cpu", rom_cpu_8k_03905a76, {.size = 0x4000} },
		{ "cpu2", rom_cpu_8k_03905a76, {.start = 0x4000, .size = 0x4000} },
		{ "video", rom_video_4k_8ee4a3b0, {.size = 0x0800} },
		{ "video", rom_video_4k_8ee4a3b0, {.start = 0x1000, .size = 0x0800} },
		{ "video", rom_video_4k_8ee4a3b0, {.start = 0x0800, .size = 0x0800} },
		{ "video", rom_video_4k_8ee4a3b0, {.start = 0x1800, .size = 0x0800} },
		{ "palette", rom_palette_20_3545e7e9 },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = woodpecker_configuration
} };
static aos::RegistryHandler<GameDriver> mspacmanbgc{ "mspacmanbgc", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-man (Greece Enavi)",
	.emulator = "namco",
	.creator = mspacman_init,
	.roms = {
		{ "cpu", rom_cpu_8k_a846bd10, {.size = 0x4000} },
		{ "cpu2", rom_cpu_8k_a846bd10, {.start = 0x4000, .size = 0x4000} },
		{ "video", rom_video_8k_f2c5da43, {.size = 0x0800} },
		{ "video", rom_video_8k_f2c5da43, {.start = 0x1000, .size = 0x0800} },
		{ "video", rom_video_8k_f2c5da43, {.start = 0x0800, .size = 0x0800} },
		{ "video", rom_video_8k_f2c5da43, {.start = 0x1800, .size = 0x0800} },
		{ "palette", rom_palette_20_3545e7e9 },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = woodpecker_configuration
} };
static aos::RegistryHandler<GameDriver> mspacmanbe{ "mspacmanbe", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-man (encrypted)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
		auto emul = mspacman_init(config, roms);
		emul->romDecodingFn([](const string c, uint8_t* m, const size_t s) {
			if (c == "cpu") {
				for (size_t i = 0x1000; i < 0x2000; i += 4)
				{
					if (!(i & 8))
					{
						uint8_t temp = m[i + 1];
						m[i + 1] = m[i + 2];
						m[i + 2] = temp;
					}
				}
			}});
		emul->port0().set(7);
		return emul;
	},
	.roms = {
		{ "cpu", rom_cpu_1k_d16b31b7 },
		{ "cpu", rom_cpu_1k_04e6c486 },
		{ "cpu", rom_cpu_1k_1821ee0b },
		{ "cpu", rom_cpu_1k_165a9dd8 },
		{ "cpu2", rom_cpu_1k_8c3e6de6 },
		{ "cpu2", rom_cpu_1k_206a9623 },
		{ "video", rom_video_1k_5c281d01 },
		{ "video", rom_video_1k_615af909 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mspacman_configuration
} };
static aos::RegistryHandler<GameDriver> mspacmab3{ "mspacmab3", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-man (bootleg, set 3)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
		auto emul = mspacman_init(config, roms);
		emul->romDecodingFn(eyes_decodeRom);
		emul->interruptDecodingFn([](const uint8_t v) { return (v == 0xfa) ? 0x78 : v; });
		return emul;
	},
	.roms = {
		{ "cpu", rom_cpu_1k_50b38941 },
		{ "cpu", rom_cpu_1k_195883b8 },
		{ "cpu", rom_cpu_1k_124a4507 },
		{ "cpu", rom_cpu_1k_08ac65da },
		{ "cpu2", rom_cpu_1k_50b29f09 },
		{ "cpu2", rom_cpu_800_33b09ed9 },
		{ "video", rom_video_800_b5d8c872 },
		{ "video", rom_video_800_9b2b936c },
		{ "video", rom_video_800_a70a6ac4 },
		{ "video", rom_video_800_53368498 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_200_720528b4, {.size = 0x100} },
		{ "sound", rom_sound_200_459d2618, {.size = 0x100} }
	},
	.configuration = mspacman_configuration
} };
static aos::RegistryHandler<GameDriver> mspacii{ "mspacii", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-man II (set 1)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
		auto emul = mspacman_init(config, roms);
		emul->interruptDecodingFn([](const uint8_t v) { return (v == 0xfb) ? 0x78 : v; });
		emul->portDecodingFn([](const uint8_t a, const uint8_t v) { return ((a >= 0x0d) && (a <= 0x2f)) ? (uint8_t)((v & 0xef) - ((a - 0x0d) << 4 & 0x10)) : v; });
		return emul;
	},
	.roms = {
		{ "cpu", rom_cpu_1k_df673b57 },
		{ "cpu", rom_cpu_1k_7591f606 },
		{ "cpu", rom_cpu_1k_c8ef1a7f },
		{ "cpu", rom_cpu_1k_d498f435 },
		{ "cpu2", rom_cpu_1k_fbbc3d2e },
		{ "cpu2", rom_cpu_1k_aba3096d },
		{ "video", rom_video_1k_04333722 },
		{ "video", rom_video_1k_615af909 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mspacman_configuration
} };
static aos::RegistryHandler<GameDriver> mspacii2{ "mspacii2", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-man II (set 2)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
		auto emul = mspacman_init(config, roms);
		emul->interruptDecodingFn([](const uint8_t v) { return (v == 0xfb) ? 0x78 : v; });
		emul->portDecodingFn([](const uint8_t a, const uint8_t v) { return ((a >= 0x0d) && (a <= 0x2f)) ? (uint8_t)((v & 0xef) - ((a - 0x0d) << 4 & 0x10)) : v; });
		return emul;
	},
	.roms = {
		{ "cpu", rom_cpu_1k_df673b57 },
		{ "cpu", rom_cpu_1k_7591f606 },
		{ "cpu", rom_cpu_1k_c8ef1a7f },
		{ "cpu", rom_cpu_1k_d498f435 },
		{ "cpu2", rom_cpu_1k_fbbc3d2e },
		{ "cpu2", rom_cpu_1k_aa3887c5 },
		{ "video", rom_video_1k_04333722 },
		{ "video", rom_video_1k_615af909 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mspacman_configuration
} };
static aos::RegistryHandler<GameDriver> mspacmbn{ "mspacmbn", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-man ((Novatronic)",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
		auto emul = mspacman_init(config, roms);
		emul->romDecodingFn([](const string s, uint8_t* a, const size_t v) {
			if ((s != "cpu") && (s != "cpu2"))
				return;
			uint8_t* dump = new uint8_t[v];
			for (size_t i = 0; i < v; dump[i] = a[i], ++i);
			for (size_t i = 0; i < v; a[i] = dump[i ^ 0xff], ++i); });
		return emul;
	},
	.roms = {
		{ "cpu", rom_cpu_1k_40257a62, {.size = 0x0200 } }, // 0 - 200
		{ "cpu", rom_cpu_1k_0839a86e, {.start = 0x0200, .size = 0x0200 } }, // 200 - 400
		{ "cpu", rom_cpu_1k_40257a62, {.start = 0x0400, .size = 0x0200 } }, // 400 - 600
		{ "cpu", rom_cpu_1k_0839a86e, {.start = 0x0600, .size = 0x0a00 } }, // 600 - 1000
		{ "cpu", rom_cpu_1k_269e0bdb, {.size = 0x0200 } }, // 1000 - 1200
		{ "cpu", rom_cpu_1k_f40b1ce9, {.start = 0x0200, .size = 0x0200 } }, // 1200 - 1400
		{ "cpu", rom_cpu_1k_269e0bdb, {.start = 0x0400, .size = 0x0200 } }, // 1400 - 1600
		{ "cpu", rom_cpu_1k_f40b1ce9, {.start = 0x0600, .size = 0x0a00 } }, // 1600 - 2000
		{ "cpu", rom_cpu_1k_0839a86e, {.size = 0x0200 } }, // 2000 - 2200
		{ "cpu", rom_cpu_1k_40257a62, {.start = 0x0200, .size = 0x0200 } }, // 2200 - 2400
		{ "cpu", rom_cpu_1k_0839a86e, {.start = 0x0400, .size = 0x0200 } }, // 2400 - 2600
		{ "cpu", rom_cpu_1k_40257a62, {.start = 0x0600, .size = 0x0a00 } }, // 2600 - 3000
		{ "cpu", rom_cpu_1k_f40b1ce9, {.size = 0x0200 } }, // 3000 - 3200
		{ "cpu", rom_cpu_1k_269e0bdb, {.start = 0x0200, .size = 0x0200 } }, // 3200 - 3400
		{ "cpu", rom_cpu_1k_f40b1ce9, {.start = 0x0400, .size = 0x0200 } }, // 3400 - 3600
		{ "cpu", rom_cpu_1k_269e0bdb, {.start = 0x0600, .size = 0x0a00 } }, // 3200 - 3400
		{ "cpu2", rom_cpu_1k_d0c0f66b, {.size = 0x0200 } },
		{ "cpu2", rom_cpu_1k_370b55a4, {.start = 0x0200, .size = 0x0200 } },
		{ "cpu2", rom_cpu_1k_d0c0f66b, {.start = 0x0400, .size = 0x0200 } },
		{ "cpu2", rom_cpu_1k_370b55a4, {.start = 0x0600, .size = 0x0a00 }  },
		{ "cpu2", rom_cpu_1k_370b55a4, {.size = 0x0200 } },
		{ "cpu2", rom_cpu_1k_d0c0f66b, {.start = 0x0200, .size = 0x0200 } },
		{ "cpu2", rom_cpu_1k_370b55a4, {.start = 0x0400, .size = 0x0200 } },
		{ "cpu2", rom_cpu_1k_d0c0f66b, {.start = 0x0600, .size = 0x0a00 } },
		{ "cpu2", rom_cpu_1k_0839a86e, {.size = 0x0200 } }, // 2000 - 2200
		{ "cpu2", rom_cpu_1k_40257a62, {.start = 0x0200, .size = 0x0200 } }, // 2200 - 2400
		{ "cpu2", rom_cpu_1k_0839a86e, {.start = 0x0400, .size = 0x0200 } }, // 2400 - 2600
		{ "cpu2", rom_cpu_1k_40257a62, {.start = 0x0600, .size = 0x0a00 } }, // 2600 - 3000
		{ "cpu2", rom_cpu_1k_f40b1ce9, {.size = 0x0200 } }, // 3000 - 3200
		{ "cpu2", rom_cpu_1k_269e0bdb, {.start = 0x0200, .size = 0x0200 } }, // 3200 - 3400
		{ "cpu2", rom_cpu_1k_f40b1ce9, {.start = 0x0400, .size = 0x0200 } }, // 3400 - 3600
		{ "cpu2", rom_cpu_1k_269e0bdb, {.start = 0x0600, .size = 0x0a00 } }, // 3200 - 3400
		{ "video", rom_video_800_93933d1d },
		{ "video", rom_video_800_7409fbec },
		{ "video", rom_video_800_22b0188a },
		{ "video", rom_video_800_50c7477d },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mspacman_configuration
} };
static aos::RegistryHandler<GameDriver> mspackpls{ "mspackpls", {
	.name = "Ms. Pac-man",
	.version = "Miss Packman Plus",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
		auto emul = mspacman_init(config, roms);
		emul->romDecodingFn([](const string s, uint8_t* a, const size_t v) {
			if ((s != "cpu") && (s != "cpu2"))
				return;
			for (size_t i = 0; i < v; a[i] = (a[i] & 0xe7) | ((a[i] & 0x08) << 1) | ((a[i] & 0x10) >> 1), ++i);
			});
		return emul;
	},
	.roms = {
		{ "cpu", rom_cpu_2k_88c89824, {.size = 0x0800 } },
		{ "cpu", rom_cpu_2k_19620d5d, {.start = 0x0800, .size = 0x0800 } },
		{ "cpu", rom_cpu_2k_59cb7ea0, {.start = 0x1000, .size = 0x0800 } },
		{ "cpu", rom_cpu_2k_19620d5d, {.start = 0x1800, .size = 0x0800 } },
		{ "cpu", rom_cpu_2k_19620d5d, {.size = 0x0800 } },
		{ "cpu", rom_cpu_2k_59cb7ea0, {.start = 0x0800, .size = 0x0800 } },
		{ "cpu", rom_cpu_2k_88c89824, {.start = 0x1000, .size = 0x0800 } },
		{ "cpu", rom_cpu_2k_59cb7ea0, {.start = 0x1800, .size = 0x0800 } },
		{ "cpu2", rom_cpu_2k_59cb7ea0, {.size = 0x0800 } },
		{ "cpu2", rom_cpu_2k_88c89824, {.start = 0x0800, .size = 0x0800 } },
		{ "cpu2", rom_cpu_2k_19620d5d, {.start = 0x1000, .size = 0x0800 } },
		{ "cpu2", rom_cpu_2k_88c89824, {.start = 0x1800, .size = 0x0800 } },

		{ "video", rom_video_1k_5c281d01 },
		{ "video", rom_video_1k_615af909 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mspacman_configuration
} };

static aos::RegistryHandler<GameDriver> lizwiz{ "lizwiz", {
	.name = "Lizard Wizard",
	.version = "Techstar",
	.main_version = true,
	.emulator = "namco",
	.creator = lizwiz_init,
	.roms = {
		{ "cpu", rom_cpu_1k_32bc1990 },
		{ "cpu", rom_cpu_1k_ef24b414 },
		{ "cpu", rom_cpu_1k_30bed83d },
		{ "cpu", rom_cpu_1k_dd09baeb },
		{ "cpu2", rom_cpu_1k_f6dea3a6 },
		{ "cpu2", rom_cpu_1k_f27fb5a8 },

		{ "video", rom_video_1k_45059e73 },
		{ "video", rom_video_1k_d2469717 },
		{ "palette", rom_palette_20_7549a947 },
		{ "palette", rom_palette_100_5fdca536 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = lizwiz_configuration
} };

static aos::RegistryHandler<GameDriver> ponpoko{ "ponpoko", {
	.name = "Ponpoko",
	.version = "Ponpoko (Sigma Enterprises)",
	.main_version = true,
	.emulator = "namco",
	.creator = ponpoko_init,
	.roms = {
		{ "cpu", rom_cpu_1k_ffa3c004 },
		{ "cpu", rom_cpu_1k_4a496866 },
		{ "cpu", rom_cpu_1k_17da6ca3 },
		{ "cpu", rom_cpu_1k_9d39a565 },
		{ "cpu2", rom_cpu_1k_54ca3d7d },
		{ "cpu2", rom_cpu_1k_3055c7e0 },
		{ "cpu2", rom_cpu_1k_3cbe47ca },
		{ "cpu2", rom_cpu_1k_04b63fc6 },

		{ "video", rom_video_1k_b73e1a06 },
		{ "video", rom_video_1k_62069b5d },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = lizwiz_configuration
} };
static aos::RegistryHandler<GameDriver> ponpokov{ "ponpokov", {
	.name = "Ponpoko",
	.version = "Ponpoko (Venture Line)",
	.emulator = "namco",
	.creator = ponpoko_init,
	.roms = {
		{ "cpu", rom_cpu_1k_49077667 },
		{ "cpu", rom_cpu_1k_5101781a },
		{ "cpu", rom_cpu_1k_d790ed22 },
		{ "cpu", rom_cpu_1k_4e449069 },
		{ "cpu2", rom_cpu_1k_54ca3d7d },
		{ "cpu2", rom_cpu_1k_3055c7e0 },
		{ "cpu2", rom_cpu_1k_3cbe47ca },
		{ "cpu2", rom_cpu_1k_b39be27d },

		{ "video", rom_video_1k_b73e1a06 },
		{ "video", rom_video_1k_62069b5d },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = ponpoko_configuration
} };
static aos::RegistryHandler<GameDriver> candory{ "candory", {
	.name = "Ponpoko",
	.version = "Candory bootleg",
	.emulator = "namco",
	.creator = ponpoko_init,
	.roms = {
		{ "cpu", rom_cpu_1k_ffa3c004 },
		{ "cpu", rom_cpu_1k_4a496866 },
		{ "cpu", rom_cpu_1k_17da6ca3 },
		{ "cpu", rom_cpu_1k_9d39a565 },
		{ "cpu2", rom_cpu_1k_54ca3d7d },
		{ "cpu2", rom_cpu_1k_3055c7e0 },
		{ "cpu2", rom_cpu_1k_3cbe47ca },
		{ "cpu2", rom_cpu_1k_04b63fc6 },

		{ "video", rom_video_1k_7d16bdff },
		{ "video", rom_video_1k_e08ac188 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = ponpoko_configuration
} };

/*
GAME( 1983, theglobme,suprglob, woodpek,  theglobp, epospm_state,  empty_init,    ROT90,  "Magic Electronics Inc.",   "The Glob (Pacman hardware, Magic Electronics Inc. license)", MACHINE_SUPPORTS_SAVE )
*/