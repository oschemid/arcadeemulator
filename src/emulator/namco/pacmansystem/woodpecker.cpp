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
