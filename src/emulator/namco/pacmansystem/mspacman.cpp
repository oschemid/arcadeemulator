#include "registry.h"
#include "../pacmansystem.h"
#include "../gpu/pacmangpu.h"

#include "rom_library.h"
#include "configuration.h"


using aos::namco::PacmanSystem;
using aos::namco::PacmanGpu;
using aos::emulator::GameDriver;


//
// Pacman System - MsPacman
//
// Configuration:
//   * ROM 0x0000 - 0x4000 (bank 1)
//   * ROM 0x0000 - 0x4000 + 0x8000 - 0xC000 (bank 2)
//   * Ports 0 & 1
//   * DipSwitch 0
//
class MsPacman : public PacmanSystem<PacmanGpu>
{
public:
	MsPacman(const vector<aos::mmu::RomMapping>& roms,
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
		_port2.set(4, "dsw1-4");
		_port2.set(6, "dsw1-6");
		_port2.init(game);
	}
	static std::unique_ptr<MsPacman> mspacman_init(const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
		// Port 0 - JOYSTICK 0x0f CHEAT 0x10 COIN 0x20
		auto emul = std::make_unique<MsPacman>(roms, config);
		emul->port0().joystick1().coin();
		emul->port0().set(4, "_CHEAT", true);
		emul->port1().starts();
		return emul;
	}

	void init(aos::display::RasterDisplay* raster) override
	{
		PacmanSystem::init(raster);
		_mmu.selectBank(2);

		// Patch
		static const uint16_t tab[] = {
	0x0410, 0x8008, 0x08E0, 0x81D8, 0x0A30, 0x8118, 0x0BD0, 0x80D8,
	0x0C20, 0x8120, 0x0E58, 0x8168, 0x0EA8, 0x8198, 0x1000, 0x8020,
	0x1008, 0x8010, 0x1288, 0x8098, 0x1348, 0x8048, 0x1688, 0x8088,
	0x16B0, 0x8188, 0x16D8, 0x80C8, 0x16F8, 0x81C8, 0x19A8, 0x80A8,
	0x19B8, 0x81A8, 0x2060, 0x8148, 0x2108, 0x8018, 0x21A0, 0x81A0,
	0x2298, 0x80A0, 0x23E0, 0x80E8, 0x2418, 0x8000, 0x2448, 0x8058,
	0x2470, 0x8140, 0x2488, 0x8080, 0x24B0, 0x8180, 0x24D8, 0x80C0,
	0x24F8, 0x81C0, 0x2748, 0x8050, 0x2780, 0x8090, 0x27B8, 0x8190,
	0x2800, 0x8028, 0x2B20, 0x8100, 0x2B30, 0x8110, 0x2BF0, 0x81D0,
	0x2CC0, 0x80D0, 0x2CD8, 0x80E0, 0x2CF0, 0x81E0, 0x2D60, 0x8160
		};
		for (size_t i = 0; i < 80; i += 2) {
			for (uint8_t j = 0; j < 8; ++j) {
				uint8_t a = _mmu.read(tab[i + 1] + j);
				_mmu.patch(tab[i] + j, _mmu.read(tab[i + 1] + j));
			}
		}
		_mmu.before_fn([this](const uint16_t address) { beforeRW(address); });
	}

protected:
	void mapping() override
	{
		_mmu.map(0, 0x3fff).mirror(0x7fff).bank(1).name("cpu1").rom();
		_mmu.map(0, 0x3fff).bank(2).name("cpu2").rom().decodefn(MsPacman::decodeU7);
		_mmu.map(0x8000, 0xbfff).bank(2).name("cpu2b").rom().decodefn(MsPacman::decodeU56);
		_mmu.map(0x4000, 0x4fff).mirror(0x7fff).readfn([this](const uint16_t a) { return _gpu->readVRAM(a); }).writefn([this](const uint16_t a, const uint8_t v) { _gpu->writeVRAM(a, v); });
		_mmu.map(0x5000, 0x503f).readfn([this](const uint16_t) { return _port0.get(); });
		_mmu.map(0x5040, 0x507f).readfn([this](const uint16_t) { return _port1.get(); });
		_mmu.map(0x5080, 0x50bf).readfn([this](const uint16_t) { return _port2.get(); });
		_mmu.map(0x5000, 0x5000).mirror(0x7fff).writefn([this](const uint16_t, const uint8_t value) { this->enableInterrupts(((value & 1) == 1) ? true : false); });
		_mmu.map(0x5003, 0x5003).mirror(0x7fff).writefn([this](const uint16_t, const uint8_t value) { _gpu->flip(((value & 1) == 1) ? true : false); });
		_mmu.map(0x5040, 0X505f).mirror(0x7fff).writefn([this](const uint16_t address, const uint8_t value) { _wsg.write(address & 0xff, value); });
		_mmu.map(0x5060, 0x506f).mirror(0x7fff).writefn([this](const uint16_t address, const uint8_t value) { _gpu->writeSpritePos(address, value); });
	}
	static void decodeU7(uint8_t* memory, const size_t size)
	{
		uint8_t* tmp = new uint8_t[0x1000];
		for (size_t offset = 0; offset < 0x1000; ++offset)
		{
			const size_t newoffset =
				(offset & 0xf807) |
				((offset & 0x0270) >> 1) |
				((offset & 0x0008) << 7) |
				((offset & 0x0080) << 2) |
				((offset & 0x0400) >> 3) |
				((offset & 0x0100) >> 2);

			const uint8_t value = memory[0x3000 + newoffset];
			tmp[offset] = ((value & 0x01) << 7) | ((value & 0x10) << 2) | (value & 0x20) | ((value & 0xc0) >> 3) | ((value & 0x0e) >> 1);
		}
		for (size_t offset = 0; offset < 0x1000; ++offset)
		{
			memory[0x3000 + offset] = tmp[offset];
		}
		delete[] tmp;
	}
	static void decodeU56(uint8_t* memory, const size_t size)
	{
		uint8_t* tmp = new uint8_t[0x1000];
		for (size_t offset = 0; offset < 0x0800; ++offset)
		{
			const size_t newoffset =
				(offset & 0xf807) |
				((offset & 0x0400) >> 4) |
				((offset & 0x0200) >> 2) |
				((offset & 0x0180) << 2) |
				((offset & 0x0050) >> 1) |
				((offset & 0x0008) << 1) |
				((offset & 0x0020) << 3);
			const uint8_t value = memory[newoffset];

			tmp[offset] = ((value & 0x01) << 7) | ((value & 0x10) << 2) | (value & 0x20) | ((value & 0xc0) >> 3) | ((value & 0x0e) >> 1);
		}
		for (size_t offset = 0; offset < 0x0800; ++offset)
		{
			memory[offset] = tmp[offset];
		}
		for (size_t offset = 0; offset < 0x1000; ++offset)
		{
			const size_t newoffset =
				(offset & 0x807) |
				((offset & 0x400) >> 3) |
				((offset & 0x100) >> 2) |
				((offset & 0x080) << 2) |
				((offset & 0x270) >> 1) |
				((offset & 0x008) << 7);
			const uint8_t value = memory[0x0800 + newoffset];
			tmp[offset] = ((value & 0x01) << 7) | ((value & 0x10) << 2) | (value & 0x20) | ((value & 0xc0) >> 3) | ((value & 0x0e) >> 1);
		}
		for (size_t offset = 0; offset < 0x0800; ++offset)
		{
			memory[0x0800 + offset] = tmp[0x0800 + offset];
			memory[0x1000 + offset] = tmp[offset];
		}
	}
	void beforeRW(const uint16_t address)
	{
		if ((address >= 0x0038 && address <= 0x003f) ||
			(address >= 0x03b0 && address <= 0x03b7) ||
			(address >= 0x1600 && address <= 0x1607) ||
			(address >= 0x2120 && address <= 0x2127) ||
			(address >= 0x3ff0 && address <= 0x3ff7) ||
			(address >= 0x8000 && address <= 0x8007) ||
			(address >= 0x97f0 && address <= 0x97f7))
		{
			_mmu.selectBank(1);
		}
		if (address >= 0x3ff8 && address <= 0x3fff) {
			_mmu.selectBank(2);
		}
	}
};

static aos::RegistryHandler<GameDriver> mspacman{ "mspacman", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-man (Midway)",
	.main_version = true,
	.emulator = "namco",
	.creator = MsPacman::mspacman_init,
	.roms = {
		{ "cpu1", rom_cpu_1k_c1e6ab10 },
		{ "cpu1", rom_cpu_1k_1a6fb2d4 },
		{ "cpu1", rom_cpu_1k_bcdd1beb },
		{ "cpu1", rom_cpu_1k_817d94e3 },
		{ "cpu2", rom_cpu_1k_c1e6ab10 },
		{ "cpu2", rom_cpu_1k_1a6fb2d4 },
		{ "cpu2", rom_cpu_1k_bcdd1beb },
		{ "cpu2", rom_cpu_1k_c82cd714 },
		{ "cpu2b", rom_cpu_800_f45fbbcd },
		{ "cpu2b", rom_cpu_1k_a90e7000 },
		{ "cpu2b", rom_cpu_1k_1a6fb2d4, {.start = 0x0800 } },
		{ "cpu2b", rom_cpu_1k_bcdd1beb },
		{ "cpu2b", rom_cpu_1k_817d94e3 },
		{ "video", rom_video_1k_5c281d01 },
		{ "video", rom_video_1k_615af909 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mspacman_configuration
} };
static aos::RegistryHandler<GameDriver> mspacmnf{ "mspacmnf", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-man (Midway speedup hack)",
	.emulator = "namco",
	.creator = MsPacman::mspacman_init,
	.roms = {
		{ "cpu1", rom_cpu_1k_c1e6ab10 },
		{ "cpu1", rom_cpu_1k_720dc3ee },
		{ "cpu1", rom_cpu_1k_bcdd1beb },
		{ "cpu1", rom_cpu_1k_817d94e3 },
		{ "cpu2", rom_cpu_1k_c1e6ab10 },
		{ "cpu2", rom_cpu_1k_720dc3ee },
		{ "cpu2", rom_cpu_1k_bcdd1beb },
		{ "cpu2", rom_cpu_1k_c82cd714 },
		{ "cpu2b", rom_cpu_800_f45fbbcd },
		{ "cpu2b", rom_cpu_1k_a90e7000 },
		{ "cpu2b", rom_cpu_1k_720dc3ee, 0x0800 },
		{ "cpu2b", rom_cpu_1k_bcdd1beb },
		{ "cpu2b", rom_cpu_1k_817d94e3 },
		{ "video", rom_video_1k_5c281d01 },
		{ "video", rom_video_1k_615af909 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mspacman_configuration
} };
static aos::RegistryHandler<GameDriver> mspacmat{ "mspacmat", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac Attack",
	.emulator = "namco",
	.creator = MsPacman::mspacman_init,
	.roms = {
		{ "cpu1", rom_cpu_1k_c1e6ab10 },
		{ "cpu1", rom_cpu_1k_1a6fb2d4 },
		{ "cpu1", rom_cpu_1k_bcdd1beb },
		{ "cpu1", rom_cpu_1k_817d94e3 },
		{ "cpu2", rom_cpu_1k_c1e6ab10 },
		{ "cpu2", rom_cpu_1k_1a6fb2d4 },
		{ "cpu2", rom_cpu_1k_bcdd1beb },
		{ "cpu2", rom_cpu_1k_c82cd714 },
		{ "cpu2b", rom_cpu_800_f45fbbcd },
		{ "cpu2b", rom_cpu_1k_f6d83f4d },
		{ "cpu2b", rom_cpu_1k_1a6fb2d4, 0x0800 },
		{ "cpu2b", rom_cpu_1k_bcdd1beb },
		{ "cpu2b", rom_cpu_1k_817d94e3 },
		{ "video", rom_video_1k_5c281d01 },
		{ "video", rom_video_1k_615af909 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mspacman_configuration
} };
static aos::RegistryHandler<GameDriver> msheartb{ "msheartb", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-Man Heart Burn",
	.main_version = true,
	.emulator = "namco",
	.creator = MsPacman::mspacman_init,
	.roms = {
		{ "cpu1", rom_cpu_1k_c1e6ab10 },
		{ "cpu1", rom_cpu_1k_1a6fb2d4 },
		{ "cpu1", rom_cpu_1k_bcdd1beb },
		{ "cpu1", rom_cpu_1k_817d94e3 },
		{ "cpu2", rom_cpu_1k_c1e6ab10 },
		{ "cpu2", rom_cpu_1k_1a6fb2d4 },
		{ "cpu2", rom_cpu_1k_bcdd1beb },
		{ "cpu2", rom_cpu_1k_c82cd714 },
		{ "cpu2b", rom_cpu_800_f45fbbcd },
		{ "cpu2b", rom_cpu_1k_a90e7000 },
		{ "cpu2b", rom_cpu_1k_1a6fb2d4, 0x0800 },
		{ "cpu2b", rom_cpu_1k_bcdd1beb },
		{ "cpu2b", rom_cpu_1k_817d94e3 },
		{ "video", rom_video_1k_5431d4c4 },
		{ "video", rom_video_1k_ceb50654 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mspacman_configuration
} };
static aos::RegistryHandler<GameDriver> pacgal2{ "pacgal2", {
	.name = "Ms. Pac-man",
	.version = "Pac-Gal (set 2)",
	.emulator = "namco",
	.creator = MsPacman::mspacman_init,
	.roms = {
		{ "cpu1", rom_cpu_800_8167fffc },
		{ "cpu1", rom_cpu_800_618bd9b3 },
		{ "cpu1", rom_cpu_800_7d177853 },
		{ "cpu1", rom_cpu_800_4f91d245 },
		{ "cpu1", rom_cpu_800_d0f20748 },
		{ "cpu1", rom_cpu_800_a948ce83 },
		{ "cpu1", rom_cpu_800_4c842da6 },
		{ "cpu1", rom_cpu_800_022764dc },
		{ "cpu2", rom_cpu_800_8167fffc },
		{ "cpu2", rom_cpu_800_618bd9b3 },
		{ "cpu2", rom_cpu_800_7d177853 },
		{ "cpu2", rom_cpu_800_4f91d245 },
		{ "cpu2", rom_cpu_800_d0f20748 },
		{ "cpu2", rom_cpu_800_a948ce83 },
		{ "cpu2", rom_cpu_1k_5fafec7c },
		{ "cpu2b", rom_cpu_800_f45fbbcd },
		{ "cpu2b", rom_cpu_1k_3fdcb271 },
		{ "cpu2b", rom_cpu_800_4f91d245 },
		{ "cpu2b", rom_cpu_800_d0f20748 },
		{ "cpu2b", rom_cpu_800_a948ce83 },
		{ "cpu2b", rom_cpu_800_4c842da6 },
		{ "cpu2b", rom_cpu_800_022764dc },
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
static aos::RegistryHandler<GameDriver> mspacmancr{ "mspacmancr", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-Man (Crush Roller Hardware)",
	.emulator = "namco",
	.creator = MsPacman::mspacman_init,
	.roms = {
		{ "cpu1", rom_cpu_1k_c1e6ab10 },
		{ "cpu1", rom_cpu_1k_1a6fb2d4 },
		{ "cpu1", rom_cpu_1k_bcdd1beb },
		{ "cpu1", rom_cpu_1k_817d94e3 },
		{ "cpu2", rom_cpu_1k_c1e6ab10 },
		{ "cpu2", rom_cpu_1k_1a6fb2d4 },
		{ "cpu2", rom_cpu_1k_bcdd1beb },
		{ "cpu2", rom_cpu_1k_bd972c0c },
		{ "cpu2b", rom_cpu_800_f45fbbcd },
		{ "cpu2b", rom_cpu_1k_a90e7000 },
		{ "cpu2b", rom_cpu_1k_1a6fb2d4, 0x0800 },
		{ "cpu2b", rom_cpu_1k_bcdd1beb },
		{ "cpu2b", rom_cpu_1k_817d94e3 },
		{ "video", rom_video_1k_c45d4857 },
		{ "video", rom_video_1k_615af909 },
		{ "palette", rom_palette_20_ff344446 },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = mspacman_configuration
} };
