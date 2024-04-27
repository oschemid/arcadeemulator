#include "midway8080.h"
#include "types.h"
#include "emulator.h"
#include "tools.h"
#include "registry.h"
#include "mb14241.h"
#include "../../controller/arcadecontroller.h"
#include "io.h"


using namespace aos::midway8080;


namespace aos::midway8080
{
	class SpaceInvaders : public Midway8080
	{
	public:
		SpaceInvaders(const vector<aos::mmu::RomMapping>&,
			const emulator::GameConfiguration&,
			std::function<rgb_t(const uint8_t, const uint8_t)> = nullptr);
		virtual ~SpaceInvaders();

		void init(aos::display::RasterDisplay*, aos::controller::Controller*) override;

	protected:
		xprocessors::MB14241::Ptr _shifter{ nullptr };
		std::function<aos::rgb_t(const uint8_t, const uint8_t)> _colorfn;

		uint8_t in(const uint8_t);
		void out(const uint8_t, const uint8_t);
		void updateDisplay();
	};

	class AlienInvaders : public SpaceInvaders
	{
	public:
		AlienInvaders(const vector<aos::mmu::RomMapping>&, const aos::emulator::GameConfiguration&);
		virtual ~AlienInvaders( );
	};
}


SpaceInvaders::SpaceInvaders(const vector<aos::mmu::RomMapping>& roms, const aos::emulator::GameConfiguration& config,
	std::function<aos::rgb_t(const uint8_t, const uint8_t)> colorfn) :
	aos::midway8080::Midway8080{ roms },
	_colorfn{ colorfn }
{
	_port1 = 0b10001000;
	_port1.set(0, "_COIN");
	_port1.set(1, "_START2");
	_port1.set(2, "_START1");
	_port1.set(4, "_JOY1_FIRE");
	_port1.set(5, "_JOY1_LEFT");
	_port1.set(6, "_JOY1_RIGHT");
	_port1.init(config);

	_port2.set(0, "ships");
	_port2.set(3, "extra");
	_port2.set(4, "_JOY1_FIRE");
	_port2.set(5, "_JOY1_LEFT");
	_port2.set(6, "_JOY1_RIGHT");
	_port2.set(7, "coinInfo");
	_port2.init(config);

	_port0 = 1;
}

void SpaceInvaders::init(aos::display::RasterDisplay* raster, aos::controller::Controller*)
{
	Midway8080::init(raster, nullptr);

	_shifter = xprocessors::MB14241::create();
	_controller = ae::controller::ArcadeController::create();

	_mmu.bank("cpu", 0x2000).rom(); // 8k ROM
	_mmu.bank("ram", 0x400).ram(); // 1k RAM
	_mmu.bank("vram", 0x1c00).ram(); // 7k Video RAM

	_mmu.map(0x0000, 0x1fff, "cpu").mirror(0x3fff);
	_mmu.map(0x2000, 0x23ff, "ram").mirror(0x3fff);
	_mmu.map(0x2400, 0x3fff, "vram").mirror(0x3fff);

	_mmu.init(_roms);
}

uint8_t SpaceInvaders::in(const uint8_t port) {
	uint8_t result{ 0 };

	switch (port) {
	case 0:
		return _port0.get();
	case 1:
		return _port1.get();
	case 2:
		return _port2.get();
	case 3:
		return _shifter->readValue();
	default:
		return result;
	}
}

SpaceInvaders::~SpaceInvaders()
{
}

void SpaceInvaders::out(const uint8_t port, const uint8_t value) {
	switch (port)
	{
	case 2:
		_shifter->writeOffset(value);
		break;
	case 4:
		_shifter->writeValue(value);
		break;
	}
}

void SpaceInvaders::updateDisplay()
{
	uint8_t* _memory = _mmu.getvram();
	for (int x = 0; x < 224; x++) {
		for (int y = 0; y < 256; y += 8) {
			uint8_t VRAMByte = _memory[(x << 5) + (y >> 3)];

			for (int bit = 0; bit < 8; bit++) {
				uint8_t CoordX;
				uint8_t CoordY;
				CoordX = x;
				CoordY = (256 - 1 - (y + bit));

				aos::rgb_t color = { 0,0,0 };
				if ((VRAMByte >> bit) & 1) {
					color = (_colorfn) ? _colorfn(CoordX, CoordY) : aos::rgb_t{ 255, 255, 255 };
				}
				_raster->set(CoordX, CoordY, color);
			}
		}
	}
}

aos::rgb_t spaceinvader_gels(const uint8_t x, const uint8_t y, const bool b)
{
	if (!b)
		return { 0,0,0 };
	if ((y >= 32) && (y < 62))
		return { 255, 32,32 };
	if ((y >= 184) && (y < 240))
		return { 32, 255, 32 };
	if ((y >= 240) && (x >= 16) && (x < 134))
		return  { 32, 255,32 };
	return { 255, 255, 255 };
}

static aos::RegistryHandler<aos::emulator::GameDriver> spaceinvaders("spaceinvaders", {
	.name = "Space Invaders",
	.version = "Midway",
	.main_version = true,
	.emulator = "midway8080",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) {
		auto emul = std::make_unique<aos::midway8080::SpaceInvaders>(roms, config);
		emul->colorfn(spaceinvader_gels);
		return emul;
	},
	.roms = {
			{ "cpu", 0x800, 0x734f5ad8 },
			{ "cpu", 0x800, 0x6bfaca4a },
			{ "cpu", 0x800, 0x0ccead96 },
			{ "cpu", 0x800, 0x14e538b0 }
			},
	.configuration = {
		.switches = {{ "coinage", 1, "Coinage", {"Free", "1C/1C", "1C/2C", "2C/1C"} },
				{ "coinInfo", 1, "Display Coin", {"False", "True"} },
				{ "ships", 0, "Ships", {"3", "4", "5", "6"} },
				{ "extra", 0, "Extra Ship", {"1500", "1000"} }
		}
	}
});

static aos::RegistryHandler<aos::emulator::GameDriver> spaceinvaderstv("spaceinvaderstv", {
	.name = "Space Invaders",
	.version = "TV ver 2",
	.emulator = "midway8080",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) { return std::make_unique<aos::midway8080::SpaceInvaders>(roms, config /*, spaceinvader_gels*/); },
	.roms = {
			{ "cpu", 0x800, 0xfef18aad },
			{ "cpu", 0x800, 0x3c759a90 },
			{ "cpu", 0x800, 0x0ad3657f },
			{ "cpu", 0x800, 0xcd2c67f6 }
			},
	.configuration = {
		.switches = {{ "coinage", 1, "Coinage", {"Free", "1C/1C", "1C/2C", "2C/1C"} },
				{ "coinInfo", 1, "Display Coin", {"False", "True"} },
				{ "ships", 0, "Ships", {"3", "4", "5", "6"} },
				{ "extra", 0, "Extra Ship", {"1500", "1000"} }
		}
	}
});
static aos::RegistryHandler<aos::emulator::GameDriver> spaceinvaderssv2("spaceinvaderssv2", {
	.name = "Space Invaders",
	.version = "SV ver 2",
	.emulator = "midway8080",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) { return std::make_unique<aos::midway8080::SpaceInvaders>(roms, config); },
	.roms = {
			{ "cpu", 0x400,0xd0c32d72 },
			{ "cpu", 0x400,0x0e159534 },
			{ "cpu", 0x400,0x483e651e },
			{ "cpu", /*0x1400,*/0x400,0x1293b826 },
			{ "cpu", 0x400,0xcd80b13f },
			{ "cpu", 0x400,0x2c68e0b4 }
			},
	.configuration = {
		.switches = {{ "coinage", 1, "Coinage", {"Free", "1C/1C", "1C/2C", "2C/1C"} },
				{ "coinInfo", 1, "Display Coin", {"False", "True"} },
				{ "ships", 0, "Ships", {"3", "4", "5", "6"} },
				{ "extra", 0, "Extra Ship", {"1500", "1000"} }
		}
	}
});


AlienInvaders::AlienInvaders(const vector<aos::mmu::RomMapping>& roms, const aos::emulator::GameConfiguration& config) :
	SpaceInvaders{ roms, config }
{
	_port0 = 1;

	_port2.reset();
	_port2.set(0, "ships");
	_port2.set(1, "penceCoinage");
	_port2.set(2, "_COIN2");
	_port2.set(4, "_JOY1_FIRE");
	_port2.set(5, "_JOY1_LEFT");
	_port2.set(6, "_JOY1_RIGHT");
	_port2.set(7, "coinage");
	_port2.init(config);
}

AlienInvaders::~AlienInvaders()
{
}

static aos::RegistryHandler<aos::emulator::GameDriver> alieninvaders("alieninvaders", {
	.name = "Alien Invaders",
	.emulator = "midway8080",
	.creator = [](const aos::emulator::GameConfiguration& config, const aos::mmu::RomMappings& roms) { return std::make_unique<aos::midway8080::AlienInvaders>(roms, config); },
	.roms = {
			{ "cpu", 0x800, 0x6ad601c3 },
	        { "cpu", 0x800, 0xc6bb6fb3 },
	        { "cpu", 0x800, 0x1d2ff324 },
	        { "cpu", 0x800, 0x2f2e6791 }
			},
	.configuration = {
		.switches = {{ "penceCoinage", 1, "Pence ?", {"False", "True"} },
					 { "ships", 0, "Ships", {"3", "4"} },
					 { "coinage", 0, "Coinage", {"2C/1C 50p/3C", "1C/1C 50p/5C"} }
		}
	}
});
