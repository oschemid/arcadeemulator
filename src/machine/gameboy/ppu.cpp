#include "ppu.h"
#include "../ui/ui.h"


using namespace ae::gameboy;

Ppu::Pixel_Fifo::Pixel_Fifo() :
	_index(0)
{}

const Ppu::Pixel_Fifo::Item Ppu::Pixel_Fifo::pop() {
	if (_index < 8) {
		return _items[_index++];
	}
	throw "ERROR";
}

void Ppu::Pixel_Fifo::push(const uint8_t lowB, const uint8_t highB, const uint8_t origin,
							  const uint8_t palette) {
	if (_index < 8)
		throw "ERROR";
	uint8_t l = lowB, h = highB;
	for (uint8_t i = 0; i < 8; ++i) {
		_items[i] = { (uint8_t)(((h >> 7) << 1) | (l >> 7)), origin, palette };
		h <<= 1;
		l <<= 1;
	}
	_index = 0;
}

void Ppu::Pixel_Fifo::empty() {
	_index = 8;
}

Ppu::Ppu() :
	_lx(0),
	_dots(0),
	_mode(OAMSEARCH),
	_display(),
	displayVram(false),
	_dmatransfer(0),
	_oamobject_count(0)
{
	_display.setSize(160, 144);
	_display.registerCallback([](uint32_t*) { return; });
	_display.init();
}

void Ppu::pixel(const uint8_t x, const uint8_t y, const uint8_t c) {
	switch (c) {
	case 0x00:
		_display.setPixel(x, y, 0x009bbc0f);
		break;
	case 0x01:
		_display.setPixel(x, y, 0x008bac0f);
		break;
	case 0x02:
		_display.setPixel(x, y, 0x00306230);
		break;
	case 0x03:
		_display.setPixel(x, y, 0x000f380f);
		break;
	}
}

uint8_t Ppu::executeOne() {
	if (_dmatransfer == 0) {
		uint8_t dma = _handlerRead(0xff46);
		if (dma) {
			_handlerWrite(0xff46, 0);
			_dmatransfer = 160;
			_dmaaddress = dma << 8;
		}
	}
	if (_dmatransfer) {
		_dmatransfer--;
		_handlerWrite(0xfe00 | _dmatransfer, _handlerRead(_dmaaddress | _dmatransfer));
	}
	if (_mode == OAMSEARCH) {
		if (_dots == 0) {
			_oamobject_count = 0;
		}
		if ((_dots & 1) == 0) {
			if (_oamobject_count < 10) {
				uint8_t y = _get_ly();
				uint8_t oy = _handlerRead(0xfe00 | (_dots << 1));
				uint8_t ox = _handlerRead(0xfe01 | (_dots << 1));
				if ((ox != 0) && (oy <= y + 16) && (oy > y + 8)) {
					_oamobject[_oamobject_count++] = { oy,
						ox,
						_handlerRead(0xfe02 | (_dots << 1)),
						_handlerRead(0xfe03 | (_dots << 1))
					};
				}
			}
		}

		if (_dots == 79) {
			_mode = DRAWING;
			_lx = 0;
			_fetcher_stopped = false;
			_background_fifo.empty();
			_sprite_fifo.empty();
			_nextsprite = (_oamobject_count > 0) ? 0 : 10;
			_fetcher = { ._cycle = 0, ._tile_count = 0 };
			_set_stat(_get_stat() | 3);
		}
	}
	else if (_mode == DRAWING) {
		// FETCHER
		switch (_fetcher._cycle) {
		case 0:
			if (_fetcher_stopped) {
				_fetcher._y = _get_ly() - _oamobject[_nextsprite]._y;
				_fetcher._tile = _oamobject[_nextsprite]._tile;
				_fetcher._flags = _oamobject[_nextsprite]._flags;
			}
			else {
				_fetcher._y = (_get_ly() + _get_scrolly()) & 0xff;
				_fetcher._tile = _handlerRead(0x9800 + ((_fetcher._y & 0xf8) << 2) + _fetcher._tile_count);
			}
			break;
		case 2:
			_fetcher._high = _handlerRead(0x8000 + ((_fetcher._tile << 4) + ((_fetcher._y & 7) << 1)));
			break;
		case 4:
			_fetcher._low = _handlerRead(0x8000 + ((_fetcher._tile << 4) + ((_fetcher._y & 7) << 1)) + 1);
			break;
		case 6:
			if (!_fetcher_stopped)
				_fetcher._tile_count++;
			break;
		case 7:
			if (_fetcher_stopped) {
				_sprite_fifo.push(_fetcher._low, _fetcher._high, 0, _fetcher._flags);
				_nextsprite++;
				if (_nextsprite == _oamobject_count) {
					_nextsprite = 10;
					_fetcher_stopped = false;
				}
				else if (_oamobject[_nextsprite]._x - 8 > _lx)
					_fetcher_stopped = false;
			}
			else {
				_background_fifo.push(_fetcher._low, _fetcher._high, 0, _get_bgp());
				if ((_nextsprite < 10) && (_oamobject[_nextsprite]._x - 8 == _lx)) {
					_fetcher_stopped = true;
				}
			}
			break;
		}
		_fetcher._cycle++;
		_fetcher._cycle &= 7;

		if (!_fetcher_stopped) {
			if (!_background_fifo.is_empty()) {
				Ppu::Pixel_Fifo::Item item = _background_fifo.pop();
				uint8_t color = item.color;
				uint8_t palette = item.palette;
				if (!_sprite_fifo.is_empty()) {
					Ppu::Pixel_Fifo::Item sprite = _sprite_fifo.pop();
					if (sprite.color) {
						color = sprite.color;
						palette = (sprite.palette & 0x10) ? _get_obp(1) : _get_obp(0);
					}
				}
				if (!is_lcdc_on()) {
					if (_lx < 160) {
						_display.setPixel(_lx++, _get_ly(), 0x00000000);
					}
				}
				else {
					if (_lx < 160) {
						for (uint8_t i = 0; i < color; i++)
							palette >>= 2;
						color = palette & 0x03;
						pixel(_lx++, _get_ly(), color);
					}
				}
			}
		}
		if (_lx == 160) {
			_mode = HBLANK;
			_set_stat(_get_stat() & 0xfc);
		}
	}
	_dots++;
	if (_dots == 456)
	{
		_dots = 0;
		_set_ly(_get_ly() + 1);
		if (_get_ly() == _get_lyc())
			_set_stat(_get_stat() | 4);
		else
			_set_stat(_get_stat() | 0xfb);

		if (_get_ly() < 144) {
			_mode = OAMSEARCH;
			_set_stat((_get_stat() & 0xfc) | 2);
		}
		else if (_get_ly() == 144) {
			_mode = VBLANK;
			_set_stat((_get_stat() & 0xfc) | 1);
			_handlerWrite(0xff0f, _handlerRead(0xff0f) | 1);
			draw();
		}
		else if (_get_ly() == 154) {
			_set_ly(0);
			_mode = OAMSEARCH;
			_set_stat((_get_stat() & 0xfc) | 2);
		}
	}
	return 0;
}

void Ppu::switchDisplayVram() {
	displayVram = (displayVram) ? false : true;
}

void Ppu::draw() {
	SDL_Renderer* renderer = ae::ui::getRenderer();
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
	SDL_RenderClear(renderer);

	SDL_Rect rect;
	rect.x = 512 - 320;
	rect.y = 384 - 288;
	rect.w = 320 * 2;
	rect.h = 288 * 2;
	if (displayVram) {
		uint16_t address = 0x8000;
		for (uint16_t tile = 0; tile < 60; tile++) {
			uint16_t x = (tile % 10) * 8;
			uint16_t y = (tile / 10) * 8;
			for (uint8_t b = 0; b < 8; b++) {
				uint8_t a1 = _handlerRead(address++);
				uint8_t a2 = _handlerRead(address++);
				for (uint8_t c = 0; c < 8; c++) {
					uint8_t color = (a1 >> 7) | ((a2 >> 7) << 1);
					a1 <<= 1;
					a2 <<= 1;
					uint32_t truecolor = 0;
					switch (color) {
					case 1:
						truecolor = 0x00FF0000;
						break;
					case 2:
						truecolor = 0x0000FF00;
						break;
					case 3:
						truecolor = 0x000000FF;
						break;
					}
					_display.setPixel(x++, y, truecolor);
				}
				y++;
				x -= 8;
			}
		}
	}

	_display.update(rect);

	SDL_RenderPresent(renderer);
}
