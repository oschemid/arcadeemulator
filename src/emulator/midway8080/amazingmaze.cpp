#include "amazingmaze.h"

using namespace ae::midway8080;

static ae::midway8080::RegistryHandler reg{ "amazingmaze", [] { return std::unique_ptr<GameBoard>(new AmazingMaze()); } };


AmazingMaze::AmazingMaze() :
	GameBoard{ DisplayOrientation::Horizontal }
{
}

void AmazingMaze::init(const emulator::Game& settings)
{
	_controller = ae::controller::ArcadeController::create();

	_port1 |= settings.settings("coinage") << 4;
	_port1 |= settings.settings("gametime") << 6;
}

std::vector<std::pair<uint16_t, std::string>> AmazingMaze::romFiles() const
{
	return { {0,"maze.h"}, {0,"maze.g"} };
}

void AmazingMaze::out(const uint8_t port, const uint8_t value) 
{
}

ae::rgb_t AmazingMaze::color(const uint8_t, const uint8_t, const bool bw) const
{
	return (bw) ? ae::rgb_t{ 51, 255, 127 } : ae::rgb_t{0, 0, 0};
}

uint8_t AmazingMaze::in(const uint8_t port) {
	uint8_t result{ 0 };
	switch (port) {
	case 0:
		result = 0;
		if (_controller->joystick1(ae::controller::ArcadeController::joystick_control::left))
			result |= 0x01;
		if (_controller->joystick1(ae::controller::ArcadeController::joystick_control::right))
			result |= 0x02;
		if (_controller->joystick1(ae::controller::ArcadeController::joystick_control::down))
			result |= 0x04;
		if (_controller->joystick1(ae::controller::ArcadeController::joystick_control::up))
			result |= 0x08;
		if (_controller->joystick2(ae::controller::ArcadeController::joystick_control::left))
			result |= 0x10;
		if (_controller->joystick2(ae::controller::ArcadeController::joystick_control::right))
			result |= 0x20;
		if (_controller->joystick2(ae::controller::ArcadeController::joystick_control::down))
			result |= 0x40;
		if (_controller->joystick2(ae::controller::ArcadeController::joystick_control::up))
			result |= 0x80;
		return result;
	case 1:
		result = _port1;
		if (_controller->coin())
			result |= 0x08;
		if (_controller->button(ae::controller::ArcadeController::button_control::start2))
			result |= 0x02;
		if (_controller->button(ae::controller::ArcadeController::button_control::start1))
			result |= 0x01;
		return result;
	default:
		return result;
	}
}
