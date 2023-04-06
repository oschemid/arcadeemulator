#include "spacechaser.h"

using namespace ae::midway8080;

static ae::midway8080::RegistryHandler reg{ "spacechaser", [] { return std::unique_ptr<GameBoard>(new SpaceChaser()); } };


SpaceChaser::SpaceChaser() :
	GameBoard{DisplayOrientation::Vertical}
{
	_colorram = new uint8_t[0x2000];
	_port0 = 0xff;
	_port1.set(0, "_COIN");
	_port1.set(1, "_START2");
	_port1.set(2, "_START1");
	_port1.set(3, "_JOY1_DOWN");
	_port1.set(4, "_JOY1_FIRE");
	_port1.set(5, "_JOY1_LEFT");
	_port1.set(6, "_JOY1_RIGHT");
	_port1.set(7, "_JOY1_UP");

	_port2.set(0, "ships");
	_port2.set(1, "_JOY2_DOWN");
	_port2.set(2, "_JOY2_UP");
	_port2.set(3, "difficulty");
	_port2.set(4, "_JOY2_FIRE");
	_port2.set(5, "_JOY2_LEFT");
	_port2.set(6, "_JOY2_RIGHT");
}

SpaceChaser::~SpaceChaser()
{
	delete[] _colorram;
}

void SpaceChaser::init(const emulator::Game& settings)
{
	_shifter = xprocessors::MB14241::create();
	_controller = ae::controller::ArcadeController::create();

	_port1.init(settings);
	_port2.init(settings);
}

std::vector<std::pair<uint16_t, std::string>> SpaceChaser::romFiles() const
{
	return { {0,"1"}, {0,"2"}, {0,"3"}, {0,"4"}, {0,"5"}, {0,"6"}, {0,"7"}, {0,"8"}, {0x4000,"9"}, {0,"10"} };
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

ae::rgb_t SpaceChaser::color(const uint8_t x, const uint8_t y, const bool bw) const
{
	ae::rgb_t color = { 0,0,255 };
	if (bw) {
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
	return color;
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
