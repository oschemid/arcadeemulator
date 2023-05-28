#include "spacechaser.h"
#include "registry.h"
#include "file.h"

using namespace aos::midway8080;


SpaceChaser::SpaceChaser(vector<pair<uint16_t, string>> roms, const aos::emulator::GameConfiguration& config) :
	aos::midway8080::Midway8080{ },
	_roms{ roms }
{
	_port0 = 0xff;
	_port1.set(0, "_COIN");
	_port1.set(1, "_START2");
	_port1.set(2, "_START1");
	_port1.set(3, "_JOY1_DOWN");
	_port1.set(4, "_JOY1_FIRE");
	_port1.set(5, "_JOY1_LEFT");
	_port1.set(6, "_JOY1_RIGHT");
	_port1.set(7, "_JOY1_UP");
	_port1.init(config);

	_port2.set(0, "ships");
	_port2.set(1, "_JOY2_DOWN");
	_port2.set(2, "_JOY2_UP");
	_port2.set(3, "difficulty");
	_port2.set(4, "_JOY2_FIRE");
	_port2.set(5, "_JOY2_LEFT");
	_port2.set(6, "_JOY2_RIGHT");
	_port2.init(config);
}

void SpaceChaser::init(ae::display::RasterDisplay* raster)
{
	Midway8080::init(raster);


	_shifter = xprocessors::MB14241::create();
	_controller = ae::controller::ArcadeController::create();

	_memory = new uint8_t[0x8000]{ 0 };
	_colorram = new uint8_t[0x2000];

	string path = "roms/midway8080/schaser.zip";
	ae::filemanager::readRoms(path, _roms, _memory);

	_cpu->read([this](const uint16_t p) { if (p >= 0xc000) return readColorRam(p-0xc400); else return _memory[p & 0x7fff]; });
	_cpu->write([this](const uint16_t p, const uint8_t v) { if (p >= 0xc000) writeColorRam(p-0xc400, v); else if ((p & 0x3fff) > 0x1fff) _memory[p & 0x3fff] = v; });
}

uint8_t SpaceChaser::readColorRam(const uint16_t p) const
{
	return _colorram[(p & 0x1f) | ((p & 0x1f80) >> 2)];
}

void SpaceChaser::writeColorRam(const uint16_t p, const uint8_t v)
{
	_colorram[(p & 0x1f) | ((p & 0x1f80) >> 2)] = v;
}
uint8_t SpaceChaser::tick()
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

void SpaceChaser::updateDisplay()
{
	for (int x = 0; x < 224; x++) {
		for (int y = 0; y < 256; y += 8) {
			uint8_t VRAMByte = _memory[0x2400 + (x << 5) + (y >> 3)];

			for (int bit = 0; bit < 8; bit++) {
				uint8_t CoordX;
				uint8_t CoordY;
				CoordX = x;
				CoordY = (256 - 1 - (y + bit));

				aos::rgb_t color = { 0,0,255 };
				if ((VRAMByte >> bit) & 1) {
					uint16_t offset = (x << 5) + (y >> 3);
					uint8_t colorid = _colorram[((offset & 0x1f) | ((offset & 0x1f80) >> 2))] & 0x07;
					switch (colorid) {
					case 0:
						color = { 128, 128, 255 };
						break;
					case 1:
						color = { 255, 0, 0 };
						break;
					case 2:
						color = { 0,0,255 };
						break;
					case 3:
						color = { 255,0,255 };
						break;
					case 4:
						color = { 0,255,0 };
						break;
					case 5:
						color = { 255, 255, 0 };
						break;
					case 6:
						color = { 0,255,255 };
						break;
					case 7:
						color = { 255, 255, 255 };
						break;
					default:
						break;
					}
				}
				_raster->set(CoordX, CoordY, color);
			}
		}
	}
}


SpaceChaser::~SpaceChaser()
{
	delete[] _colorram;
}

void SpaceChaser::out(const uint8_t port, const uint8_t value) {
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

uint8_t SpaceChaser::in(const uint8_t port) {
	uint8_t result = 0;
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
		return (uint8_t)0;
	}
}

static ae::RegistryHandler<aos::emulator::GameDriver> spacechaser("spacechaser", {
	.name = "Space Chaser",
	.emulator = "midway8080",
	.creator = [](const aos::emulator::GameConfiguration& config) { return std::make_unique<aos::midway8080::SpaceChaser>(vector<pair<uint16_t,string>>({
					{ 0,"schasercv/1" },
			{ 0,"schasercv/2" },
			{ 0,"schasercv/3" },
			{ 0,"schasercv/4" },
			{ 0,"schasercv/5" },
			{ 0,"schasercv/6" },
			{ 0,"schasercv/7" },
			{ 0,"schasercv/8" },
			{ 0x4000,"schasercv/9" },
			{ 0,"schasercv/10" }
		}), config); },
	.roms = {
			{ 0,"schasercv/1" },
			{ 0,"schasercv/2" },
			{ 0,"schasercv/3" },
			{ 0,"schasercv/4" },
			{ 0,"schasercv/5" },
			{ 0,"schasercv/6" },
			{ 0,"schasercv/7" },
			{ 0,"schasercv/8" },
			{ 0x4000,"schasercv/9" },
			{ 0,"schasercv/10" }
			},
	.configuration = {
		.switches = {{ "difficulty", 0, "Difficulty", {"Normal", "Hard"} },
					 { "ships", 0, "Ships", {"3", "4"} }
		  }
	}
});
