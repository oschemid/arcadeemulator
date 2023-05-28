#include "amazingmaze.h"
#include "registry.h"
#include "file.h"


using namespace aos::midway8080;


AmazingMaze::AmazingMaze(vector<pair<uint16_t, string>> roms, const aos::emulator::GameConfiguration& config) :
	aos::midway8080::Midway8080{ },
	_roms{ roms }
{
	_port0.set(0, "_JOY1_LEFT");
	_port0.set(1, "_JOY1_RIGHT");
	_port0.set(2, "_JOY1_DOWN");
	_port0.set(3, "_JOY1_UP");
	_port0.set(4, "_JOY2_LEFT");
	_port0.set(5, "_JOY2_RIGHT");
	_port0.set(6, "_JOY2_DOWN");
	_port0.set(7, "_JOY2_UP");
	_port0.init(config);

	_port1.set(4, "coinage");
	_port1.set(6, "gametime");
	_port1.set(3, "_COIN");
	_port1.set(0, "_START1");
	_port1.set(1, "_START2");
	_port1.set(7, "service");
	_port1.init(config);
}

void AmazingMaze::init(ae::display::RasterDisplay* raster)
{
	Midway8080::init(raster);

	_controller = ae::controller::ArcadeController::create();

	_memory = new uint8_t[0x4000]{ 0 };

	string path = "roms/midway8080/maze.zip";
	ae::filemanager::readRoms(path, _roms, _memory);

	_cpu->read([this](const uint16_t p) { return _memory[p & 0x3fff]; });
	_cpu->write([this](const uint16_t p, const uint8_t v) { if ((p & 0x3fff) > 0x1fff) _memory[p & 0x3fff] = v; });
}

AmazingMaze::~AmazingMaze()
{
}

void AmazingMaze::out(const uint8_t port, const uint8_t value) 
{
}

uint8_t AmazingMaze::in(const uint8_t port) {
	uint8_t result{ 0 };
	switch (port) {
	case 0:
		return _port0.get();
	case 1:
		return _port1.get();
	default:
		return result;
	}
}

uint8_t AmazingMaze::tick()
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
		_controller->tick(); _port0.tick(*_controller); _port1.tick(*_controller);
		deltaController -= 66566;
	}
	deltaController += deltaClock;
	return static_cast<uint8_t>(deltaClock);
}

void AmazingMaze::updateDisplay()
{
	for (int y = 0; y < 224; y++) {
		for (int x = 0; x < 256; x += 8) {
			uint8_t VRAMByte = _memory[0x2400 + (y << 5) + (x >> 3)];

			for (int bit = 0; bit < 8; bit++) {
				uint8_t CoordX;
				uint8_t CoordY;
				CoordX = x + bit;
				CoordY = y;

				aos::rgb_t color = { 0,0,0 };
				if ((VRAMByte >> bit) & 1) {
					color = aos::rgb_t{ 51, 255, 127 };
				}
				_raster->set(CoordX, CoordY, color);
			}
		}
	}
}

static ae::RegistryHandler<aos::emulator::GameDriver> amazingmaze("amazingmaze", {
	.name = "Amazing Maze",
	.emulator = "midway8080",
	.creator = [](const aos::emulator::GameConfiguration& config) { return std::make_unique<aos::midway8080::AmazingMaze>(vector<pair<uint16_t,string>>({
			{ 0,"maze.h" },
			{ 0,"maze.g" }
		}), config); },
	.roms = {
			{ 0,"maze.h" },
			{ 0,"maze.g" }
			},
	.configuration = {
		.switches = {{ "coinage", 0, "Coin", {"One coin One credit", "Two coins One credit", "One coin Two credits", "Two coins Two redits"} },
					 { "gametime", 0, "Two player time", {"6 mins", "4 mins"} },
					 { "service", 0, "Service", {"Off", "On"} }
					  }
	}
});
