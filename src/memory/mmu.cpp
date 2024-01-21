#include "memory.h"
#include <ranges>


using namespace aos::mmu;


Mmu::Mmu()
{

}

Mmu::~Mmu()
{

}

MemoryBank& Mmu::bank(const string name, const uint16_t size)
{
	if (_banks.find(name) == _banks.end()) {
		auto bank = _banks.emplace(name, MemoryBank(size));
		return bank.first->second;
	}
	throw std::invalid_argument("bank already exists");
}
MemoryBank& Mmu::bank(const string name)
{
	return _banks[name];
}

MemoryMap& Mmu::map(const uint16_t start, const uint16_t end, const string bank)
{
	_mappings.push_back({ MemoryMap(start, end), bank });
	return _mappings.back().first;
}

void Mmu::init(const RomMappings& roms)
{
	for (auto& [name, bank] : _banks)
		bank.init(roms | std::ranges::views::filter([name](const auto i) { return i.region == name; }) | std::ranges::to<std::vector>());
}

uint8_t Mmu::read(const uint16_t address)
{
//	if (_beforefn)
//		_beforefn(address);
	for (auto& [map, bank] : _mappings)
	{
		if (map.is_mapped(address)) // &&
			//		(map.is_readable()))
			return _banks[bank].read(map.map_address(address));
	}
	return 0xbf;
}

void Mmu::write(const uint16_t address, const uint8_t value)
{
	//if (_beforefn)
	//	_beforefn(address);
	for (auto& [map, bank] : _mappings)
	{
		if (map.is_mapped(address))
	//		(map.is_writable())) {
			_banks[bank].write(map.map_address(address), value);
	}
	//}
}
