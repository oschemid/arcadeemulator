#include "spaceinvaders.h"
#include <fstream>
#include <chrono>
#include "time.h"
#include <iostream>
#include "SDL2/SDL.h"
#include "../registry.h"

static ae::emulator::RegistryHandler reg("spaceinvadersmiday", [] { return std::make_unique<ae::machine::SpaceInvadersMidway>(); });
static ae::emulator::RegistryHandler reg2("spaceinvaderstv", [] { return std::make_unique<ae::machine::SpaceInvadersTV>(); });

ae::machine::SpaceInvaders::SpaceInvaders() :
	Taito8080(0x3ffff /*, ae::machine::Taito8080::invaders_layout*/),
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

const uint8_t ae::machine::SpaceInvaders::in1() {
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
/*
std::list<ae::IParameter*> ae::machine::SpaceInvaders::getParameters() const {
	return { (ae::IParameter*)&ships,
			 (ae::IParameter*)&extraShip,
			 (ae::IParameter*)&coinInfo };
}*/

void ae::machine::SpaceInvaders::out(const uint8_t port, const uint8_t value) {
	if (port == 3) {
		if ((value & 2) == 2) {
			audiodevice->playSound(s1);
		}
		return;
	}
	Taito8080::out(port, value);
}

const uint8_t ae::machine::SpaceInvaders::in2() {
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

const uint8_t ae::machine::SpaceInvaders::in(const uint8_t port) {
	switch (port) {
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

ae::machine::SpaceInvadersMidway::SpaceInvadersMidway() :
	SpaceInvaders()
{
}

void ae::machine::SpaceInvadersMidway::loadMemory() {
	memory->map(0, 0x1fff, ae::IMemory::type::ROM);
	memory->map(0x2000, 0x3fff, ae::IMemory::type::RAM);
	memory->load(0, "roms/spaceinvaders/invaders/invaders.h");
	memory->load(0x0800, "roms/spaceinvaders/invaders/invaders.g");
	memory->load(0x1000, "roms/spaceinvaders/invaders/invaders.f");
	memory->load(0x1800, "roms/spaceinvaders/invaders/invaders.e");


	s1 = new ae::Sound("roms/spaceinvaders/shoot.wav");
	audiodevice = new ae::AudioDevice();

}

ae::machine::SpaceInvadersTV::SpaceInvadersTV() :
	SpaceInvaders()
{
}

void ae::machine::SpaceInvadersTV::loadMemory() {
	memory->map(0, 0x1fff, ae::IMemory::type::ROM);
	memory->map(0x2000, 0x3fff, ae::IMemory::type::RAM);
	memory->load(0, "roms/spaceinvaders/sitv/tv0h.s1");
	memory->load(0x0800, "roms/spaceinvaders/sitv/tv02.rp1");
	memory->load(0x1000, "roms/spaceinvaders/sitv/tv03.n1");
	memory->load(0x1800, "roms/spaceinvaders/sitv/tv04.m1");
}

const uint8_t ae::machine::SpaceInvadersTV::in(const uint8_t port) {
	if (port == 0)
		return 1;
	return ae::machine::SpaceInvaders::in(port);
}

const uint8_t ae::machine::SpaceInvadersTV::in2() {
	uint8_t port = SpaceInvaders::in2();

	const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

	if (Keyboard[SDL_SCANCODE_T])
		port |= 0x04;
	return port;
}
