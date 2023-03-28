#include "spacechaser.h"

using namespace ae::midway8080;

static ae::midway8080::RegistryHandler reg{ "spacechaser", [] { return std::unique_ptr<GameBoard>(new SpaceChaser()); } };


SpaceChaser::SpaceChaser() :
	GameBoard{DisplayOrientation::Vertical}
{
	_colorram = new uint8_t[0x2000];
}

SpaceChaser::~SpaceChaser()
{
	delete[] _colorram;
}

void SpaceChaser::init(const emulator::Game& settings)
{
	_shifter = xprocessors::MB14241::create();
	_controller = ae::controller::ArcadeController::create();

	_port2 |= settings.settings("ships");
	_port2 |= settings.settings("difficulty") << 3;
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
		return 0xff;
		break;
	case 1:
		if (_controller->joystick1(ae::controller::ArcadeController::joystick_control::left))
			result |= 0x20;
		if (_controller->joystick1(ae::controller::ArcadeController::joystick_control::right))
			result |= 0x40;
		if (_controller->joystick1(ae::controller::ArcadeController::joystick_control::down))
			result |= 0x08;
		if (_controller->joystick1(ae::controller::ArcadeController::joystick_control::up))
			result |= 0x80;
		if (_controller->joystick1(ae::controller::ArcadeController::joystick_control::fire))
			result |= 0x10;
		if (_controller->coin())
			result |= 0x01;
		if (_controller->button(ae::controller::ArcadeController::button_control::start2))
			result |= 0x02;
		if (_controller->button(ae::controller::ArcadeController::button_control::start1))
			result |= 0x04;
		return result;
	case 2:
		result = _port2;
		if (_controller->joystick2(ae::controller::ArcadeController::joystick_control::left))
			result |= 0x20;
		if (_controller->joystick2(ae::controller::ArcadeController::joystick_control::right))
			result |= 0x40;
		if (_controller->joystick2(ae::controller::ArcadeController::joystick_control::down))
			result |= 0x02;
		if (_controller->joystick2(ae::controller::ArcadeController::joystick_control::up))
			result |= 0x04;
		if (_controller->joystick2(ae::controller::ArcadeController::joystick_control::fire))
			result |= 0x10;
		return result;
	case 3:
		return _shifter->readValue();
	default:
		return (uint8_t)0;
	}
}
