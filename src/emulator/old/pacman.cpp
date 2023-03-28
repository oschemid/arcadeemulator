#include "pacman.h"
#include <fstream>
#include <chrono>
#include "time.h"
#include <iostream>
#include "SDL2/SDL.h"


static ae::emulator::Emulator::registry reg("pacman", [](const ae::emulator::Game& game) { return std::make_unique<ae::machine::Pacman>(game); });


ae::machine::Pacman::Pacman(const emulator::Game&) :
	memory(nullptr),
	cpu(nullptr),
	interrupt_enabled(false),
	sound_enabled(false),
	flip_screen(false),
	coinage("Coinage"),
	lives("Lives"),
	bonus("Bonus"),
	difficulty("Difficulty"),
	ghostname("Ghost Name"),
	rackadvance("Rack Advance (Cheat)")
{
	coinage.addAlias(0, "Free play");
	coinage.addAlias(1, "1 coin / 1 credit");
	coinage.addAlias(2, "1 coin / 2 credits");
	coinage.addAlias(3, "2 coins / 1 credit");
	coinage.setValue(1);
	lives.addAlias(0, "1");
	lives.addAlias(1, "2");
	lives.addAlias(2, "3");
	lives.addAlias(3, "5");
	lives.setValue(2);
	bonus.addAlias(0, "10000 points");
	bonus.addAlias(1, "15000 points");
	bonus.addAlias(2, "20000 points");
	bonus.addAlias(3, "no");
	bonus.setValue(0);
	difficulty.addAlias(0, "Hard");
	difficulty.addAlias(1, "Normal");
	difficulty.setValue(1);
	ghostname.addAlias(0, "Alternate");
	ghostname.addAlias(1, "Normal");
	ghostname.setValue(1);
	rackadvance.addAlias(0, "On");
	rackadvance.addAlias(1, "Off");
	rackadvance.setValue(1);
}

ae::emulator::SystemInfo ae::machine::Pacman::getSystemInfo() const
{
	return ae::emulator::SystemInfo{
		.geometry = {.width = 224, .height = 288}
	};
}

uint8_t ae::machine::Pacman::readMemory(const uint16_t p) const {
	const uint16_t p_mirror = p & 0x7fff;
	if (p_mirror == 0x5000)
		return in0();
	if (p_mirror == 0x5040)
		return in1();
	if (p_mirror == 0x5080)
		return dip();
	if ((p_mirror >= 0x5060) && (p_mirror <= 0x506F))
		return spritesxy[p_mirror - 0x5060];
	return memory->read(p_mirror);
}

const uint8_t ae::machine::Pacman::dip() const {
	return (coinage.getValue() |
			lives.getValue() << 2 |
			bonus.getValue() << 4 |
			difficulty.getValue() << 6 |
			ghostname.getValue() << 7);
}
const uint8_t ae::machine::Pacman::in0() const {
	uint8_t port = 0xff;
	if (rackadvance.getValue() == 0)
		port &= 0b11101111;

	const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

	if (Keyboard[SDL_SCANCODE_UP])
		port &= 0b11111110;
	if (Keyboard[SDL_SCANCODE_LEFT])
		port &= 0b11111101;
	if (Keyboard[SDL_SCANCODE_RIGHT])
		port &= 0b11111011;
	if (Keyboard[SDL_SCANCODE_DOWN])
		port &= 0b11110111;
	if (Keyboard[SDL_SCANCODE_RETURN])
		port &= 0b11011111;
	return port;
}

const uint8_t ae::machine::Pacman::in1() const {
	uint8_t port = 0xff;

	const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

	if (Keyboard[SDL_SCANCODE_UP])
		port &= 0b11111110;
	if (Keyboard[SDL_SCANCODE_LEFT])
		port &= 0b11111101;
	if (Keyboard[SDL_SCANCODE_RIGHT])
		port &= 0b11111011;
	if (Keyboard[SDL_SCANCODE_DOWN])
		port &= 0b11110111;
	if (Keyboard[SDL_SCANCODE_1])
		port &= 0b11011111;
	return port;
}

bool ae::machine::Pacman::writeMemory(const uint16_t p, const uint8_t v) {
	const uint16_t p_mirror = p & 0x7fff;
	if (p_mirror < 0x5000)
		return memory->write(p_mirror, v);
	if ((p_mirror >= 0x50C0) && (p_mirror <= 0x50FF))
		// Watchdog
		return true;
	if ((p_mirror >= 0x5040) && (p_mirror <= 0x505F))
		// Sound
		return true;
	if ((p_mirror >= 0x5060) && (p_mirror <= 0x506F)) {
		spritesxy[p_mirror - 0x5060] = v;
		return true;
	}
	switch (p_mirror) {
	case 0x5000:
		interrupt_enabled = ((v & 1) == 1) ? true : false;
		break;
	case 0x5001:
		sound_enabled = ((v & 1) == 1) ? true : false;
		break;
	case 0X5002:
	case 0X5004:
	case 0x5005:
	case 0x5006:
	case 0x5007:
		break;
	case 0x5003:
		flip_screen = ((v & 1) == 1) ? true : false;
		break;
	default:
		return false;
	}
	return true;
}
void ae::machine::Pacman::init()
{
	_src = new uint32_t[224 * 288];

	cpu = xprocessors::Cpu::create("Z80");
	memory = newMemory(0x5000);
	videorom = newMemory(0x2000);
	paletterom = newMemory(0x200);
	spritesxy = new uint8_t[0x10];
	loadMemory();
	load_palettes();

	tiles = new uint8_t[256 * 64];
	for (uint16_t tileno = 0; tileno < 256; ++tileno) {
		for (uint8_t i = 0; i < 8; ++i) {
			const uint8_t m = videorom->read(tileno * 16 + i);
			uint8_t m1 = m & 0x0f;
			uint8_t m2 = m >> 3;
			tiles[tileno * 64 + 63 - i] = (m1 & 1) | (m2 & 2);
			m1 >>= 1; m2 >>= 1;
			tiles[tileno * 64 + 55 - i] = (m1 & 1) | (m2 & 2);
			m1 >>= 1; m2 >>= 1;
			tiles[tileno * 64 + 47 - i] = (m1 & 1) | (m2 & 2);
			m1 >>= 1; m2 >>= 1;
			tiles[tileno * 64 + 39 - i] = (m1 & 1) | (m2 & 2);
		}
		for (uint8_t i = 0; i < 8; ++i) {
			const uint8_t m = videorom->read(tileno * 16 + 8 + i);
			uint8_t m1 = m & 0x0f;
			uint8_t m2 = m >> 3;
			tiles[tileno * 64 + 31 - i] = (m1 & 1) | (m2 & 2);
			m1 >>= 1; m2 >>= 1;
			tiles[tileno * 64 + 23 - i] = (m1 & 1) | (m2 & 2);
			m1 >>= 1; m2 >>= 1;
			tiles[tileno * 64 + 15 - i] = (m1 & 1) | (m2 & 2);
			m1 >>= 1; m2 >>= 1;
			tiles[tileno * 64 + 7 - i] = (m1 & 1) | (m2 & 2);
		}
	}
	sprites = new uint8_t[64 * 256];
	for (uint16_t spriteno = 0; spriteno < 64; ++spriteno) {
		for (uint8_t i = 0; i < 64; ++i) {
			const uint8_t x = (i < 32) ? 15 - (i % 8) : 7 - (i % 8);
			const uint8_t y = (i % 32 < 8) ? 15 : ((i % 32) / 8) * 4 - 1;
			const uint8_t m = videorom->read(0x1000 + spriteno * 64 + i);
			uint8_t m1 = m & 0x0f;
			uint8_t m2 = m >> 3;
			sprites[spriteno * 256 + x + y * 16] = (m1 & 1) | (m2 & 2);
			m1 >>= 1; m2 >>= 1;
			sprites[spriteno * 256 + x + (y - 1) * 16] = (m1 & 1) | (m2 & 2);
			m1 >>= 1; m2 >>= 1;
			sprites[spriteno * 256 + x + (y - 2) * 16] = (m1 & 1) | (m2 & 2);
			m1 >>= 1; m2 >>= 1;
			sprites[spriteno * 256 + x + (y - 3) * 16] = (m1 & 1) | (m2 & 2);
		}
	}
	cpu->in([this](const uint8_t p) { return 0; });
	cpu->out([this](const uint8_t p, const uint8_t v) { if (p == 0) interrupt_vector = v; return; });
	cpu->read([this](const uint16_t p) { return this->readMemory(p); });
	cpu->write([this](const uint16_t p, const uint8_t v) { return this->writeMemory(p, v); });

	StartTime = std::chrono::high_resolution_clock::now();
}


void ae::machine::Pacman::load_palettes() {
	for (int i = 0; i < 32; ++i) {
		const uint8_t data = paletterom->read(i);
		uint8_t r = (data & 1) * 0x21 + ((data >> 1) & 1) * 0x47 + ((data >> 2) & 1) * 0x97;
		uint8_t g = ((data >> 3) & 1) * 0x21 + ((data >> 4) & 1) * 0x47 + ((data >> 5) & 1) * 0x97;
		uint8_t b = ((data >> 6) & 1) * 0x51 + ((data >> 7) & 1) * 0xae;
		colors[i] = 0xff000000 | (b << 16) | (g << 8) | r;
		//colors[i] = (r << 24) | (g << 16) | (b << 8) | 0xff;
	}
}

void ae::machine::Pacman::draw_tile(uint32_t* pixels,
									const uint8_t x,
									const uint8_t y,
									const uint8_t tile,
									const uint8_t palette) const {
	uint8_t offset_x = 0;
	uint8_t offset_y = 0;
	uint32_t colors_tile[4] = {
		colors[paletterom->read(0x100 + (4 * (palette & 0x3f)))],
		colors[paletterom->read(0x100 + (4 * (palette & 0x3f) + 1))],
		colors[paletterom->read(0x100 + (4 * (palette & 0x3f) + 2))],
		colors[paletterom->read(0x100 + (4 * (palette & 0x3f) + 3))],
	};
	for (uint8_t i = 0; i < 64; ++i) {
		const uint8_t p = tiles[tile * 64 + i];
		pixels[(x * 8 + offset_x) + (y * 8 + offset_y) * 224] = colors_tile[p];
		if (++offset_x == 8) {
			offset_x = 0; offset_y++;
		}
	}
}
void ae::machine::Pacman::draw_sprite(uint32_t* pixels,
									  const uint8_t x,
									  const uint8_t y,
									  const uint8_t sprite,
									  const uint8_t palette,
									  const bool flip_x,
									  const bool flip_y) const {
	uint32_t colors_tile[4] = {
		colors[paletterom->read(0x100 + (4 * (palette & 0x3f)))],
		colors[paletterom->read(0x100 + (4 * (palette & 0x3f) + 1))],
		colors[paletterom->read(0x100 + (4 * (palette & 0x3f) + 2))],
		colors[paletterom->read(0x100 + (4 * (palette & 0x3f) + 3))],
	};
	const int16_t x1 = 224 - x + 15;
	const int16_t y1 = 288 - y - 16;
	if ((x1 < 0) || (x1 >= 224 - 16))
		return;
	if ((y1 < 0) || (y1 >= 288 - 16))
		return;
	for (uint16_t i = 0; i < 256; ++i) {
		const uint8_t px = i % 16;
		const uint8_t py = i / 16;
		const uint8_t p = sprites[sprite * 256 + i];
		if (paletterom->read(0x100 + (4 * (palette & 0x3f) + p)) == 0)
			continue;
		pixels[(x1 + ((flip_x) ? 15 - px : px)) + (y1 + ((flip_y) ? 15 - py : py)) * 224] = colors_tile[p];
	}
}
void ae::machine::Pacman::updateDisplay(uint32_t* pixels) {
	uint16_t address = 0x4002;
	uint16_t x=2;
	uint16_t y=5;

	// bottom of screen
	for (uint16_t i = 0; i < 56; ++i) {
		const uint8_t tile = memory->read(address);
		const uint8_t palette = memory->read(address + 0x400);
		draw_tile(pixels, 27 - (i % 28), 34 + (i / 28), tile, palette);
		address++;
		if (i == 28) address += 4;
	}
	address += 2;

	// middle of screen
	for (uint16_t i = 0; i < 28 * 32; ++i) {
		const uint8_t tile = memory->read(address);
		const uint8_t palette = memory->read(address + 0x400);
		draw_tile(pixels, 27 - (i / 32), 2 + (i % 32), tile, palette);
		address++;
	}

	address += 2;
	// top of screen
	for (uint16_t i = 0; i < 56; ++i) {
		const uint8_t tile = memory->read(address);
		const uint8_t palette = memory->read(address + 0x400);
		draw_tile(pixels, 27 - (i % 28), (i / 28), tile, palette);
		address++;
		if (i == 28) address += 4;
	}

	// sprites
	for (int i = 7; i >= 0; --i) {
		const uint8_t spriteflips = memory->read(0x4ff0 + 2 * i);
		const uint8_t palette = memory->read(0x4ff1 + 2 * i);
		const uint16_t x = spritesxy[2 * i];
		const uint16_t y = spritesxy[1 + 2 * i];
		draw_sprite(pixels, x, y, spriteflips >> 2, palette, spriteflips & 2, spriteflips & 1);
	}
}

extern uint64_t getNanoSeconds(std::chrono::time_point<std::chrono::high_resolution_clock>* start);

void ae::machine::Pacman::run(ae::gui::RasterDisplay* raster)
{
	_raster = raster;

//	auto StartTime = std::chrono::high_resolution_clock::now();
	uint64_t CurrentTime = 0;

	const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

	while (0 == 0) {
		CurrentTime = getNanoSeconds(&StartTime);
		if (CurrentTime - LastThrottle < 1000000) {		// 1ms
			if (ClockCount < ClocksPerMS + ClockCompensation) {
				cpu->executeOne();
				ClockCount = cpu->elapsed_cycles();
			}
		}
		else {
			ClockCompensation += ClocksPerMS * (CurrentTime - LastThrottle) / 1000000;
			LastThrottle = CurrentTime;
		}
		if (CurrentTime - LastDisplay > 1000000000) {
			float a = (float)ClockCount / (CurrentTime / 1000);
			std::cout << a << std::endl;
			LastDisplay = CurrentTime;
		}
		if (CurrentTime - LastDraw > 1000000000 / 60 || LastDraw > CurrentTime) {
			LastDraw = CurrentTime;

			bool interrupt = false;
			if (interrupt_enabled) {
				draw();
				interrupt = cpu->interrupt(interrupt_vector);
			}
		}
		if (CurrentTime - LastInput > 1000000000 / 30 || LastInput > CurrentTime) { // 30 Hz - Manage Events
			LastInput = CurrentTime;
			while (SDL_PollEvent(&ev)) {
			}
			if (Keyboard[SDL_SCANCODE_ESCAPE]) {
				return;
			}
		}
	}
}

void ae::machine::Pacman::draw() {
	updateDisplay(_src);
	_raster->refresh((uint8_t*)_src);
}

void ae::machine::Pacman::loadMemory() {
	memory->map(0, 0x3fff, ae::IMemory::type::ROM);
	memory->map(0x4000, 0x4fff, ae::IMemory::type::RAM);
	memory->load(0, "roms/pacman/pacman.6e");
	memory->load(0x1000, "roms/pacman/pacman.6f");
	memory->load(0x2000, "roms/pacman/pacman.6h");
	memory->load(0x3000, "roms/pacman/pacman.6j");

	videorom->map(0, 0x1fff, ae::IMemory::type::ROM);
	videorom->load(0, "roms/pacman/pacman.5e");
	videorom->load(0x1000, "roms/pacman/pacman.5f");

	paletterom->map(0, 0x1ff, ae::IMemory::type::ROM);
	paletterom->load(0, "roms/pacman/pm1-1.7f");
	paletterom->load(0x100, "roms/pacman/pm1-4.4a");
}
