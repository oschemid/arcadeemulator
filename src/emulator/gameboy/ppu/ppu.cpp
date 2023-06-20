#include "ppu.h"


using namespace ae::gameboy;

Ppu::Ppu() :
	Cpu{Cpu::MEM_AVAILABLE},
	_registers{ 0, 0, 0, 0, 0 },
	_dots(0),
	_mode(Ppu::NONE),
	_dmatransfer(0)
{
	_vram = new uint8_t[0x2000];
}

void Ppu::init(ae::display::RasterDisplay* raster)
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
	case MemoryMap::REGISTER_STAT:
		return _registers.stat;
	case MemoryMap::REGISTER_SCY:
		return _registers.scy;
	case MemoryMap::REGISTER_SCX:
		return _registers.scx;
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
	case MemoryMap::REGISTER_STAT:
		_registers.stat &= 0x07;
		_registers.stat |= v & 0x78;
		break;
	case MemoryMap::REGISTER_LY:
//		throw std::runtime_error("Trying to write ly");
//		_registers.ly = v;
		break;
	case MemoryMap::REGISTER_SCY:
		_registers.scy = v;
		break;
	case MemoryMap::REGISTER_SCX:
		_registers.scx = v;
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

void Ppu::set_stat_mode(const uint8_t mode)
{
	_registers.stat &= 0xfc;
	_registers.stat |= mode;
	switch (mode)
	{
	case 0:
		if (_registers.stat&0x08)
			_handlerWrite(0xff0f, _handlerRead(0xff0f) | 2);
		break;
	case 1:
		if (_registers.stat & 0x10)
			_handlerWrite(0xff0f, _handlerRead(0xff0f) | 2);
		break;
	case 2:
		if (_registers.stat & 0x20)
			_handlerWrite(0xff0f, _handlerRead(0xff0f) | 2);
		break;
	}
}

uint8_t Ppu::tileHigh(const uint8_t id, const uint8_t row, const bool isSprite) const
{
	const bool mode8000 = (isSprite) || (_registers.lcdc_tile_mode8000());

	if ((mode8000) || (id > 127)) {
		return _vram[(id << 4)+(row<<1)];
	}
	return _vram[0x1000 + (id << 4) + (row << 1)];
}

uint8_t Ppu::tileLow(const uint8_t id, const uint8_t row, const bool isSprite) const
{
	const bool mode8000 = (isSprite) || (_registers.lcdc_tile_mode8000());

	if ((mode8000) || (id > 127)) {
		return _vram[(id << 4) | (row << 1) | 1];
	}
	return _vram[0x1000 + (id << 4) | (row << 1)| 1];
}

void Ppu::pixel(const uint8_t x, const uint8_t y, const uint8_t c) {
	switch (c) {
	case 0x00:
		_raster->set(x, y, { .red = 0x9b, .green = 0xbc, .blue = 0x0f }); // src[x + 160 * y] = 0xff0fbc9b;
		break;
	case 0x01:
		_raster->set(x, y, { .red = 0x8b, .green = 0xac, .blue = 0x0f }); // src[x + 160 * y] = 0xff0fac8b;
		break;
	case 0x02:
		_raster->set(x, y, { .red = 0x30, .green = 0x62, .blue = 0x30 }); //src[x + 160 * y] = 0xff306230;
		break;
	case 0x03:
		_raster->set(x, y, { .red = 0x0f, .green = 0x38, .blue = 0x0f }); //src[x + 160 * y] = 0xff0f380f;
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
		return 0;
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
		}
		break;
	case DRAWING:
		if (!tickModePixelTransfer())
		{
			_mode = HBLANK;
			set_stat_mode(0);
		}
	}
	_dots++;
	if (_dots == 456)
	{
		_dots = 0;
		_registers.ly++;
		_registers.stat &= 0xfb;
		if (_registers.ly == _get_lyc()) {
			_registers.stat |= 0x04;
			if (_registers.stat & 0x40)
				_handlerWrite(0xff0f, _handlerRead(0xff0f) | 2);
		}

		if (_registers.ly < 144) {
			startModeOamSearch();
		}
		else if (_registers.ly == 144) {
			_mode = VBLANK;
			set_stat_mode(1);
			_handlerWrite(0xff0f, _handlerRead(0xff0f) | 1);
			draw();
		}
		else if (_registers.ly == 154) {
			_registers.ly = 0;
			startModeOamSearch();
		}
	}
	return 0;
}

void Ppu::draw() {
	_raster->refresh();
}
