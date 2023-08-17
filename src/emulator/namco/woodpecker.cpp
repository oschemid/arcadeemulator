#include "pacmansystem.h"
#include "gpu/pacmangpu.h"
#include "registry.h"


namespace aos::namco
{
	class Woodpecker : public PacmanSystem<PacmanGpu>
	{
	public:
		Woodpecker(const vector<aos::emulator::RomConfiguration>& roms,
			const aos::emulator::GameConfiguration& game) :
			PacmanSystem(roms, game,
				PacmanGpu::create({
				.orientation = geometry_t::rotation_t::ROT90,
				.tileModel = PacmanGpu::Configuration::TileModel::PACMAN,
				.romModel = PacmanGpu::Configuration::RomModel::WOODPECKER,
				.spriteAddress = 0xff0 }
				))
		{
			//	_port0 = 0x80;
			_port0.set(0, "_JOY1_UP", true);
			_port0.set(1, "_JOY1_LEFT", true);
			_port0.set(2, "_JOY1_RIGHT", true);
			_port0.set(3, "_JOY1_DOWN", true);
			_port0.set(4, "racktest");
			_port0.set(5, "_COIN", true);
			_port0.set(6, "_COIN2", true);
			//	_port0.set(7, "racktest");
			_port0.init(game);

			//	_port1 = 0x80;
			_port1.set(0, "_JOY1_UP", true);
			_port1.set(1, "_JOY1_LEFT", true);
			_port1.set(2, "_JOY1_RIGHT", true);
			_port1.set(3, "_JOY1_DOWN", true);
			_port1.set(4, "_JOY1_FIRE", true);
			_port1.set(5, "_START1", true);
			_port1.set(6, "_START2", true);
			_port1.init(game);

			_port2 = 0x80;
			_port2.set(0, "coinage");
			_port2.set(2, "lives");
			_port2.set(4, "bonus");
			_port2.set(6, "cabinet");
			_port2.init(game);

			_port3 = 0xff;
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
			_mmu.map(0x5003, 0x5003).writefn([this](const uint16_t, const uint8_t value) { static_cast<PacmanGpu*>(&(*_gpu))->flip(((value & 1) == 1) ? true : false); });
			_mmu.map(0x5040, 0X505f).writefn([this](const uint16_t address, const uint8_t value) { _wsg.write(address, value); });
			_mmu.map(0x5060, 0x506f).writefn([this](const uint16_t address, const uint8_t value) { static_cast<PacmanGpu*>(&(*_gpu))->writeSpritePos(address, value); });
			_mmu.map(0x8000, 0xbfff, "cpu2").rom();
		}
	};
}
	

static aos::RegistryHandler<aos::emulator::GameDriver> woodpecker{ "woodpecker", {
	.name = "Woodpecker",
	.version = "Amenip",
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<aos::namco::Woodpecker>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x1000, 0x37ea66ca },
		{ "cpu2", 0, 0x1000, 0xcd115dba },
		{ "cpu2", 0, 0x1000, 0xd40b2321 },
		{ "cpu2", 0, 0x1000, 0x024092f4 },
		{ "cpu2", 0, 0x1000, 0x18ef0fc8 },
		{ "video", 0, 0x0800, 0x15a87f62 },
		{ "video", 0, 0x0800, 0xab4abd88 },
		{ "video", 0, 0x0800, 0x5b9ba95b },
		{ "video", 0, 0x0800, 0xd7b80a45 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0xd8772167 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = {
		.switches = {{ "coinage", 1, "Coinage", {"Free", "1C 1C", "1C 2C", "2C 1C"}},
					 { "lives", 2, "Lives", {"1", "2", "3", "5"} },
					 { "bonus", 1, "Bonus", {"5000", "10000", "15000", "None"} },
					 { "cabinet", 1, "Cabinet", {"Cocktail", "Upright"} },
					 { "racktest", 1, "Rack Test", { "On", "Off"} }
		  }
	}
} };
