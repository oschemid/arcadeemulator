#pragma once
#include "types.h"
#include "xprocessors.h"
#include "../memorymap.h"
#include "../../../gui/vulkan/engine.h"
#include <deque>


namespace ae::gameboy
{
	class Ppu : public xprocessors::Cpu
	{
	public:
		Ppu();

		void init(ae::gui::RasterDisplay*);

		uint8_t readOAM(const uint16_t) const;
		void writeOAM(const uint16_t, const uint8_t);

		uint8_t getRegister(const MemoryMap) const;
		void setRegister(const MemoryMap, const uint8_t);

	protected:
		// Video registers
		struct Registers {
			uint8_t lcdc;
			uint8_t stat;
			uint8_t scy;
			uint8_t scx;
			uint8_t ly;
			uint8_t wx;
			uint8_t wy;

			// Internal
			uint8_t lcdenable_delay;
			void set_lcdc(const uint8_t);

			// Helpers
			bool lcdc_bg_enable() const { return (lcdc & 0x01) ? true : false; }
			bool lcdc_sprite_enable() const { return (lcdc & 0x02) ? true : false; }
			uint16_t lcdc_bg_map() const { return (lcdc & 0x08) ? 0x1c00 : 0x1800; }
			bool lcdc_tile_mode8000() const { return (lcdc & 0x10) ? true : false; }
			bool lcdc_window_enable() const { return (lcdc & 0x20) ? true : false; }
			uint16_t lcdc_window_map() const { return (lcdc & 0x40) ? 0x1c00 : 0x1800; }
			bool lcd_enable() const { return (lcdc & 0x80) ? true : false; }
		} _registers;

		// OAM structure
		struct OAMSprite {
			uint8_t y;
			uint8_t x;
			uint8_t tile;
			uint8_t flags;

			bool isXFlip() const { return (flags & 0x20) ? true : false; }
		} _oam[40];

		// VRAM
		uint8_t* _vram;

		// Tile structure
		struct Tile {
			uint8_t id;
			uint8_t high;
			uint8_t low;
		};
		uint8_t tileHigh(const uint8_t, const uint8_t, const bool) const;
		uint8_t tileLow(const uint8_t, const uint8_t, const bool) const;

		// Scanline processing
		std::vector<uint8_t> _selectedSprites;
		std::deque<uint8_t> _pixelFifo;
		enum { NONE, HBLANK, VBLANK, OAMSEARCH, DRAWING } _mode;

		// Mode 2
		struct Mode2 {
			enum { STEP_1, STEP_2 } state;
			uint8_t offset;

			void init() { state = STEP_1; offset = 0; }
		} _modeOamSearch;
		void startModeOamSearch();
		bool tickModeOamSearch();

		// Mode 3
		struct Mode3 {
			enum { TILE_ID, TILE_HIGH, TILE_LOW, TILE_PUSH, SPRITE_ID, SPRITE_HIGH, SPRITE_LOW, SPRITE_PUSH } state;
			uint8_t lx;
			bool window;
			uint16_t mapAddress;
			uint8_t scrollX;
			uint8_t offset;
			uint8_t row;
			Tile tile;
			OAMSprite sprite;

			void init() { state = TILE_ID; window = false; tile = { 0, 0, 0 }; offset = 0; row = 0; lx = 0; scrollX = 0; }
			void add(const OAMSprite& s) { state = SPRITE_ID; sprite = s; }
			bool sprite_processing() const { return ((state == SPRITE_ID) || (state == SPRITE_HIGH) || (state == SPRITE_LOW) || (state == SPRITE_PUSH)) ? true : false; }
		} _modePixelTransfer;

		void startModePixelTransfer();
		void fetchModePixelTransfer();
		bool tickModePixelTransfer();

		uint16_t _dots;

		ae::gui::RasterDisplay* _raster;
		uint32_t* _src;
		
		uint8_t _dmatransfer;
		uint16_t _dmaaddress;

		uint8_t _get_lyc() const { return _handlerRead(0xff45); }
		uint8_t _get_bgp() const { return _handlerRead(0xff47); }
		uint8_t _get_obp(const uint8_t t) const { return _handlerRead(0xff48 + t); }

		void set_stat_mode(const uint8_t);

		void draw();
		void pixel(const uint8_t, const uint8_t, const uint8_t);
	public:
		uint8_t executeOne() override;
		bool reset(const uint16_t = 0) override { return true; }
		bool interrupt(const uint8_t) override { return true; }

		uint8_t readVRAM(const uint16_t p) { return _vram[p - 0x8000]; }
		void writeVRAM(const uint16_t p, const uint8_t v) { _vram[p - 0x8000] = v; }
	};
}