#pragma once
#include "types.h"
#include "../../display/rasterdisplay.h"
#include "xprocessors.h"


namespace ae::gameboy {
	class Ppu : public xprocessors::Cpu {
	protected:
		class Pixel_Fifo {
		public:
			struct Item {
				uint8_t color;
				uint8_t origin;
				uint8_t palette;
			};

			Pixel_Fifo();
			const Item pop();
			void push(const uint8_t lowB, const uint8_t highB, const uint8_t origin, const uint8_t palette);
			void empty();
			bool is_empty() const { return _index == 8; }

		protected:
			uint8_t _index;
			Item _items[8];
		};

		Pixel_Fifo _background_fifo;
		Pixel_Fifo _sprite_fifo;
		enum { HBLANK, VBLANK, OAMSEARCH, DRAWING } _mode;
		uint16_t _dots;
		uint8_t _lx;

		struct {
			uint8_t _y;
			uint8_t _x;
			uint8_t _tile;
			uint8_t _flags;
		} _oamobject[10];
		uint8_t _oamobject_count;

		struct {
			uint8_t _cycle;
			uint8_t _tile_count;
			uint8_t _tile;
			uint16_t _tile_address;
			uint8_t _low;
			uint8_t _high;
			uint8_t _x;
			uint8_t _y;
			uint8_t _flags;
		} _fetcher;
		bool _fetcher_stopped;
		uint8_t _nextsprite;
		display::RasterDisplay _display;
		uint8_t _dmatransfer;
		uint16_t _dmaaddress;

		uint8_t _get_lcdc() const { return _handlerRead(0xff40); }
		uint8_t _get_scrolly() const { return _handlerRead(0xff42); }
		uint8_t _get_ly() const { return _handlerRead(0xff44); }
		void _set_ly(const uint8_t v) { _handlerWrite(0xff44, v); }
		uint8_t _get_lyc() const { return _handlerRead(0xff45); }
		uint8_t _get_stat() const { return _handlerRead(0xff41); }
		uint8_t _get_bgp() const { return _handlerRead(0xff47); }
		uint8_t _get_obp(const uint8_t t) const { return _handlerRead(0xff48 + t); }
		void _set_stat(const uint8_t v) { _handlerWrite(0xff41, v); }
		bool is_lcdc_on() const { return _get_lcdc() & 0x80; }

		void draw();
		void pixel(const uint8_t, const uint8_t, const uint8_t);
		bool displayVram;
	public:
		Ppu();

		uint8_t executeOne() override;
		bool reset(const uint16_t = 0) override { return true; }
		bool interrupt(const uint8_t) override { return true; }

		void switchDisplayVram();
	};
}