#include "amstradmmu.h"


using namespace aos::amstrad;


Memory::Memory()
{
}

void Memory::init(aos::mmu::RomMappings& roms)
{
	_rom1 = new uint8_t[0x4000];
	_rom2 = new uint8_t[0x4000];
	_romamsdos = new uint8_t[0x4000];
	_ram = new uint8_t[0x10000];

	for (auto& rom : roms)
	{
		if (rom.region == "os")
		{
			rom.rom.read(_rom1, 0, 0x4000);
		}
		if (rom.region == "basic")
		{
			rom.rom.read(_rom2, 0, 0x4000);
		}
		if (rom.region == "amsdos")
		{
			rom.rom.read(_romamsdos, 0, 0x4000);
		}
	}
}

Memory::~Memory()
{
	if (_ram)
		delete[] _ram;
	if (_romamsdos)
		delete[] _romamsdos;
	if (_rom1)
		delete[] _rom2;
	if (_rom2)
		delete[] _rom1;
}

uint8_t Memory::read(const uint16_t address) const
{
	switch (address & 0xc000)
	{
	case 0x0:
		return (_rom1_enabled) ? _rom1[address] : _ram[address];
	case 0x4000:
	case 0x8000:
		return _ram[address];
	case 0xc000:
		if (_rom2_enabled)
		{
			switch (_rom2_selected)
			{
			case 0:
				return _rom2[address - 0xc000];
			case 7:
				return _romamsdos[address - 0xc000];
			default:
				return 0;
			}
		}
		else
		{
			return _ram[address];
		}
	}
}

void Memory::write(const uint16_t address, const uint8_t value)
{
	_ram[address] = value;
}
