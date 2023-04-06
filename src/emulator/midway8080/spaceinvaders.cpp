#include "spaceinvaders.h"
#include "SDL2/SDL.h"

using namespace ae::midway8080;

static ae::midway8080::RegistryHandler reg1("spaceinvaders", [] { return std::unique_ptr<GameBoard>(new SpaceInvaders()); });
static ae::midway8080::RegistryHandler reg2("alieninvaders", [] { return std::unique_ptr<GameBoard>(new AlienInvaders()); });


SpaceInvaders::SpaceInvaders() :
	GameBoard{DisplayOrientation::Vertical},
	_port2{0}
{
	_port1 = 0b10001000;
	_port1.set(0, "_COIN");
	_port1.set(1, "_START2");
	_port1.set(2, "_START1");
	_port1.set(4, "_JOY1_FIRE");
	_port1.set(5, "_JOY1_LEFT");
	_port1.set(6, "_JOY1_RIGHT");

	_port2.set(0, "ships");
	_port2.set(3, "extra");
	_port2.set(4, "_JOY1_FIRE");
	_port2.set(5, "_JOY1_LEFT");
	_port2.set(6, "_JOY1_RIGHT");
	_port2.set(7, "coinInfo");
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
	_port1.init(settings);

	if (_version == "sitv")
		_port2.set(2, "_TILT");
	_port2.init(settings);
}

std::vector<std::pair<uint16_t, std::string>> SpaceInvaders::romFiles() const
{
	if (_version == "midway")
		return { {0,"invaders.h"}, {0,"invaders.g"}, {0,"invaders.f"}, {0,"invaders.e"} };
	if (_version == "sitv")
		return { {0,"sitv/tv0h.s1"}, {0,"sitv/tv02.rp1"}, {0,"sitv/tv03.n1"}, {0,"sitv/tv04.m1"} };
	if (_version == "alieninv")
		return { {0,"alieninv/alieninv.h"}, {0,"alieninv/alieninv.g"}, {0,"alieninv/alieninv.f"}, {0,"alieninv/alieninv.e"} };
	if (_version == "sisv2")
		return { {0,"sv01.36"}, {0,"sv02.35"}, {0,"sv10.34"}, {0x1400,"sv04.31"}, {0,"sv09.42"}, {0,"sv06.41"} };
	if (_version == "sisv3")
		return { {0,"sv0h.36"}, {0,"sv02.35"}, {0,"sv10.34"}, {0x1400,"sv04.31"}, {0,"sv09.42"}, {0,"sv06.41"} };
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
		if ((_version != "sisv2") && (_version != "sisv3")) {
			if ((y >= 32) && (y < 62))
				color = { 255, 32,32 };
			if ((y >= 184) && (y < 240))
				color = { 32, 255, 32 };
			if ((y >= 240) && (x >= 16) && (x < 134))
				color = { 32, 255,32 };
		}
	}
	return color;
}

uint8_t SpaceInvaders::in(const uint8_t port) {
	uint8_t result{ 0 };
	const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

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

AlienInvaders::AlienInvaders() : SpaceInvaders()
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
}

AlienInvaders::~AlienInvaders()
{
}