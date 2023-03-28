#include "types.h"
#include "file.h"
#include "minizip-ng/unzip.h"


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


void ae::filemanager::readRoms(const string& filename, std::vector<std::pair<uint16_t,string>> mapping, uint8_t* destination)
{
	unzFile zipfile = unzOpen64(filename.c_str());
	if (!zipfile)
		throw std::runtime_error("Archive doesn't exist");

	size_t offset = 0;
	for (auto map : mapping)
	{
		const string f = map.second;
		const uint16_t initial_offset = map.first;
		if (initial_offset > 0)
			offset = initial_offset;
		unz_file_info64 info;
		if (unzLocateFile(zipfile, f.c_str(), nullptr) == UNZ_END_OF_LIST_OF_FILE)
			throw std::runtime_error("Bad archive");
		if (unzOpenCurrentFile(zipfile) != UNZ_OK)
			throw std::runtime_error("Bad archive 2");
		unzGetCurrentFileInfo64(zipfile, &info, nullptr,0,nullptr,0,nullptr,0);
		int error = UNZ_OK;
		do
		{
			error = unzReadCurrentFile(zipfile, destination + offset, info.uncompressed_size);
			offset += error;
		} while (error > 0);
		unzCloseCurrentFile(zipfile);
	}
	unzClose(zipfile);
}
