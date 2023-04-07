#include "amazingmaze.h"

using namespace ae::midway8080;

static ae::midway8080::RegistryHandler reg{ "amazingmaze", [] { return std::unique_ptr<GameBoard>(new AmazingMaze()); } };


AmazingMaze::AmazingMaze() :
	GameBoard{ DisplayOrientation::Horizontal }
{
	_port0.set(0, "_JOY1_LEFT");
	_port0.set(1, "_JOY1_RIGHT");
	_port0.set(2, "_JOY1_DOWN");
	_port0.set(3, "_JOY1_UP");
	_port0.set(4, "_JOY2_LEFT");
	_port0.set(5, "_JOY2_RIGHT");
	_port0.set(6, "_JOY2_DOWN");
	_port0.set(7, "_JOY2_UP");

	_port1.set(4, "coinage");
	_port1.set(6, "gametime");
	_port1.set(3, "_COIN");
	_port1.set(0, "_START1");
	_port1.set(1, "_START2");
}

void AmazingMaze::init(const emulator::Game& settings)
{
	_controller = ae::controller::ArcadeController::create();
	_port0.init(settings);
	_port1.init(settings);
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
		return _port0.get();
	case 1:
		return _port1.get();
	default:
		return result;
	}
}
