#include "tilemap.h"
#include <SDL2/SDL.h>


using namespace ae::gameboy::debug;


void Debugger::run()
{
	const uint16_t map = 0x9800;
	const uint16_t tiledata = 0x8000;
	for (;;)
	{
		for (uint16_t address = 0; address < 32 * 32; ++address) {
			uint8_t tileid = _console->_mmu->read(map + address, ae::gameboy::Mmu::origin::ppu);
			for (uint8_t i = 0; i < 16; i += 2) {
				uint8_t tileh = _console->_mmu->read(tiledata + tileid * 16 + i, ae::gameboy::Mmu::origin::ppu);
				uint8_t tilel = _console->_mmu->read(tiledata + tileid * 16 + i+1, ae::gameboy::Mmu::origin::ppu);

				uint8_t x = (address % 32) * 8;
				uint8_t y = (address / 32) * 8 + (i>>1);
				_map[y * 256 + x] = (tileh & 0x80) >> 6 | (tilel & 0x80) >> 7;
				_map[y * 256 + x+1] = (tileh & 0x40) >> 5 | (tilel & 0x40) >> 6;
				_map[y * 256 + x+2] = (tileh & 0x20) >> 4 | (tilel & 0x20) >> 5;
				_map[y * 256 + x+3] = (tileh & 0x10) >> 3 | (tilel & 0x10) >> 4;
				_map[y * 256 + x+4] = (tileh & 0x08) >> 2 | (tilel & 0x08) >> 3;
				_map[y * 256 + x+5] = (tileh & 0x04) >> 1 | (tilel & 0x04) >> 2;
				_map[y * 256 + x + 6] = (tileh & 0x02) | (tilel & 0x02) >> 1;
				_map[y * 256 + x + 7] = (tileh & 0x01)<<1 | (tilel & 0x01) ;
			}
		}
		for (uint32_t t = 0; t < 256 * 256; t++)
		{
			switch (_map[t]) {
			case 0x00:
				_map[t] = 0xff0fbc9b;
				break;
			case 0x01:
				_map[t] = 0xff0fac8b;
				break;
			case 0x02:
				_map[t] = 0xff306230;
				break;
			case 0x03:
				_map[t] = 0xff0f380f;
				break;
			}
		}
		_raster->refresh((uint8_t*)_map);
		SDL_Delay(1000);
	}
}
