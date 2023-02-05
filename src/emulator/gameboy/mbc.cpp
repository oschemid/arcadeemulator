#include "Mbc.h"


using namespace ae::gameboy;


BootRom::BootRom(const string& filename) {
	File file(filename);
	file.read(0, 0x0100, _rom);
}

Mbc::Mbc(const uint16_t rom_size,
	     const uint16_t ram_size):
	_rom_size(rom_size),
	_ram_size(ram_size) {
	_rom = new uint8_t[_rom_size];
	_ram = new uint8_t[_ram_size];
}

Mbc::~Mbc() {
	delete[] _rom;
	delete[] _ram;
}

uint8_t Mbc::read(const uint16_t address) const {
	if (address < 0x8000)
		return _rom[address];
	if (address < 0xa000)
		return 0;
	if (address < 0xa000 + _ram_size)
		return _ram[address - 0xa000];
	return 0;
}

bool Mbc::write(const uint16_t address, const uint8_t value) {
	if (address < 0xa000)
		return false;
	if (address < 0xa000 + _ram_size) {
		_ram[address - 0xa000] = value;
		return true;
	}
	return false;
}

Mbc* Mbc::create(const string& filename) {
	Mbc* mbc = new Mbc(0x8000, 0);
	File file(filename);
	file.read(0, 0x8000, mbc->_rom);
	return mbc;
}

void Mbc::decode_header() {
	_header.title = _rom[0x134];
}
