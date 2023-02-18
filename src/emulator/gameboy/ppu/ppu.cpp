#include "ppu.h"


using namespace ae::gameboy;

Ppu::Ppu() :
	_registers{ 0, 0, 0, 0, 0 },
	_dots(0),
	_mode(Ppu::NONE),
	_dmatransfer(0)
{
	_src = new uint32_t[160 * 144];
	_vram = new uint8_t[0x2000];
}

void Ppu::init(ae::gui::RasterDisplay* raster)
{
	_raster = raster;
}

uint8_t Ppu::readOAM(const uint16_t address) const
{
	const uint8_t index = (address - 0xfe00) >> 2;
	const uint8_t attribute = address & 0x0003;
	if (index < 40)
	{
		switch (attribute)
		{
		case 0:
			return _oam[index].y;
		case 1:
			return _oam[index].x;
		case 2:
			return _oam[index].tile;
		case 3:
			return _oam[index].flags;
		}
	}
	return 0xff;
}

void Ppu::writeOAM(const uint16_t address, const uint8_t value)
{
	const uint8_t index = (address - 0xfe00) >> 2;
	const uint8_t attribute = address & 0x0003;
	if (index < 40)
	{
		switch (attribute)
		{
		case 0:
			_oam[index].y = value;
			break;
		case 1:
			_oam[index].x = value;
			break;
		case 2:
			_oam[index].tile = value;
			break;
		case 3:
			_oam[index].flags = value;
			break;
		}
	}
}

uint8_t Ppu::getRegister(const MemoryMap io) const
{
	switch (io)
	{
	case MemoryMap::REGISTER_LCDC:
		return _registers.lcdc;
	case MemoryMap::REGISTER_SCY:
		return _registers.scy;
	case MemoryMap::REGISTER_LY:
		return _registers.ly;
	case MemoryMap::REGISTER_WX:
		return _registers.wx;
	case MemoryMap::REGISTER_WY:
		return _registers.wy;
	default:
		return 0;
	}
}

void Ppu::setRegister(const MemoryMap io, const uint8_t v)
{
	switch (io)
	{
	case MemoryMap::REGISTER_LCDC:
		_registers.set_lcdc(v);
		break;
	case MemoryMap::REGISTER_LY:
		_registers.ly = v;
		break;
	case MemoryMap::REGISTER_SCY:
		_registers.scy = v;
		break;
	case MemoryMap::REGISTER_WX:
		_registers.wx = v;
		break;
	case MemoryMap::REGISTER_WY:
		_registers.wy = v;
		break;
	}
}

void Ppu::Registers::set_lcdc(const uint8_t v)
{
	if (!lcd_enable())
	{
		bool lcd_enable = (v & 0x80) ? true : false;
		if (lcd_enable)
			lcdenable_delay = 244;
	}
	lcdc = v;
}

uint8_t Ppu::tileHigh(const uint8_t id, const uint8_t row, const bool isSprite) const
{
	const bool mode8000 = (isSprite) || (_registers.lcdc_tile_area());

	if ((mode8000) || (id > 127)) {
		return _vram[(id << 4)+(row<<1)];
	}
	return _vram[0x1000 + (id << 4) + (row << 1)];
}

uint8_t Ppu::tileLow(const uint8_t id, const uint8_t row, const bool isSprite) const
{
	const bool mode8000 = (isSprite) || (_registers.lcdc_tile_area());

	if ((mode8000) || (id > 127)) {
		return _vram[(id << 4) | (row << 1) | 1];
	}
	return _vram[0x1000 + (id << 4) | (row << 1)| 1];
}

void Ppu::pixel(const uint8_t x, const uint8_t y, const uint8_t c) {
	switch (c) {
	case 0x00:
		_src[x + 160 * y] = 0xff0fbc9b;
		break;
	case 0x01:
		_src[x + 160 * y] = 0xff0fac8b;
		break;
	case 0x02:
		_src[x + 160 * y] = 0xff306230;
		break;
	case 0x03:
		_src[x + 160 * y] = 0xff0f380f;
		break;
	}
}

uint8_t Ppu::executeOne() {
	if (!_registers.lcd_enable())
		return 0;
	if (_registers.lcdenable_delay > 0) {
		// Incorrect
		--_registers.lcdenable_delay;
		return 0;
	}

	// DMA Transfer tempporaire
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
		writeOAM(0xfe00|_dmatransfer, _handlerRead(_dmaaddress | _dmatransfer));
	}

	switch (_mode)
	{
	case NONE:
		startModeOamSearch();
		[[fallthrough]];
	case OAMSEARCH:
		if (!tickModeOamSearch()) {
			startModePixelTransfer();
			_pixelFifo = {};

			_set_stat(_get_stat() | 3);
		}
		break;
	case DRAWING:
		if (!tickModePixelTransfer())
		{
			_mode = HBLANK;
		}
	}
	_dots++;
	if (_dots == 456)
	{
		_dots = 0;
		_registers.ly++;
		if (_registers.ly == _get_lyc())
			_set_stat(_get_stat() | 4);
		else
			_set_stat(_get_stat() | 0xfb);

		if (_registers.ly < 144) {
			startModeOamSearch();
			_set_stat((_get_stat() & 0xfc) | 2);
		}
		else if (_registers.ly == 144) {
			_mode = VBLANK;
			_set_stat((_get_stat() & 0xfc) | 1);
			_handlerWrite(0xff0f, _handlerRead(0xff0f) | 1);
			draw();
		}
		else if (_registers.ly == 154) {
			_registers.ly = 0;
			startModeOamSearch();
			_set_stat((_get_stat() & 0xfc) | 2);
		}
	}
	return 0;
}

void Ppu::draw() {
	_raster->refresh((uint8_t*)_src);
}
