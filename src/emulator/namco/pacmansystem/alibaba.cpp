#include "../pacmansystem.h"
#include "../gpu/pacmangpu.h"
#include "registry.h"
#include "rom_library.h"
#include "configuration.h"


using aos::namco::PacmanSystem;
using aos::namco::PacmanGpu;
using aos::emulator::GameDriver;


//
// Pacman System - Alibaba
//
// Configuration:
//   * ROM 0x0000 - 0x4000
//   * ROM 0x8000 - 0x9000
//   * RAM 0x9000 - 0xA000
//   * ROM 0xA000 - 0xA800
//   * Ports 0 & 1
//   * DipSwitch 0
//
class Alibaba : public PacmanSystem<PacmanGpu>
	{
	public:
		Alibaba(const vector<aos::mmu::RomMapping>& roms,
			const aos::emulator::GameConfiguration& game) :
			PacmanSystem(roms, game,
				PacmanGpu::create({
				.orientation = geometry_t::rotation_t::ROT90,
				.tileModel = PacmanGpu::Configuration::TileModel::PACMAN,
				.spriteAddress = 0xef0 }
				))
		{
			_port2.set(0, "dsw1-0");
			_port2.set(2, "dsw1-2");
			_port2.set(4, "dsw1-4");
			_port2.set(6, "dsw1-6");
			_port2.init(game);
		}
		static std::unique_ptr<Alibaba> alibaba_instance(const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms)
		{
			auto emul = std::make_unique<Alibaba>(roms, config);
			emul->port0().joystick1().coin().fire1(6);
			emul->port1().starts();
			return emul;
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
			_mmu.map(0x5040, 0X504f).writefn([this](const uint16_t address, const uint8_t value) { _wsg.write(address & 0xff, value); });
			_mmu.map(0x5050, 0x505f).writefn([this](const uint16_t address, const uint8_t value) { _gpu->writeSpritePos(address, value); });
			_mmu.map(0x5060, 0X506f).writefn([this](const uint16_t address, const uint8_t value) { _wsg.write(0x10 | address, value); });
			_mmu.map(0x50c2, 0x50c2).writefn([this](const uint16_t, const uint8_t value) { this->enableInterrupts(((value & 1) == 1) ? true : false); });
			_mmu.map(0x50c1, 0x50c1).writefn([this](const uint16_t, const uint8_t value) { _gpu->flip(((value & 1) == 1) ? true : false); });
			_mmu.map(0x8000, 0x8fff).name("cpu2").rom();
			_mmu.map(0x9000, 0x9fff).ram();
			_mmu.map(0xa000, 0xa7ff).name("cpu3").rom();
		}

		uint16_t _mystery{ 0 };
	};

static aos::RegistryHandler<GameDriver> alibaba{ "alibaba", {
	.name = "Alibaba & 40 Thieves",
	.version = "Sega",
	.emulator = "namco",
	.creator = Alibaba::alibaba_instance,
	.roms = {
		{ "cpu", rom_cpu_1k_38d701aa },
		{ "cpu", rom_cpu_1k_3d0e35f3 },
		{ "cpu", rom_cpu_1k_823bee89 },
		{ "cpu", rom_cpu_1k_474d032f },
		{ "cpu2", rom_cpu_1k_5ab315c1 },
		{ "cpu3", rom_cpu_800_438d0357 },
		{ "video", rom_video_800_85bcb8f8 },
		{ "video", rom_video_800_38e50862 },
		{ "video", rom_video_800_b5715c86 },
		{ "video", rom_video_800_713086b3 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = alibaba_configuration
} };
static aos::RegistryHandler<GameDriver> alibabab{ "alibabab", {
	.name = "Alibaba & 40 Thieves",
	.version = "Mustafa & 40 Thieves",
	.emulator = "namco",
	.creator = Alibaba::alibaba_instance,
	.roms = {
		{ "cpu", rom_cpu_1k_38d701aa },
		{ "cpu", rom_cpu_1k_3d0e35f3 },
		{ "cpu", rom_cpu_1k_823bee89 },
		{ "cpu", rom_cpu_1k_474d032f },
		{ "cpu2", rom_cpu_1k_ae2f4aac },
		{ "cpu3", rom_cpu_800_438d0357 },
		{ "video", rom_video_800_85bcb8f8 },
		{ "video", rom_video_800_38e50862 },
		{ "video", rom_video_800_b5715c86 },
		{ "video", rom_video_800_713086b3 },
		{ "palette", rom_palette_20_2fc650bd },
		{ "palette", rom_palette_100_3eb3a8e4 },
		{ "sound", rom_sound_100_a9cc86bf }
	},
	.configuration = alibaba_configuration
} };
