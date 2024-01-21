#include "SDL2/SDL.h"
#include "types.h"
#include "midway8080.h"
#include "mb14241.h"


using namespace aos::midway8080;


namespace aos::midway8080
{
	class SeaWolf : public Midway8080
	{
	protected:
		xprocessors::MB14241::Ptr _shifter;

		uint8_t _periscopePosition;
		bool _fire;

	public:
		SeaWolf();
		virtual ~SeaWolf();
		void init(const emulator::Game&) override;
		void out(const uint8_t, const uint8_t) override;
		uint8_t in(const uint8_t) override;
		void controllerTick() override;
		std::vector<std::pair<uint16_t, string>> romFiles() const override;
		rgb_t color(const uint8_t, const uint8_t, const bool) const override;
	};
}


static ae::midway8080::RegistryHandler reg{ "seawolf", [] { return std::unique_ptr<GameBoard>(new SeaWolf()); } };


SeaWolf::SeaWolf() :
	GameBoard{DisplayOrientation::Horizontal}
{
}

SeaWolf::~SeaWolf()
{
}

void SeaWolf::init(const emulator::Game& settings)
{
	_shifter = xprocessors::MB14241::create();

	_periscopePosition = 1;
}

std::vector<std::pair<uint16_t, std::string>> SeaWolf::romFiles() const
{
	return { {0,"sw0041.h"}, {0,"sw0042.g"}, {0,"sw0043.f"}, {0,"sw0044.e"} };
}

void SeaWolf::out(const uint8_t port, const uint8_t value) {
	switch (port)
	{
	case 4:
		_shifter->writeOffset(value);
		break;
	case 3:
		_shifter->writeValue(value);
		break;
	}
}

ae::rgb_t SeaWolf::color(const uint8_t, const uint8_t, const bool bw) const
{
	return (bw) ? ae::rgb_t{ 255,255,255 } : ae::rgb_t{ 0,0,0 };
}

void periscope(uint8_t* pixels) {
	uint8_t _periscopePosition = 0;
	for (uint8_t y = 0; y < 224; y++)
	{
		pixels[y * 256 + _periscopePosition*256/32 + 10] = 0xffff0000;
	}
}

static const uint8_t periscopeIO[] = {
	0x00, 0x01, 0x03, 0x02, 0x06, 0x07, 0x05, 0x04, 0x0c,
	0x0d, 0x0f, 0x0e, 0x0a, 0x0b, 0x09, 0x08, 0x18, 0x19,
	0x1b, 0x1a, 0x1e, 0x1f, 0x1d, 0x1c, 0x14, 0x15,	0x17,
	0x16, 0x12, 0x13, 0x11, 0x10 };

uint8_t SeaWolf::in(const uint8_t port) {
	switch (port) {
	case 0:
	{
		uint8_t b = _shifter->readValue();
		b = (b & 0xF0) >> 4 | (b & 0x0F) << 4;
		b = (b & 0xCC) >> 2 | (b & 0x33) << 2;
		b = (b & 0xAA) >> 1 | (b & 0x55) << 1;
		return b;
	}
	break;
	case 1:
	{
		uint8_t port = 0b11000000;
		port |= periscopeIO[_periscopePosition];
		if (_fire)
			port |= 0x20;
		return port;
	}
	break;
	case 2:
	{
		uint8_t port = 0b11000000;

const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

		if (Keyboard[SDL_SCANCODE_A])
			port |= 0x01;
		if (Keyboard[SDL_SCANCODE_RETURN])
			port |= 0x02;
		return port;
	}
	break;
	case 3:
		return _shifter->readValue();
		break;
	default:
		return (uint8_t)0;
		break;
	}
}

void SeaWolf::controllerTick()
{
	const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

	if ((Keyboard[SDL_SCANCODE_LEFT]) && (_periscopePosition > 0))
		--_periscopePosition;
	if ((Keyboard[SDL_SCANCODE_RIGHT]) && (_periscopePosition < 31))
		++_periscopePosition;
	_fire = (Keyboard[SDL_SCANCODE_SPACE]) ? true : false;
}
