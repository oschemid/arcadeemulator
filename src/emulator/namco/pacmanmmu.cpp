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

Mmu::mapping& Mmu::mapping::decodefn(std::function<void(uint8_t*, const size_t)> decodefn)
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

void Mmu::mapping::init(const vector<aos::mmu::RomMapping>& roms)
{
	if (_internalmemory) {
		const uint16_t memorysize = _end - _start + 1;
		_memory = new uint8_t[memorysize];

		size_t offset = 0;
		for(const auto& rom : roms | std::ranges::views::filter([this](const auto i) { return i.region == _name; }))
		{
			offset += rom.rom.read(_memory + offset, rom.mapping.start, rom.mapping.size);
		}

		if (_decodingfn)
			_decodingfn(_memory, memorysize);
	}
}

Mmu::mapping& Mmu::map(const uint16_t start, const uint16_t end)
{
	mapping map{ start, end };
	_mapping.push_front(map);
	return _mapping.front();
}

void Mmu::init(const vector<aos::mmu::RomMapping>& roms)
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
		if ((map.is_mapped(address, _bank_selected)) &&
			(map.is_readable()))
			return map.read(address);
	}
	return 0xbf;
}

void Mmu::write(const uint16_t address, const uint8_t value)
{
	if (_beforefn)
		_beforefn(address);
	for (auto& map : _mapping)
	{
		if ((map.is_mapped(address, _bank_selected)) &&
			(map.is_writable())) {
			map.write(address, value);
		}
	}
}

void Mmu::patch(const uint16_t address, const uint8_t value)
{
	for (auto& map : _mapping)
	{
		if ((map.is_mapped(address, _bank_selected)))
			map.patch(address, value);
	}
}
