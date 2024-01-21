#include "memory.h"

using namespace aos::mmu;


MemoryBank::~MemoryBank()
{
	if (_memory)
		delete[] _memory;
	_memory = nullptr;
}

MemoryBank& MemoryBank::rom()
{
	_managed = true;
	_readfn = [this](const uint16_t a) { return _memory[a]; };
	_writefn = nullptr;
	return *this;
}

MemoryBank& MemoryBank::ram()
{
	_managed = true;
	_readfn = [this](const uint16_t a) { return _memory[a]; };
	_writefn = [this](const uint16_t a, const uint8_t v) { _memory[a] = v; };
	return *this;
}

void MemoryBank::init(const RomMappings& roms)
{
	if (_managed)
	{
		_memory = new uint8_t[_size]{ 0 };
		load(roms);
	}
}

void MemoryBank::load(const RomMappings& roms)
{
	size_t offset = 0;
	for (const auto& rom : roms)
		offset += rom.rom.read(_memory + offset, rom.mapping.start, rom.mapping.size);
}
