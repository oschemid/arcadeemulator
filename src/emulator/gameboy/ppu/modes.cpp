#include "ppu.h"

using namespace ae::gameboy;

void Ppu::startModeOamSearch()
{
	_modeOamSearch.init();
	_selectedSprites.clear();
	_mode = OAMSEARCH;
}

bool Ppu::tickModeOamSearch()
{
	switch (_modeOamSearch.state)
	{
	case Mode2::STEP_1:
		_modeOamSearch.state = Mode2::STEP_2;
		break;
	case Mode2::STEP_2:
		if (_selectedSprites.size() < 10) {
			OAMSprite sprite = _oam[_modeOamSearch.offset];
			if ((sprite.y <= _registers.ly + 16) && (sprite.y > _registers.ly + 8))
				_selectedSprites.push_back(_modeOamSearch.offset);
		}
		_modeOamSearch.state = Mode2::STEP_1;
		++_modeOamSearch.offset;
		break;
	}
	return _modeOamSearch.offset < 40;
}

void Ppu::startModePixelTransfer()
{
	_modePixelTransfer.init();
	_modePixelTransfer.mapAddress = (_registers.lcdc_bg_map()) ? 0x1c00 : 0x1800;

	uint8_t y = _registers.ly + _registers.scy;
	_modePixelTransfer.mapAddress += (y & 0xf8) << 2;
	_modePixelTransfer.row = y & 0x07;
	_modePixelTransfer.mapAddress += 0; // SCX
	_mode = DRAWING;
}

void Ppu::fetchModePixelTransfer()
{
	static uint8_t divider = 2;
	if (--divider == 0) {
		divider = 2;
		return;
	}
	switch (_modePixelTransfer.state)
	{
	case Mode3::TILE_ID:
		_modePixelTransfer.tile.id = _vram[_modePixelTransfer.mapAddress + _modePixelTransfer.offset];
		_modePixelTransfer.state = Mode3::TILE_HIGH;
		break;
	case Mode3::TILE_HIGH:
		_modePixelTransfer.tile.high = tileHigh(_modePixelTransfer.tile.id, _modePixelTransfer.row, false);
		_modePixelTransfer.state = Mode3::TILE_LOW;
		break;
	case Mode3::TILE_LOW:
		_modePixelTransfer.tile.low = tileLow(_modePixelTransfer.tile.id, _modePixelTransfer.row, false);
		_modePixelTransfer.state = Mode3::TILE_PUSH;
		break;
	case Mode3::TILE_PUSH:
		for (uint8_t i = 0; i < 8; ++i) {
			_pixelFifo.push_back(((_modePixelTransfer.tile.high & 0x80) >> 6) | (_modePixelTransfer.tile.low >> 7));
			_modePixelTransfer.tile.high <<= 1;
			_modePixelTransfer.tile.low <<= 1;
		}
		_modePixelTransfer.offset++;
		_modePixelTransfer.state = Mode3::TILE_ID;
		break;
	case Mode3::SPRITE_ID:
		_modePixelTransfer.tile.id = _modePixelTransfer.sprite.tile;
		_modePixelTransfer.state = Mode3::SPRITE_HIGH;
		break;
	case Mode3::SPRITE_HIGH:
		_modePixelTransfer.tile.high = tileHigh(_modePixelTransfer.tile.id, _registers.ly-(_modePixelTransfer.sprite.y-16), true);
		_modePixelTransfer.state = Mode3::SPRITE_LOW;
		break;
	case Mode3::SPRITE_LOW:
		_modePixelTransfer.tile.low = tileLow(_modePixelTransfer.tile.id, _registers.ly+16-_modePixelTransfer.sprite.y, true);
		_modePixelTransfer.state = Mode3::SPRITE_PUSH;
		break;
	case Mode3::SPRITE_PUSH:
		for (uint8_t i = 0; i < 8; ++i) {
			uint8_t color = ((_modePixelTransfer.tile.high & 0x80) >> 6) | (_modePixelTransfer.tile.low >> 7);
			_modePixelTransfer.tile.high <<= 1;
			_modePixelTransfer.tile.low <<= 1;
			if (color > 0) {
				if (_modePixelTransfer.sprite.isXFlip())
					_pixelFifo[7-i] = color;
				else
					_pixelFifo[i] = color;
			}
		}
		_modePixelTransfer.state = Mode3::TILE_ID;
		break;
	}
}

bool Ppu::tickModePixelTransfer()
{
	fetchModePixelTransfer();
	if (_pixelFifo.size() < 8)
		return true;
	if (_modePixelTransfer.sprite_processing())
		return true;
	if (_registers.lcdc_bg_enable())
	{
		// Check if window should be displayed
		if (!_modePixelTransfer.window) {
			if ((_registers.lcdc_window_enable()) && (_registers.ly >= _registers.wy) && (_modePixelTransfer.lx + 7 == _registers.wx))
			{
				_modePixelTransfer.window = true;
				_modePixelTransfer.mapAddress = (_registers.lcdc_window_map()) ? 0x1c00 : 0x1800;
				uint8_t y = _registers.ly - _registers.wy;
				_modePixelTransfer.mapAddress += (y & 0xf8) << 2;
				_modePixelTransfer.row = y & 0x07;
				_modePixelTransfer.offset = 0;
				_pixelFifo = {};
				_modePixelTransfer.state = Mode3::TILE_ID;
				return true;
			}
		}
	}
	if (_registers.lcdc_sprite_enable())
	{
		for (auto it=_selectedSprites.begin();it<_selectedSprites.end();++it)
		{
			OAMSprite sprite = _oam[*it];
			if (sprite.x == _modePixelTransfer.lx+8)
			{
				_modePixelTransfer.add(sprite);
				_selectedSprites.erase(it);
				return true;
			}
		}
	}

	//DISPLAY
	uint8_t color = _pixelFifo.front();
	pixel(_modePixelTransfer.lx++, _registers.ly, color);
	_pixelFifo.pop_front();
	if (_modePixelTransfer.lx == 160)
		return false;
	return true;
}