#include "memory.h"
#include <fstream>


ae::memory::memory()
{
	memset(data, 0, 0x4000);
}


bool ae::memory::load(const string& filename,
					  const uint16_t offset)
{
	std::ifstream ifs(filename, std::ios::binary | std::ios::ate);
	if (!ifs)
		throw std::runtime_error("Unable to read "+filename);
	auto end = ifs.tellg();
	ifs.seekg(0, std::ios::beg);
	auto size = std::size_t(end - ifs.tellg());

	if (!ifs.read((char*)data + offset, size))
		throw std::runtime_error("Error during reading "+filename);
	return true;
}
