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
			_mmu.map(0, 0x3fff).mirror(0x7fff).name("cpu").rom();
			_mmu.map(0x4000, 0x4fff).mirror(0x7fff).readfn([this](const uint16_t a) { return _gpu->readVRAM(a); }).writefn([this](const uint16_t a, const uint8_t v) { _gpu->writeVRAM(a, v); });
			_mmu.map(0x5000, 0x503f).mirror(0x7fff).readfn([this](const uint16_t) { return _port0.get(); });
			_mmu.map(0x5040, 0x507f).mirror(0x7fff).readfn([this](const uint16_t) { return _port1.get(); });
			_mmu.map(0x5080, 0x50bf).mirror(0x7fff).readfn([this](const uint16_t) { return _port2.get(); });
			_mmu.map(0x5000, 0x5000).mirror(0x7fff).writefn([this](const uint16_t, const uint8_t value) { _interrupt_enabled = ((value & 1) == 1) ? true : false; });
			_mmu.map(0x5003, 0x5003).mirror(0x7fff).writefn([this](const uint16_t, const uint8_t value) { _gpu->flip(((value & 1) == 1) ? true : false); });
			_mmu.map(0x5040, 0X505f).mirror(0x7fff).writefn([this](const uint16_t address, const uint8_t value) { _wsg.write(address, value); });
			_mmu.map(0x5060, 0x506f).mirror(0x7fff).writefn([this](const uint16_t address, const uint8_t value) { _gpu->writeSpritePos(address, value); });
		}
	};

class PacmanPlus : public Pacman
{
public:
	PacmanPlus(const vector<aos::emulator::RomConfiguration>& roms,
		const aos::emulator::GameConfiguration& game) :
		Pacman(roms, game)
	{
	}

	void mapping() override
	{
		_mmu.map(0, 0x3fff).mirror(0x7fff).name("cpu").rom().decodefn(PacmanPlus::decodeRom);
		_mmu.map(0x4000, 0x4fff).mirror(0x7fff).readfn([this](const uint16_t a) { return _gpu->readVRAM(a); }).writefn([this](const uint16_t a, const uint8_t v) { _gpu->writeVRAM(a, v); });
		_mmu.map(0x5000, 0x503f).mirror(0x7fff).readfn([this](const uint16_t) { return _port0.get(); });
		_mmu.map(0x5040, 0x507f).mirror(0x7fff).readfn([this](const uint16_t) { return _port1.get(); });
		_mmu.map(0x5080, 0x50bf).mirror(0x7fff).readfn([this](const uint16_t) { return _port2.get(); });
		_mmu.map(0x5000, 0x5000).mirror(0x7fff).writefn([this](const uint16_t, const uint8_t value) { _interrupt_enabled = ((value & 1) == 1) ? true : false; });
		_mmu.map(0x5003, 0x5003).mirror(0x7fff).writefn([this](const uint16_t, const uint8_t value) { _gpu->flip(((value & 1) == 1) ? true : false); });
		_mmu.map(0x5040, 0X505f).mirror(0x7fff).writefn([this](const uint16_t address, const uint8_t value) { _wsg.write(address, value); });
		_mmu.map(0x5060, 0x506f).mirror(0x7fff).writefn([this](const uint16_t address, const uint8_t value) { _gpu->writeSpritePos(address, value); });
	}

protected:
	static uint8_t decodeRom(const uint16_t address, const uint8_t value)
	{
		static const uint8_t swap_xor_table[6][9] =
		{
			{ 7,6,5,4,3,2,1,0, 0x00 },
			{ 7,6,5,4,3,2,1,0, 0x28 },
			{ 6,1,3,2,5,7,0,4, 0x96 },
			{ 6,1,5,2,3,7,0,4, 0xbe },
			{ 0,3,7,6,4,2,1,5, 0xd5 },
			{ 0,3,4,6,7,2,1,5, 0xdd }
		};
		static const int picktable[32] =
		{
			0,2,4,2,4,0,4,2,2,0,2,2,4,0,4,2,
			2,2,4,0,4,2,4,0,0,4,0,4,4,2,4,2
		};
		uint32_t method = 0;
		const uint8_t* tbl;

		method = picktable[
			(address & 0x001) |
				((address & 0x004) >> 1) |
				((address & 0x020) >> 3) |
				((address & 0x080) >> 4) |
				((address & 0x200) >> 5)];

		if ((address & 0x800) == 0x800)
			method ^= 1;

		tbl = swap_xor_table[method];
		uint8_t res = 0;
		res |= (((value >> tbl[0]) & 1) << 7);
		res |= (((value >> tbl[1]) & 1) << 6);
		res |= (((value >> tbl[2]) & 1) << 5);
		res |= (((value >> tbl[3]) & 1) << 4);
		res |= (((value >> tbl[4]) & 1) << 3);
		res |= (((value >> tbl[5]) & 1) << 2);
		res |= (((value >> tbl[6]) & 1) << 1);
		res |= ((value >> tbl[7]) & 1);
		res ^= tbl[8];
		return res;
	}
};

class MsPacman : public PacmanSystem<PacmanGpu>
{
	public:
	MsPacman(const vector<aos::emulator::RomConfiguration>& roms,
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
	void init(aos::display::RasterDisplay* raster) override
	{
		PacmanSystem::init(raster);
		_mmu.selectBank(1);
	}

protected:
	void mapping() override
	{
		_mmu.map(0, 0x3fff).mirror(0x7fff).bank(1).name("cpu1").rom();
		_mmu.map(0x4000, 0x4fff).mirror(0x7fff).readfn([this](const uint16_t a) { return _gpu->readVRAM(a); }).writefn([this](const uint16_t a, const uint8_t v) { _gpu->writeVRAM(a, v); });
		_mmu.map(0x5000, 0x503f).readfn([this](const uint16_t) { return _port0.get(); });
		_mmu.map(0x5040, 0x507f).readfn([this](const uint16_t) { return _port1.get(); });
		_mmu.map(0x5080, 0x50bf).readfn([this](const uint16_t) { return _port2.get(); });
		_mmu.map(0x5000, 0x5000).mirror(0x7fff).writefn([this](const uint16_t, const uint8_t value) { _interrupt_enabled = ((value & 1) == 1) ? true : false; });
		_mmu.map(0x5003, 0x5003).mirror(0x7fff).writefn([this](const uint16_t, const uint8_t value) { _gpu->flip(((value & 1) == 1) ? true : false); });
		_mmu.map(0x5040, 0X505f).mirror(0x7fff).writefn([this](const uint16_t address, const uint8_t value) { _wsg.write(address, value); });
		_mmu.map(0x5060, 0x506f).mirror(0x7fff).writefn([this](const uint16_t address, const uint8_t value) { _gpu->writeSpritePos(address, value); });
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


static aos::emulator::GameConfiguration mspacman_configuration = {
		.switches = {{ "coinage", 1, "Coinage", {"Free", "1C/1C", "1C/2C", "2C/1C"} },
					 { "lives", 2, "Lives", {"1", "2", "3", "5"} },
					 { "bonus", 0, "Bonus", {"10000 points", "15000 points", "20000 points", "no"} },
					 { "difficulty", 1, "Difficulty", {"Hard", "Normal"} },
					 { "rackadvance", 1, "Rackadvance", {"On", "Off"} },
					 { "boardtest", 1, "Board test", {"On", "Off"} },
					 { "cabinet", 1, "Cabinet", {"Table", "Upright"} },
					 { "service", 1, "Service", {"On", "Off"} }
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


static aos::RegistryHandler<aos::emulator::GameDriver> pacplus{ "pacplus", {
	.name = "Pacman Plus",
	.version = "Pac-Man Plus",
	.main_version = true,
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<PacmanPlus>(roms, config); },
	.roms = {
		{ "cpu", 0, 0x1000, 0xd611ef68 },
		{ "cpu", 0, 0x1000, 0xc7207556 },
		{ "cpu", 0, 0x1000, 0xae379430 },
		{ "cpu", 0, 0x1000, 0x5a6dff7b },
		{ "video", 0, 0x1000, 0x022c35da },
		{ "video", 0, 0x1000, 0x4de65cdd },
		{ "palette", 0, 0x20, 0x063dd53a },
		{ "palette", 0, 0x100, 0xe271a166 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = pacman_configuration
} };


static aos::RegistryHandler<aos::emulator::GameDriver> mspacman{ "mspacman", {
	.name = "Ms. Pac-man",
	.version = "Ms. Pac-man",
	.main_version = true,
	.emulator = "namco",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::emulator::RomsConfiguration& roms) { return std::make_unique<MsPacman>(roms, config); },
	.roms = {
		{ "cpu1", 0, 0x1000, 0xc1e6ab10 },
		{ "cpu1", 0, 0x1000, 0x1a6fb2d4 },
		{ "cpu1", 0, 0x1000, 0xbcdd1beb },
		{ "cpu1", 0, 0x1000, 0x817d94e3 },
		{ "video", 0, 0x1000, 0x5c281d01 },
		{ "video", 0, 0x1000, 0x615af909 },
		{ "palette", 0, 0x20, 0x2fc650bd },
		{ "palette", 0, 0x100, 0x3eb3a8e4 },
		{ "sound", 0, 0x100, 0xa9cc86bf }
	},
	.configuration = mspacman_configuration
} };

/*
	ROM_REGION( 0x20000, "maincpu", 0 ) // 64k for code+64k for decrypted code
	ROM_LOAD( "pacman.6e",    0x0000, 0x1000, CRC(c1e6ab10) SHA1(e87e059c5be45753f7e9f33dff851f16d6751181) )
	ROM_LOAD( "pacman.6f",    0x1000, 0x1000, CRC(1a6fb2d4) SHA1(674d3a7f00d8be5e38b1fdc208ebef5a92d38329) )
	ROM_LOAD( "pacman.6h",    0x2000, 0x1000, CRC(bcdd1beb) SHA1(8e47e8c2c4d6117d174cdac150392042d3e0a881) )
	ROM_LOAD( "pacman.6j",    0x3000, 0x1000, CRC(817d94e3) SHA1(d4a70d56bb01d27d094d73db8667ffb00ca69cb9) )
	ROM_LOAD( "u5",           0x8000, 0x0800, CRC(f45fbbcd) SHA1(b26cc1c8ee18e9b1daa97956d2159b954703a0ec) )
	ROM_LOAD( "u6",           0x9000, 0x1000, CRC(a90e7000) SHA1(e4df96f1db753533f7d770aa62ae1973349ea4cf) )
	ROM_LOAD( "u7",           0xb000, 0x1000, CRC(c82cd714) SHA1(1d8ac7ad03db2dc4c8c18ade466e12032673f874) )

	ROM_REGION( 0x2000, "gfx1", 0 )
	ROM_LOAD( "5e",           0x0000, 0x1000, CRC(5c281d01) SHA1(5e8b472b615f12efca3fe792410c23619f067845) )
	ROM_LOAD( "5f",           0x1000, 0x1000, CRC(615af909) SHA1(fd6a1dde780b39aea76bf1c4befa5882573c2ef4) )

	ROM_REGION( 0x0120, "proms", 0 )
	ROM_LOAD( "82s123.7f",    0x0000, 0x0020, CRC(2fc650bd) SHA1(8d0268dee78e47c712202b0ec4f1f51109b1f2a5) )
	ROM_LOAD( "82s126.4a",    0x0020, 0x0100, CRC(3eb3a8e4) SHA1(19097b5f60d1030f8b82d9f1d3a241f93e5c75d6) )

/*
GAME(1980, pacmanpe, puckman, pacman, pacmanpe, pacman_state, empty_init, ROT90, "bootleg (Petaco SA)", "Come Come (Petaco SA bootleg of Puck Man)", MACHINE_SUPPORTS_SAVE) // might have a speed-up button, check
GAME(1980, pacuman, puckman, pacman, pacuman, pacman_state, empty_init, ROT90, "bootleg (Recreativos Franco S.A.)", "Pacu-Man (Spanish bootleg of Puck Man)", MACHINE_SUPPORTS_SAVE) // common bootleg in Spain, code is shifted a bit compared to the Puck Man sets. Title & Manufacturer info from cabinet/PCB, not displayed ingame
GAME(1981, piranha, puckman, piranha, mspacman, pacman_state, init_eyes, ROT90, "GL (US Billiards license)", "Piranha", MACHINE_SUPPORTS_SAVE)
GAME(1981, piranhao, puckman, piranha, mspacman, pacman_state, init_eyes, ROT90, "GL (US Billiards license)", "Piranha (older)", MACHINE_SUPPORTS_SAVE)
GAME(1981, mspacmab3, puckman, piranha, mspacman, pacman_state, init_eyes, ROT90, "bootleg", "Ms. Pac-Man (bootleg, set 3)", MACHINE_SUPPORTS_SAVE)
GAME(1981, abscam, puckman, piranha, mspacman, pacman_state, init_eyes, ROT90, "GL (US Billiards license)", "Abscam", MACHINE_SUPPORTS_SAVE)
GAME(1981, piranhah, puckman, pacman, mspacman, pacman_state, empty_init, ROT90, "hack", "Piranha (hack)", MACHINE_SUPPORTS_SAVE)
GAME(1981, titanpac, puckman, piranha, mspacman, pacman_state, init_eyes, ROT90, "hack (NSM)", "Titan (Pac-Man hack)", MACHINE_SUPPORTS_SAVE)

GAME( 1981, mspacman,   0,        mspacman, mspacman, pacman_state,  init_mspacman,  ROT90,  "Midway / General Computer Corporation", "Ms. Pac-Man",                                      MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmnf,   mspacman, mspacman, mspacman, pacman_state,  init_mspacman,  ROT90,  "hack",                                  "Ms. Pac-Man (speedup hack)",                       MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmat,   mspacman, mspacman, mspacman, pacman_state,  init_mspacman,  ROT90,  "hack",                                  "Ms. Pac Attack",                                   MACHINE_SUPPORTS_SAVE )
GAME( 1989, msheartb,   mspacman, mspacman, mspacman, pacman_state,  init_mspacman,  ROT90,  "hack (Two-Bit Score)",                  "Ms. Pac-Man Heart Burn",                           MACHINE_SUPPORTS_SAVE )
GAME( 1981, pacgal2,    mspacman, mspacman, mspacman, pacman_state,  init_mspacman,  ROT90,  "bootleg",                               "Pac-Gal (set 2)",                                  MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmancr, mspacman, mspacman, mspacman, pacman_state,  init_mspacman,  ROT90,  "bootleg",                               "Ms. Pac-Man (bootleg on Crush Roller Hardware)",   MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmab,   mspacman, woodpek,  mspacman, pacman_state,  empty_init,     ROT90,  "bootleg",                               "Ms. Pac-Man (bootleg, set 1)",                     MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmab2,  mspacman, woodpek,  mspacman, pacman_state,  empty_init,     ROT90,  "bootleg",                               "Ms. Pac-Man (bootleg, set 2)",                     MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmab4,  mspacman, woodpek,  mspacman, pacman_state,  empty_init,     ROT90,  "bootleg",                               "Ms. Pac-Man (bootleg, set 4)",                     MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmbe,   mspacman, woodpek,  mspacman, pacman_state,  init_mspacmbe,  ROT90,  "bootleg",                               "Ms. Pac-Man (bootleg, encrypted)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1982, mspacmbmc,  mspacman, woodpek,  mspacman, pacman_state,  empty_init,     ROT90,  "bootleg (Marti Colls)",                 "Ms. Pac-Man (Marti Colls bootleg)",                MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacmbn,   mspacman, woodpek,  mspacman, pacman_state,  init_pengomc1,  ROT90,  "bootleg (Novatronic)",                  "Ms. Pac-Man (Novatronic bootleg)",                 MACHINE_SUPPORTS_SAVE )
GAME( 1982, mspacmanlai,mspacman, woodpek,  mspacman, pacman_state,  empty_init,     ROT90,  "bootleg (Leisure and Allied)",          "Ms. Pac-Man (Leisure and Allied bootleg)",         MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacii,    mspacman, mspacii,  mspacman, pacman_state,  init_mspacii,   ROT90,  "bootleg (Orca)",                        "Ms. Pac-Man II (Orca bootleg set 1)",              MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacii2,   mspacman, mspacii,  mspacman, pacman_state,  init_mspacii,   ROT90,  "bootleg (Orca)",                        "Ms. Pac-Man II (Orca bootleg set 2)",              MACHINE_SUPPORTS_SAVE )
GAME( 1981, pacgal,     mspacman, woodpek,  mspacman, pacman_state,  empty_init,     ROT90,  "hack",                                  "Pac-Gal (set 1)",                                  MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspacpls,   mspacman, woodpek,  mspacman, pacman_state,  empty_init,     ROT90,  "hack",                                  "Ms. Pac-Man Plus",                                 MACHINE_SUPPORTS_SAVE )
GAME( 1992, mschamp,    mspacman, mschamp,  mschamp,  pacman_state,  init_mschamp,   ROT90,  "hack",                                  "Ms. Pacman Champion Edition / Zola-Puc Gal",       MACHINE_SUPPORTS_SAVE ) // Rayglo version
GAME( 1995, mschamps,   mspacman, mschamp,  mschamp,  pacman_state,  init_mschamp,   ROT90,  "hack",                                  "Ms. Pacman Champion Edition / Super Zola-Puc Gal", MACHINE_SUPPORTS_SAVE )
GAME( 1981, mspackpls,  mspacman, woodpek,  mspacman, pacman_state,  init_mspackpls, ROT90,  "hack",                                  "Miss Packman Plus",                                MACHINE_SUPPORTS_SAVE )

*/