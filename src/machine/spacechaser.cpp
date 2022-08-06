#include "spacechaser.h"
#include "SDL2/SDL.h"


ae::machine::SpaceChaserCV::SpaceChaserCV() :
	Taito8080(0x5fff),
	ships("Ships"),
	extraShip("Extra ship"),
	coinInfo("Coin Info")
{
	ships.addAlias(0, "3");
	ships.addAlias(1, "4");
	ships.addAlias(2, "5");
	ships.addAlias(3, "6");
	extraShip.addAlias(0, "at 1500");
	extraShip.addAlias(1, "at 1000");
	coinInfo.addAlias(0, "On");
	coinInfo.addAlias(1, "Off");
}

std::list<ae::IParameter*> ae::machine::SpaceChaserCV::getParameters() const {
	return { (ae::IParameter*)&ships,
			 (ae::IParameter*)&extraShip,
			 (ae::IParameter*)&coinInfo };
}

const uint8_t ae::machine::SpaceChaserCV::in2() {
	uint8_t port = 0b00000000;

	const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

	port |= ships.getValue();
	port |= extraShip.getValue() << 3;
	port |= coinInfo.getValue() << 7;

	if (Keyboard[SDL_SCANCODE_LEFT])
		port |= 0x20;
	if (Keyboard[SDL_SCANCODE_RIGHT])
		port |= 0x40;
	if (Keyboard[SDL_SCANCODE_SPACE])
		port |= 0x10;
	return port;
}

const uint8_t ae::machine::SpaceChaserCV::in(const uint8_t port) {
	switch (port) {
	case 0:
		return 1;
		break;
	case 1:
		return in1();
		break;
	case 2:
		return in2();
	case 3: {
		uint16_t v = (shift1 << 8) | shift0;
		return (uint8_t)((v >> (8 - shift_offset)) & 0xff); }
		  break;
	default:
		return (uint8_t)0;
		break;
	}
}

const uint8_t ae::machine::SpaceChaserCV::in1() {
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

void ae::machine::SpaceChaserCV::loadMemory() {
	memory->map(0, 0x1FFF, ae::IMemory::type::ROM);
	memory->map(0x2000, 0x3FFF, ae::IMemory::type::RAM);
	memory->map(0x4000, 0x5FFF, ae::IMemory::type::ROM);
	memory->load(0, "roms/spacechaser/1");
	memory->load(0x0400, "roms/spacechaser/2");
	memory->load(0x0800, "roms/spacechaser/3");
	memory->load(0x0C00, "roms/spacechaser/4");
	memory->load(0x1000, "roms/spacechaser/5");
	memory->load(0x1400, "roms/spacechaser/6");
	memory->load(0x1800, "roms/spacechaser/7");
	memory->load(0x1C00, "roms/spacechaser/8");
	memory->load(0x4000, "roms/spacechaser/9");
	memory->load(0x4400, "roms/spacechaser/10");
}
