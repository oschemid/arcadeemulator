#include "spaceinvaders.h"

using namespace ae::midway8080;

static ae::midway8080::RegistryHandler reg("spaceinvaders", [] { return std::unique_ptr<GameBoard>(new SpaceInvaders()); });


SpaceInvaders::SpaceInvaders() :
	GameBoard{DisplayOrientation::Vertical},
	_port0{0},
	_port2{0}
{
}

SpaceInvaders::~SpaceInvaders()
{
}

void SpaceInvaders::init(const emulator::Game& settings)
{
	_shifter = xprocessors::MB14241::create();
	_controller = ae::controller::ArcadeController::create();

	_version = settings.version();
	_port0 = (_version == "sitv") ? 1 : 0;
	_port2 |= settings.settings("ships");
	_port2 |= settings.settings("extra") << 3;
	_port2 |= settings.settings("coinInfo") << 7;
}

std::vector<std::pair<uint16_t, std::string>> SpaceInvaders::romFiles() const
{
	if (_version == "midway")
		return { {0,"invaders.h"}, {0,"invaders.g"}, {0,"invaders.f"}, {0,"invaders.e"} };
	if (_version == "sitv")
		return { {0,"tv0h.s1"}, {0,"tv02.rp1"}, {0,"tv03.n1"}, {0,"tv04.m1"} };
	if (_version == "sisv2")
		return { {0,"sv01.36"}, {0,"sv02.35"}, {0,"sv10.34"}, {0x1400,"sv04.31"}, {0,"sv09.42"}, {0,"sv06.41"} };
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

ae::rgb_t SpaceInvaders::color(const uint8_t x, const uint8_t y, const bool bw) const
{
	ae::rgb_t color = { 0,0,0 };
	if (bw) {
		color = { 255,255,255 };
		if ((y >= 32) && (y < 62))
			color = { 255, 32,32 };
		if ((y >= 184) && (y < 240))
			color = { 32, 255, 32 };
		if ((y >= 240) && (x >= 16) && (x < 134))
			color = { 32, 255,32 };
	}
	return color;
}

uint8_t SpaceInvaders::in(const uint8_t port) {
	uint8_t result{ 0 };
	const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

	switch (port) {
	case 0:
		return _port0;
	case 1:
		result = 0b10001000;
		if (_controller->coin())
			result |= 0x01;
		if (_controller->button(ae::controller::ArcadeController::button_control::start2))
			result |= 0x02;
		if (_controller->button(ae::controller::ArcadeController::button_control::start1))
			result |= 0x04;
		if (_controller->joystick1(ae::controller::ArcadeController::joystick_control::fire))
			result |= 0x10;
		if (_controller->joystick1(ae::controller::ArcadeController::joystick_control::left))
			result |= 0x20;
		if (_controller->joystick1(ae::controller::ArcadeController::joystick_control::right))
			result |= 0x40;
		return result;
	case 2:
		result = _port2;

		if ((_version=="sitv")&&(Keyboard[SDL_SCANCODE_T]))
			result |= 0x04;
		if (_controller->joystick1(ae::controller::ArcadeController::joystick_control::fire))
			result |= 0x10;
		if (_controller->joystick1(ae::controller::ArcadeController::joystick_control::left))
			result |= 0x20;
		if (_controller->joystick1(ae::controller::ArcadeController::joystick_control::right))
			result |= 0x40;
		return result;
	case 3:
		return _shifter->readValue();
	default:
		return result;
	}
}
