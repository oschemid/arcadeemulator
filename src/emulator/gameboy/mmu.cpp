#include "mmu.h"
#include "SDL2/SDL.h"


using namespace ae::gameboy;


Mmu::Mmu(std::shared_ptr<BootRom>& bootrom,
		 std::shared_ptr<Mbc>& cartridge)
	: _bootrom(bootrom),
	  _cartridge(cartridge),
	  _div{0},
	_handlerReadVRAM{nullptr},
	_handlerWriteVRAM{nullptr}
{
	_rams = new uint8_t[0x10000](0);
}

Mmu::~Mmu()
{
	delete[] _rams;
}

void Mmu::tick()
{
	if ((++_div & 0x1fff) == 0)
		notify(io::div, _div >> 8);
	if (_tac & 0x04) {
		uint8_t period = 0;
		switch (_tac & 0x03) {
		case 0:
			period = 1024;
			break;
		case 1:
			period = 16;
			break;
		case 2:
			period = 64;
			break;
		case 3:
			period = 256;
			break;
		}
		if ((_div & (period - 1)) == 0) {
			_tima++;
			if (_tima == 0) {
				_tima = _tma;
				_rams[0xff0f] |= 4;
			}
		}
	}
}

void Mmu::map(MemoryMap address, read_fn readfn, write_fn writefn)
{
	if (address == MemoryMap::VRAM) {
		_handlerReadVRAM = readfn;
		_handlerWriteVRAM = writefn;
		return;
	}
	if (address == MemoryMap::OAM) {
		_handlerReadOAM = readfn;
		_handlerWriteOAM = writefn;
		return;
	}
	if (address < MemoryMap::REGISTER_SB)
		return;
	if (address <= MemoryMap::REGISTER_WX) {
		_handlerReadIO[static_cast<uint16_t>(address) & 0xff] = readfn;
		_handlerWriteIO[static_cast<uint16_t>(address) & 0xff] = writefn;
		return;
	}
}
void Mmu::map(MemoryMap address1, MemoryMap address2, read_fn readfn, write_fn writefn)
{
	for (int i = address1; i <= address2; i++)
		map(static_cast<MemoryMap>(i), readfn, writefn);
}

void Mmu::notify(const uint8_t io, const uint8_t value) const {
	for (auto& cb : _io_callbacks)
		cb(io, value);
}

uint8_t Mmu::in(const uint8_t io, const origin caller) const {
	switch (io) {
	case io::div:
		return _div >> 8;
	case 2:
	case 1:
		if (_handlerReadIO[io])
			return _handlerReadIO[io](0xff00+io);
		return 0;
	case io::tima:
		return _tima;
	case io::tma:
		return _tma;
	case io::tac:
		return _tac;
	default:
		if (io<0x80)
			return _rams[0xff00 + io];
		throw std::runtime_error("Bad io reading");
	}
}

bool Mmu::out(const uint8_t io, const uint8_t value, const origin caller) {
	switch (io) {
	case 2:
	case 1:
		if (_handlerWriteIO[io]) {
			_handlerWriteIO[io](0xff00+io, value);
			return true;
		}
		return false;
	case io::div:
		_div = 0;
		break;
	case io::tima:
		_tima = value;
		break;
	case io::tma:
		_tma = value;
		break;
	case io::tac:
		_tac = value;
		break;
	case io::stat:
		if (caller == origin::ppu) {
			_rams[0xff00+io] &= 0xf8;
			_rams[0xff00+io] |= value & 0x07;
		}
		else {
			_rams[0xff00 + io] &= 0x07;
			_rams[0xff00 + io] |= value & 0xf8;
		}
		break;
	case io::nr52:
		if (caller != origin::apu) {
			_rams[0xff + io] &= 0x7f;
			_rams[0xff + io] |= value & 0x80;
		}
		else {
			_rams[0xff00 + io] = value;
		}
		break;
	case io::ly:
		if (caller != origin::ppu)
			return false;
		_rams[0xff00 + io] = value;
		break;
	default:
		if ((io==0x0f)&&(value&0x08))
			_rams[0xff00 + io] = value;

		if (io >= 0x80)
			throw std::runtime_error("Bad io writing");
		_rams[0xff00 + io] = value;
		break;
	}
	notify(io, value);
	return true;
}

uint8_t Mmu::read(const uint16_t address, const origin caller) const {
	// ROM cartridge
	if (address < MemoryMap::VRAM) {
		if ((address < 0x100) && (in(io::ff50, caller) == 0))
			return _bootrom->read(address);
		return _cartridge->read(address);
	}

	// VRAM
	if (address < MemoryMap::EXTRA_RAM)
		return (_handlerReadVRAM)? _handlerReadVRAM(address) : 0;

	// External RAM
	if (address < 0xc000)
		return _cartridge->read(address);

	// WRAM
//	if (address < 0xe000)
//		return memory->read(address);

	// Echo RAM
	if (address < MemoryMap::OAM)
		return _rams[address];

	// OAM
	if (address < MemoryMap::NOTUSE)
		return (_handlerReadOAM)? _handlerReadOAM(address) : 0;

	if (address >= 0xff00)
		if (address < 0xff80) {
			if (address == 0xff00) {
				const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);
				uint8_t value = in(0, caller) & 0xf0;
				if ((value & 0x10) == 0) {
					if (!Keyboard[SDL_SCANCODE_UP])
						value |= 0x04;
					if (!Keyboard[SDL_SCANCODE_LEFT])
						value |= 0x02;
					if (!Keyboard[SDL_SCANCODE_RIGHT])
						value |= 0x01;
					if (!Keyboard[SDL_SCANCODE_DOWN])
						value |= 0x08;
				}
				else if ((value & 0x20) == 0) {
					if (!Keyboard[SDL_SCANCODE_W])
						value |= 0x04;
					if (!Keyboard[SDL_SCANCODE_B])
						value |= 0x02;
					if (!Keyboard[SDL_SCANCODE_A])
						value |= 0x01;
					if (!Keyboard[SDL_SCANCODE_1])
						value |= 0x08;
				}
				else {
					value |= 0x0f;
				}
				return value;
			}
			if (_handlerReadIO[address & 0xff])
				return _handlerReadIO[address & 0xff](address);
			return in(address & 0xff, caller);
		}
		else
			return _rams[address];
	return _rams[address];
}

#include <iostream>
bool Mmu::write(const uint16_t address, const uint8_t value, const origin caller) {
	// ROM cartridge
	if (address < MemoryMap::VRAM)
		return false;

	// VRAM
	if (address < MemoryMap::EXTRA_RAM) {
		if (_handlerWriteVRAM)
			_handlerWriteVRAM(address, value);
		return true;
	}

	// External RAM
	if (address < 0xc000)
		return _cartridge->write(address, value);

	// WRAM
//	if (address < 0xe000)
//		return memory->write(address, value);

	// Echo RAM
	if (address < MemoryMap::OAM) {
		_rams[address] = value;
		return true;
	}

	// OAM
	if (address < MemoryMap::NOTUSE) {
		if (_handlerWriteOAM)
			_handlerWriteOAM(address, value);
		return true;
	}

	if (address >= 0xff00) {
		if (address < 0xff80) {
//			if ((address >= 0xff10) && (address < 0xff27))
//				return _apu.write(address & 0xff, value);
			if (_handlerWriteIO[address & 0xff]) {
				_handlerWriteIO[address & 0xff](address, value);
				return true;
			}
			else
				return out(address & 0xff, value, caller);
		}
		if (address == 0xffff)
			std::cout << "out " << std::hex << "FFFF - " << (int)value << std::endl;

		_rams[address] = value;
		return true;
	}
	_rams[address] = value;
	return true;
}
