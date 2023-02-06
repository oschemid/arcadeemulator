#include "spacechaser.h"

using namespace ae::taito8080;

static ae::taito8080::RegistryHandler reg{ "spacechaser", [] { return std::unique_ptr<Cartridge>(new SpaceChaser()); } };


SpaceChaser::SpaceChaser() :
	Cartridge{},
	_shift1{ 0 },
	_shift0{ 0 },
	_shift_offset{ 0 },
	_ships{ 0 },
	_difficulty{ 0 }
{
	_memory = new uint8_t[0x6000];
	_colorram = new uint8_t[0x2000];
}

SpaceChaser::~SpaceChaser()
{
	delete[] _colorram;
}

void SpaceChaser::init(const json& settings)
{
	json gamesettings = settings.at("settings");
	gamesettings.at("difficulty").get_to(_difficulty);
	gamesettings.at("ships").get_to(_ships);

	string path;
	settings.at("roms").at("path").get_to(path);

	loadRom(path);
}

void SpaceChaser::loadRom(const string& path)
{
	File f1(path + "/schasercv/1");
	f1.read(0, 0x0400, _memory);
	File f2(path + "/schasercv/2");
	f2.read(0, 0x0400, _memory + 0x0400);
	File f3(path + "/schasercv/3");
	f3.read(0, 0x0400, _memory + 0x0800);
	File f4(path + "/schasercv/4");
	f4.read(0, 0x0400, _memory + 0x0c00);
	File f5(path + "/schasercv/5");
	f5.read(0, 0x0400, _memory + 0x1000);
	File f6(path + "/schasercv/6");
	f6.read(0, 0x0400, _memory + 0x1400);
	File f7(path + "/schasercv/7");
	f7.read(0, 0x0400, _memory + 0x1800);
	File f8(path + "/schasercv/8");
	f8.read(0, 0x0400, _memory + 0x1c00);
	File f9(path + "/schasercv/9");
	f9.read(0, 0x0400, _memory + 0x4000);
	File fa(path + "/schasercv/10");
	fa.read(0, 0x0400, _memory + 0x4400);
}
uint8_t SpaceChaser::read(const uint16_t address)
{
	if (address >= 0xc400) {
		uint16_t offset = address - 0xC400;
		return _colorram[(offset & 0x1f) | ((offset & 0x1f80) >> 2)];
	}
	return (address < 0x6000) ? _memory[address] : 0;
}

void SpaceChaser::write(const uint16_t address, const uint8_t value)
{
	if (address >= 0xc400) {
		uint16_t offset = address - 0xC400;
		_colorram[(offset & 0x1f) | ((offset & 0x1f80) >> 2)] = value;
		return;
	}
	if ((address > 0x1fff) && (address < 0x4000))
		_memory[address] = value;
}

void SpaceChaser::out(const uint8_t port, const uint8_t value) {
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

void SpaceChaser::updateDisplay(uint32_t* pixels)
{
	uint32_t ColorToDraw = 0xffff;
	for (int x = 0; x < 224; x++) {
		for (int y = 0; y < 256; y += 8) {
			uint16_t offset = (x << 5) + (y >> 3);
			uint8_t VRAMByte = read(0x2400 + offset);
			uint8_t color = _colorram[((offset & 0x1f) | ((offset & 0x1f80) >> 2))] & 0x07;
			for (int bit = 0; bit < 8; bit++) {
				ColorToDraw = 0xffff0000;
				if (((VRAMByte >> bit) & 1)) {
					switch (color) {
					case 0:
						ColorToDraw = 0x00ff8080;
						break;
					case 1:
						ColorToDraw = 0xff0000ff;
						break;
					case 2:
						ColorToDraw = 0xffff0000;
						break;
					case 3:
						ColorToDraw = 0xffff00ff;
						break;
					case 4:
						ColorToDraw = 0xff00ff00;
						break;
					case 5:
						ColorToDraw = 0xff00ffff;
						break;
					case 6:
						ColorToDraw = 0xffffff00;
						break;
					case 7:
						ColorToDraw = 0xffffffff;
						break;
					}
				}
				uint8_t CoordX = x;
				uint8_t CoordY = (256 - 1 - (y + bit));
				pixels[CoordY * 224 + CoordX] = ColorToDraw;
			}
		}
	}
}

uint8_t SpaceChaser::in(const uint8_t port) {
	switch (port) {
	case 0:
		return 0xff;
		break;
	case 1:
	{
		uint8_t port = 0;

		const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

		if (Keyboard[SDL_SCANCODE_RETURN])
			port |= 1;
		if (Keyboard[SDL_SCANCODE_2])
			port |= 2;
		if (Keyboard[SDL_SCANCODE_1])
			port |= 4;
		if (Keyboard[SDL_SCANCODE_DOWN])
			port |= 8;
		if (Keyboard[SDL_SCANCODE_SPACE])
			port |= 0x10;
		if (Keyboard[SDL_SCANCODE_LEFT])
			port |= 0x20;
		if (Keyboard[SDL_SCANCODE_RIGHT])
			port |= 0x40;
		if (Keyboard[SDL_SCANCODE_UP])
			port |= 0x80;
		return port;
	}
	case 2:
	{
		uint8_t port = 0b00000000;

		const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

		port |= _ships;

		port |= _difficulty << 3;

		if (Keyboard[SDL_SCANCODE_DOWN])
			port |= 0x02;
		if (Keyboard[SDL_SCANCODE_UP])
			port |= 0x04;

		if (Keyboard[SDL_SCANCODE_LEFT])
			port |= 0x20;
		if (Keyboard[SDL_SCANCODE_RIGHT])
			port |= 0x40;
		if (Keyboard[SDL_SCANCODE_SPACE])
			port |= 0x10;
		return port;
	}
	case 3: {
		uint16_t v = (_shift1 << 8) | _shift0;
		return (uint8_t)((v >> (8 - _shift_offset)) & 0xff); }
		  break;
	default:
		return (uint8_t)0;
		break;
	}
}
