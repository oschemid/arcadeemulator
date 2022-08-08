#include "memory.h"
#include <vector>
#include <fstream>


using namespace ae;


class Memory : public IMemory
{
protected:
	uint8_t* data;
	size_t size;
	std::vector<std::pair<std::pair<uint16_t, uint16_t>, IMemory::type>> mapping;

public:
	Memory(const size_t);
	~Memory();
	const uint8_t read(const uint16_t) const override;
	bool write(const uint16_t, const uint8_t) override;
	bool map(const uint16_t, const uint16_t, type) override;
	bool load(const uint16_t, const string) override;
};

Memory::Memory(const size_t s) :
	size(s) {
	data = new uint8_t[size](0);
}

Memory::~Memory() {
	delete[] data;
}

const uint8_t Memory::read(const uint16_t address) const {
	if (address < size)
		return data[address];
	return 0;
}

bool Memory::write(const uint16_t address, const uint8_t value) {
	if (address >= size)
		return false;
	for (const auto& [range, memoryType] : mapping) {
		if ((address >= range.first) && (address <= range.second)) {
			if (memoryType == IMemory::type::RAM) {
				data[address] = value;
				return true;
			}
			return false;
		}
	}
	return false;
}

bool Memory::map(const uint16_t start, const uint16_t end, type memoryType) {
	for (const auto& [range, memoryType] : mapping) {
		if ((start <= range.second) && (end >= range.first))
			return false;
	}
	mapping.push_back({ {start, end}, memoryType });
	return true;
}

bool Memory::load(const uint16_t offset, const string filename) {
	std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
	if (!ifs)
		throw std::runtime_error("Unable to read " + filename);
	auto end = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	auto filesize = std::size_t(end - ifs.tellg());

	if (offset + filesize >= size)
		throw std::runtime_error("Error in file size");

	if (!ifs.read((char*)data + offset, filesize))
		throw std::runtime_error("Error during reading " + filename);
	return true;
}

IMemory* ae::newMemory(const uint16_t size) {
	return new Memory((size == 0) ? 0x10000 : size);
}
