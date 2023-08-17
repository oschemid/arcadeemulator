#include "mmu.h"
#include <ranges>


using namespace aos::namco;


Mmu::~Mmu()
{
	for (auto& [_, mapping] : _mapping)
	{
		if (mapping->mymemory)
			delete[] mapping->mymemory;
	}
}

void Mmu::mapping::rom()
{
	auto map = _map;
	map->delegated = false;
	map->myreadfn = [map](const uint16_t a) { return map->mymemory[a]; };
}

void Mmu::mapping::ram()
{
	auto map = _map;
	map->delegated = false;
	map->myreadfn = [map](const uint16_t a) { return map->mymemory[a]; };
	map->mywritefn = [map](const uint16_t a, const uint8_t v) { map->mymemory[a] = v; };
}

void Mmu::mapping::shared(std::function<uint8_t(const uint16_t)> readfn, std::function<void(const uint16_t, const uint8_t)> writefn)
{
	_map->myreadfn = readfn;
	_map->mywritefn = writefn;
}

Mmu::mapping& Mmu::mapping::readfn(std::function<uint8_t(const uint16_t)> readfn)
{
	_map->myreadfn = readfn;
	return *this;
}

Mmu::mapping& Mmu::mapping::writefn(std::function<void(const uint16_t, const uint8_t)> writefn)
{
	_map->mywritefn = writefn;
	return *this;
}

Mmu::mapping Mmu::map(const uint16_t start, const uint16_t end, const uint16_t mirror, const string name)
{
	memorymap mapping = { start, end, mirror };
	memory page = { true, 0, nullptr, nullptr, nullptr };
	size_t index = _mapping.size();
	_mapping.push_back({ mapping, std::make_shared<memory>(page) });
	if (!name.empty())
		_naming[name] = index;
	return Mmu::mapping(_mapping[index].second);
}

Mmu::mapping Mmu::map(const uint16_t start, const uint16_t end, const string name)
{
	return map(start, end, 0xffff, name);
}
Mmu::mapping Mmu::map(const uint16_t start, const uint16_t end, const uint16_t mirror)
{
	return map(start, end, mirror, "");
}

Mmu::mapping Mmu::map(const uint16_t start, const uint16_t end)
{
	return map(start, end, 0xffff, "");
}

void Mmu::init(const vector<aos::emulator::RomConfiguration>& roms)
{
	for (auto& [mapname, mapstruct] : _mapping)
	{
		if (mapstruct->delegated)
			continue;
		const uint16_t memorysize = mapname.end - mapname.start + 1;
		mapstruct->mymemory = new uint8_t[memorysize];
		mapstruct->size = memorysize;
	}
	for (auto [mapname, index] : _naming)
	{
		uint8_t* const memory = _mapping[index].second->mymemory;
		size_t offset = 0;
		for (const auto& rom : roms | std::ranges::views::filter([mapname](const auto i) { return i.region == mapname; })) {
			offset += rom.rom.read(memory + offset);
		}
	}
}

uint8_t Mmu::read(const uint16_t address)
{
	for (auto& [mapping, memory] : _mapping)
	{
		const uint16_t address_mirror = address & mapping.mirroring;
		if ((address_mirror >= mapping.start) && (address_mirror <= mapping.end)) {
			if (memory->myreadfn)
				return memory->myreadfn(address_mirror - mapping.start);
		}
	}
	return 0;
}

void Mmu::write(const uint16_t address, const uint8_t value)
{
	for (auto& [mapping, memory] : _mapping)
	{
		const uint16_t address_mirror = address & mapping.mirroring;
		if ((address_mirror >= mapping.start) && (address_mirror <= mapping.end)) {
			if (memory->mywritefn) {
				memory->mywritefn(address_mirror - mapping.start, value);
				return;
			}
		}
	}
}

