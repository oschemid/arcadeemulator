#include "spaceinvaders.h"
#include "tools.h"
#include "registry.h"

using namespace aos::midway8080;


SpaceInvaders::SpaceInvaders(const vector<aos::emulator::RomConfiguration>& roms, const aos::emulator::GameConfiguration& config,
	std::function<aos::rgb_t(const uint8_t, const uint8_t)> colorfn) :
	aos::midway8080::Midway8080{ },
	_port1{ 0b10001000 },
	_port2{ 0 },
	_roms{ roms },
	_colorfn{ colorfn }
{
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

void SpaceInvaders::init(ae::display::RasterDisplay* raster)
{
	Midway8080::init(raster);

	_shifter = xprocessors::MB14241::create();
	_controller = ae::controller::ArcadeController::create();

	_memory = new uint8_t[0x4000]{ 0 };

	size_t offset = 0;
	for (const auto& rom : _roms) {
		if (rom.start>0)
			offset = rom.start;
		offset += rom.rom.read(_memory + offset);
	}

	_cpu->read([this](const uint16_t p) { return _memory[p & 0x3fff]; });
	_cpu->write([this](const uint16_t p, const uint8_t v) { if ((p & 0x3fff) > 0x1fff) _memory[p & 0x3fff] = v; });
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

//void SpaceInvaders::init(ae::display::RasterDisplay* raster)
//{
//	_shifter = xprocessors::MB14241::create();
//	_controller = ae::controller::ArcadeController::create();
//
//	//_version = settings.version();
//	_port0 = 0;
//	//_port0 = (_version == "sitv") ? 1 : 0;
//	_port1.init(settings);
//
//	//if (_version == "sitv")
//	//	_port2.set(2, "_TILT");
//	_port2.init(settings);
//}

//std::vector<std::pair<uint16_t, std::string>> SpaceInvaders::romFiles() const
//{
//	if (_version == "sisv2")
//		return { {0,"sv01.36"}, {0,"sv02.35"}, {0,"sv10.34"}, {0x1400,"sv04.31"}, {0,"sv09.42"}, {0,"sv06.41"} };
//	if (_version == "sisv3")
//		return { {0,"sv0h.36"}, {0,"sv02.35"}, {0,"sv10.34"}, {0x1400,"sv04.31"}, {0,"sv09.42"}, {0,"sv06.41"} };
//	return std::vector<std::pair<uint16_t, std::string>>();
//}

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

uint8_t SpaceInvaders::tick()
{
	static uint64_t deltaDisplay{ 0 };
	static uint64_t deltaController{ 0 };
	static uint8_t draw{ 0 };
	uint64_t previous = _cpu->elapsed_cycles();
	_cpu->executeOne();
	uint64_t deltaClock = _cpu->elapsed_cycles() - previous;

	if (deltaDisplay > 16641) { // 120 Hz
		if (draw) {
			updateDisplay();
			_raster->refresh();
			_cpu->interrupt(2);
		}
		else
			_cpu->interrupt(1);
		draw = 1 - draw;
		deltaDisplay -= 16641;
	}
	deltaDisplay += deltaClock;

	if (deltaController > 66566) { // 30 Hz
		_controller->tick(); _port0.tick(*_controller); _port1.tick(*_controller); _port2.tick(*_controller);
		deltaController -= 66566;
	}
	deltaController += deltaClock;
	return static_cast<uint8_t>(deltaClock);
}

void SpaceInvaders::updateDisplay()
{
	for (int x = 0; x < 224; x++) {
		for (int y = 0; y < 256; y += 8) {
			uint8_t VRAMByte = _memory[0x2400 + (x << 5) + (y >> 3)];

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

aos::rgb_t spaceinvader_gels(const uint8_t x, const uint8_t y)
{
	if ((y >= 32) && (y < 62))
		return { 255, 32,32 };
	if ((y >= 184) && (y < 240))
		return { 32, 255, 32 };
	if ((y >= 240) && (x >= 16) && (x < 134))
		return  { 32, 255,32 };
	return { 255, 255, 255 };
}

static ae::RegistryHandler<aos::emulator::GameDriver> spaceinvaders("spaceinvaders", {
	.name = "Space Invaders",
	.version = "Midway",
	.main_version = true,
	.emulator = "midway8080",
	.creator = [](const aos::emulator::GameConfiguration& config, const vector<aos::emulator::RomConfiguration>& roms) { return std::make_unique<aos::midway8080::SpaceInvaders>(roms, config, spaceinvader_gels); },
	.roms = {
			{ 0, 0x800, 0x734f5ad8 },
			{ 0, 0x800, 0x6bfaca4a },
			{ 0, 0x800, 0x0ccead96 },
			{ 0, 0x800, 0x14e538b0 }
			},
	.configuration = {
		.switches = {{ "coinage", 1, "Coinage", {"Free", "1C/1C", "1C/2C", "2C/1C"} },
				{ "coinInfo", 1, "Display Coin", {"False", "True"} },
				{ "ships", 0, "Ships", {"3", "4", "5", "6"} },
				{ "extra", 0, "Extra Ship", {"1500", "1000"} }
		}
	}
});

static ae::RegistryHandler<aos::emulator::GameDriver> spaceinvaderstv("spaceinvaderstv", {
	.name = "Space Invaders",
	.version = "TV ver 2",
	.emulator = "midway8080",
	.creator = [](const aos::emulator::GameConfiguration& config, const vector<aos::emulator::RomConfiguration>& roms) { return std::make_unique<aos::midway8080::SpaceInvaders>(roms, config, spaceinvader_gels); },
	.roms = {
			{ 0, 0x800, 0xfef18aad },
			{ 0, 0x800, 0x3c759a90 },
			{ 0, 0x800, 0x0ad3657f },
			{ 0, 0x800, 0xcd2c67f6 }
			},
	.configuration = {
		.switches = {{ "coinage", 1, "Coinage", {"Free", "1C/1C", "1C/2C", "2C/1C"} },
				{ "coinInfo", 1, "Display Coin", {"False", "True"} },
				{ "ships", 0, "Ships", {"3", "4", "5", "6"} },
				{ "extra", 0, "Extra Ship", {"1500", "1000"} }
		}
	}
});
static ae::RegistryHandler<aos::emulator::GameDriver> spaceinvaderssv2("spaceinvaderssv2", {
	.name = "Space Invaders",
	.version = "SV ver 2",
	.emulator = "midway8080",
	.creator = [](const aos::emulator::GameConfiguration& config, const vector<aos::emulator::RomConfiguration>& roms) { return std::make_unique<aos::midway8080::SpaceInvaders>(roms, config); },
	.roms = {
			{ 0,0x400,0xd0c32d72 },
			{ 0,0x400,0x0e159534 },
			{ 0,0x400,0x483e651e },
			{ 0x1400,0x400,0x1293b826 },
			{ 0,0x400,0xcd80b13f },
			{ 0,0x400,0x2c68e0b4 }
			},
	.configuration = {
		.switches = {{ "coinage", 1, "Coinage", {"Free", "1C/1C", "1C/2C", "2C/1C"} },
				{ "coinInfo", 1, "Display Coin", {"False", "True"} },
				{ "ships", 0, "Ships", {"3", "4", "5", "6"} },
				{ "extra", 0, "Extra Ship", {"1500", "1000"} }
		}
	}
});


AlienInvaders::AlienInvaders(const vector<aos::emulator::RomConfiguration>& roms, const aos::emulator::GameConfiguration& config) :
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

static ae::RegistryHandler<aos::emulator::GameDriver> alieninvaders("alieninvaders", {
	.name = "Alien Invaders",
	.emulator = "midway8080",
	.creator = [](const aos::emulator::GameConfiguration& config, const vector<aos::emulator::RomConfiguration>& roms) { return std::make_unique<aos::midway8080::AlienInvaders>(roms, config); },
	.roms = {
			{ 0, 0x800, 0x6ad601c3 },
	        { 0,0x800, 0xc6bb6fb3 },
	        { 0,0x800, 0x1d2ff324 },
	        { 0,0x800, 0x2f2e6791 }
			},
	.configuration = {
		.switches = {{ "penceCoinage", 1, "Pence ?", {"False", "True"} },
					 { "ships", 0, "Ships", {"3", "4"} },
					 { "coinage", 0, "Coinage", {"2C/1C 50p/3C", "1C/1C 50p/5C"} }
		}
	}
});
