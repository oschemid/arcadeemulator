#include "memory.h"
#include <fstream>


ae::memory::memory() :
	data(nullptr),
	size(0)
{
}

void ae::memory::allocate(const uint16_t s) {
	size = s;
	data = new uint8_t[size + 1];
	memset(data, 0, size + 1);
}

ae::memory::~memory() {
	if (data)
		delete[] data;
}

bool ae::memory::load(const string& filename,
					  const uint16_t offset)
{
	std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
	if (!ifs)
		throw std::runtime_error("Unable to read " + filename);
	auto end = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	auto size = std::size_t(end - ifs.tellg());

	if (!ifs.read((char*)data + offset, size))
		throw std::runtime_error("Error during reading " + filename);
	return true;
}
