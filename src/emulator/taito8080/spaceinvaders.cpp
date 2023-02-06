#include "spaceinvaders.h"

using namespace ae::taito8080;

static ae::taito8080::RegistryHandler reg{ "spaceinvaders", [] { return std::unique_ptr<Cartridge>(new SpaceInvaders()); } };


SpaceInvaders::SpaceInvaders() :
	Cartridge{},
	_shift1{0},
	_shift0{ 0 },
	_shift_offset{0},
	_ships{0},
	_extraShip{false},
	_coinInfo{false}
{
	_memory = new uint8_t[0x4000];
}

SpaceInvaders::~SpaceInvaders()
{
}

void SpaceInvaders::init(const json& settings)
{
	json gamesettings = settings.at("settings");
	gamesettings.at("version").get_to(_version);
	gamesettings.at("extraShip").get_to(_extraShip);
	gamesettings.at("coinInfo").get_to(_coinInfo);
	gamesettings.at("ships").get_to(_ships);

	string path;
	settings.at("roms").at("path").get_to(path);

	loadRom(path);
}

void SpaceInvaders::loadRom(const string& path)
{
	if (_version == "midway") {
		File f1(path + "/invaders/invaders.h");
		f1.read(0, 0x0800, _memory);
		File f2(path + "/invaders/invaders.g");
		f2.read(0, 0x0800, _memory + 0x0800);
		File f3(path + "/invaders/invaders.f");
		f3.read(0, 0x0800, _memory + 0x1000);
		File f4(path + "/invaders/invaders.e");
		f4.read(0, 0x0800, _memory + 0x1800);
	}
	if (_version == "sitv") {
		File f1(path + "/sitv/tv0h.s1");
		f1.read(0, 0x0800, _memory);
		File f2(path + "/sitv/tv02.rp1");
		f2.read(0, 0x0800, _memory + 0x0800);
		File f3(path + "/sitv/tv03.n1");
		f3.read(0, 0x0800, _memory + 0x1000);
		File f4(path + "/sitv/tv04.m1");
		f4.read(0, 0x0800, _memory + 0x1800);
	}
}
uint8_t SpaceInvaders::read(const uint16_t address)
{
	return (address < 0x4000) ? _memory[address] : 0;
}

void SpaceInvaders::write(const uint16_t address, const uint8_t value)
{
	if ((address > 0x1fff) && (address < 0x4000))
		_memory[address] = value;
}

void SpaceInvaders::out(const uint8_t port, const uint8_t value) {
	switch (port)
	{
	case 2:
		_shift_offset = value & 0x7;
		break;
	case 4:
		_shift0 = _shift1;
		_shift1 = value;
		break;
	}
}

void SpaceInvaders::updateDisplay(uint32_t* pixels)
{
	uint32_t ColorToDraw = 0xffffffff;

	for (int x = 0; x < 224; x++) {
		for (int y = 0; y < 256; y += 8) {
			uint8_t VRAMByte = read(0x2400 + (x << 5) + (y >> 3));

			for (int bit = 0; bit < 8; bit++) {
				ColorToDraw = 0x00000000;

				if (((VRAMByte >> bit) & 1)) {
					ColorToDraw = 0xffffffff;
				}

				uint8_t CoordX = x;
				uint8_t CoordY = (256 - 1 - (y + bit));
				pixels[CoordY * 224 + CoordX] = ColorToDraw;
			}
		}
	}
}

uint8_t SpaceInvaders::in(const uint8_t port) {
	switch (port) {
	case 0:
		return (_version == "sitv") ? 1 : 0;
	case 1:
	{
		uint8_t port = 0b10001000;

		const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

		if (Keyboard[SDL_SCANCODE_RETURN])
			port |= 1;
		if (Keyboard[SDL_SCANCODE_2])
			port |= 2;
		if (Keyboard[SDL_SCANCODE_1])
			port |= 4;
		if (Keyboard[SDL_SCANCODE_SPACE])
			port |= 0x10;
		if (Keyboard[SDL_SCANCODE_LEFT])
			port |= 0x20;
		if (Keyboard[SDL_SCANCODE_RIGHT])
			port |= 0x40;
		return port;
	}
		break;
	case 2:
	{
		uint8_t port = 0b00000000;

		const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

		port |= _ships;
		port |= (_extraShip)? 1 << 3 : 0;
		port |= (_coinInfo)? 1 << 7 : 0;

		if ((_version=="sitv")&&(Keyboard[SDL_SCANCODE_T]))
			port |= 0x04;
		if (Keyboard[SDL_SCANCODE_LEFT])
			port |= 0x20;
		if (Keyboard[SDL_SCANCODE_RIGHT])
			port |= 0x40;
		if (Keyboard[SDL_SCANCODE_SPACE])
			port |= 0x10;
		return port;
	}
	break;
	case 3: {
		uint16_t v = (_shift1 << 8) | _shift0;
		return (uint8_t)((v >> (8 - _shift_offset)) & 0xff); }
		  break;
	default:
		return (uint8_t)0;
		break;
	}
}
