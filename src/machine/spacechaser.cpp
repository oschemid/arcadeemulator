#include "spacechaser.h"
#include "SDL2/SDL.h"


ae::machine::SpaceChaserCV::SpaceChaserCV() :
	Taito8080(0x5fff),
	ships("Ships"),
	difficulty("Difficulty"),
	colorram(nullptr)
{
	ships.addAlias(0, "3");
	ships.addAlias(1, "4");
	difficulty.addAlias(0, "Easy");
	difficulty.addAlias(1, "Hard");
}

std::list<ae::IParameter*> ae::machine::SpaceChaserCV::getParameters() const {
	return { (ae::IParameter*)&ships,
			 (ae::IParameter*)&difficulty };
}

const uint8_t ae::machine::SpaceChaserCV::in2() {
	uint8_t port = 0b00000000;

	const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

	port |= ships.getValue();

	port |= difficulty.getValue() << 3;

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

const uint8_t ae::machine::SpaceChaserCV::in(const uint8_t port) {
	switch (port) {
	case 0:
		return 0xff;
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
	memory->load(0, "roms/spacechaser/schasercv/1");
	memory->load(0x0400, "roms/spacechaser/schasercv/2");
	memory->load(0x0800, "roms/spacechaser/schasercv/3");
	memory->load(0x0C00, "roms/spacechaser/schasercv/4");
	memory->load(0x1000, "roms/spacechaser/schasercv/5");
	memory->load(0x1400, "roms/spacechaser/schasercv/6");
	memory->load(0x1800, "roms/spacechaser/schasercv/7");
	memory->load(0x1C00, "roms/spacechaser/schasercv/8");
	memory->load(0x4000, "roms/spacechaser/schasercv/9");
	memory->load(0x4400, "roms/spacechaser/schasercv/10");
}

const uint8_t ae::machine::SpaceChaserCV::read_colorram(const uint16_t p) {
	uint16_t offset = p - 0xC400;
	return colorram->read((offset & 0x1f) | ((offset & 0x1f80) >> 2));
}
bool ae::machine::SpaceChaserCV::write_colorram(const uint16_t p, const uint8_t v) {
	uint16_t offset = p - 0xC400;
	return colorram->write((offset & 0x1f) | ((offset & 0x1f80) >> 2), v);
}
bool ae::machine::SpaceChaserCV::init() {
	ae::machine::Taito8080::init();

	if (!colorram) {
		colorram = newMemory(0x2000);
		colorram->map(0, 0x1FFF, ae::IMemory::type::RAM);
	}

	cpu->read([this](const uint16_t p) { return (p < 0x6000) ? memory->read(p) : this->read_colorram(p); });
	cpu->write([this](const uint16_t p, const uint8_t v) { return (p < 0x6000) ? memory->write(p, v) : this->write_colorram(p, v); });
	return true;
}

void ae::machine::SpaceChaserCV::updateDisplay(uint16_t* pixels) {
	uint32_t ColorToDraw = 0xffff;
	for (int x = 0; x < 224; x++) {
		for (int y = 0; y < 256; y += 8) {
			uint16_t offset = (x << 5) + (y >> 3);
			uint8_t VRAMByte = memory->read(0x2400 + offset);
			uint8_t color = colorram->read((offset & 0x1f) | ((offset & 0x1f80) >> 2)) & 0x07;
			for (int bit = 0; bit < 8; bit++) {
				ColorToDraw = 0xf00f;
				if (((VRAMByte >> bit) & 1)) {
					switch (color) {
					case 0:
						ColorToDraw = 0x088f;
						break;
					case 1:
						ColorToDraw = 0xff00;
						break;
					case 2:
						ColorToDraw = 0xf00f;
						break;
					case 3:
						ColorToDraw = 0xff0f;
						break;
					case 4:
						ColorToDraw = 0xf0f0;
						break;
					case 5:
						ColorToDraw = 0xfff0;
						break;
					case 6:
						ColorToDraw = 0xf0ff;
						break;
					case 7:
						ColorToDraw = 0xffff;
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