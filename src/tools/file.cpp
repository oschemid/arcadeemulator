#include "types.h"


ae::File::File(const string& filename) :
	_ifs(filename, std::ios::binary | std::ios::ate) {
	auto end = _ifs.tellg();
	_ifs.seekg(0, std::ios::beg);
	_size = std::size_t(end - _ifs.tellg());
}

ae::File::~File() {
	_ifs.close();
}

bool ae::File::read(const uint16_t offset, const uint16_t size, const uint8_t* data) {
	if (offset + size > _size)
		throw std::runtime_error("Error in file size");
	_ifs.seekg(offset, std::ios::beg);
	_ifs.read((char*)data, size);
	return true;
}
