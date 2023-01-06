#include "mmu.h"
#include "SDL2/SDL.h"


using namespace ae::gameboy;


Mmu::Mmu(std::shared_ptr<BootRom>& bootrom,
		 std::shared_ptr<Mbc>& cartridge)
	: _bootrom(bootrom),
	  _cartridge(cartridge),
	  _div(0) {
	_rams = new uint8_t[0xffff](0);
}

Mmu::~Mmu() {
	delete[] _rams;
}

void Mmu::tick() {
	_div++;
	if ((_div & 0x1fff) == 0)
		notify(io::div, _div >> 8);
}

void Mmu::notify(const uint8_t io, const uint8_t value) const {
	for (auto& cb : _io_callbacks)
		cb(io, value);
}

uint8_t Mmu::in(const uint8_t io, const origin caller) const {
	switch (io) {
	case io::div:
		return _div >> 8;
	default:
		if (io<0x80)
			return _rams[0xff00 + io];
		throw std::runtime_error("Bad io reading");
	}
}

bool Mmu::out(const uint8_t io, const uint8_t value, const origin caller) {
	switch (io) {
	case io::div:
		_div = 0;
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
	if (address < 0x8000) {
		if ((address < 0x100) && (in(io::ff50, caller) == 0))
			return _bootrom->read(address);
		return _cartridge->read(address);
	}

	// VRAM
	if (address < 0xa000)
		return _rams[address];

	// External RAM
	if (address < 0xc000)
		return _cartridge->read(address);

	// WRAM
//	if (address < 0xe000)
//		return memory->read(address);

	// Echo RAM
//	if (address < 0xfe00)
//		return memory->read(address - 0x2000);

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
			return in(address & 0xff, caller);
		}
		else
			return _rams[address];
	return _rams[address];
}

bool Mmu::write(const uint16_t address, const uint8_t value, const origin caller) {
	// ROM cartridge
	if (address <= 0x7fff)
		return false;

	// VRAM
	if (address < 0xa000) {
		_rams[address] = value;
		return true;
	}

	// External RAM
	if (address < 0xc000)
		return _cartridge->write(address, value);

	// WRAM
//	if (address < 0xe000)
//		return memory->write(address, value);

	// Echo RAM
//	if (address < 0xfe00)
//		return memory->write(address, value);

	if (address >= 0xff00) {
		if (address < 0xff80) {
//			if ((address >= 0xff10) && (address < 0xff27))
//				return _apu.write(address & 0xff, value);
			return out(address & 0xff, value, caller);
		}
		_rams[address] = value;
		return true;
	}
	_rams[address] = value;
	return true;
}
