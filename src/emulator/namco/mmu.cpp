#include "mmu.h"
#include <ranges>


using namespace aos::namco;


Mmu::mapping& Mmu::mapping::rom()
{
	_internalmemory = true;
	_readfn = [this](const uint16_t a) { return _memory[a]; };
	return* this;
}

Mmu::mapping& Mmu::mapping::ram()
{
	_internalmemory = true;
	_readfn = [this](const uint16_t a) { return _memory[a]; };
	_writefn = [this](const uint16_t a, const uint8_t v) { _memory[a] = v; };
	return *this;
}

Mmu::mapping& Mmu::mapping::readfn(std::function<uint8_t(const uint16_t)> readfn)
{
	_internalmemory = false;
	_readfn = readfn;
	return *this;
}

Mmu::mapping& Mmu::mapping::decodefn(std::function<uint8_t(const uint16_t, const uint8_t)> decodefn)
{
	_internalmemory = true;
	_decodingfn = decodefn;
	return *this;
}

Mmu::mapping& Mmu::mapping::writefn(std::function<void(const uint16_t, const uint8_t)> writefn)
{
	_internalmemory = false;
	_writefn = writefn;
	return *this;
}

void Mmu::mapping::init(const vector<aos::emulator::RomConfiguration>& roms)
{
	if (_internalmemory) {
		const uint16_t memorysize = _end - _start + 1;
		_memory = new uint8_t[memorysize];

		size_t offset = 0;
		for(const auto& rom : roms | std::ranges::views::filter([this](const auto i) { return i.region == _name; }))
		{
			offset += rom.rom.read(_memory + offset);
		}

		if (_decodingfn)
		{
			for (size_t offset = 0; offset < memorysize; ++offset)
			{
				_memory[offset] = _decodingfn(_start + offset, _memory[offset]);
			}
		}
	}
}

Mmu::mapping& Mmu::map(const uint16_t start, const uint16_t end)
{
	mapping map{ start, end };
	_mapping.push_front(map);
	return _mapping.front();
}

void Mmu::init(const vector<aos::emulator::RomConfiguration>& roms)
{
	for (auto& map : _mapping)
		map.init(roms);
}

uint8_t Mmu::read(const uint16_t address)
{
	if (_beforefn)
		_beforefn(address);
	for (auto& map : _mapping)
	{
		if (map.is_mapped(address, _bank_selected))
			return map.read(address);
	}
	return 0;
}

void Mmu::write(const uint16_t address, const uint8_t value)
{
	if (_beforefn)
		_beforefn(address);
	for (auto& map : _mapping)
	{
		if (map.is_mapped(address, _bank_selected)) {
			map.write(address, value);
		}
	}
}

