#include "types.h"
#include "tools.h"
#include "minizip-ng/unzip.h"
#include <filesystem>
#include <map>


using aos::string;
namespace fs = std::filesystem;


const uint32_t Polynomial = 0xEDB88320;
constexpr auto Crc32Lookup{ []() constexpr {
	std::array<uint32_t,256> table{0};
	for (unsigned int i = 0; i <= 0xFF; i++)
	{
		uint32_t crc = i;
		for (unsigned int j = 0; j < 8; j++)
			crc = (crc >> 1) ^ (-int(crc & 1) & Polynomial);
		table[i] = crc;
	}
	return table;
	}()
};

uint32_t Crc32(const uint8_t* data, size_t length)
{
	uint32_t crc = 0xFFFFFFFF;
	for (size_t i = 0; i < length; i++)
		crc = (crc >> 8) ^ Crc32Lookup[(crc & 0xFF) ^ data[i]];
	return crc ^ 0xFFFFFFFF;
}


namespace aos::tools
{
	RomManager::RomManager(const string& path) :
		_path(path) 
	{
	}

	void RomManager::preload()
	{
		_roms.clear();
		loadRomDirectory(fs::directory_entry(_path));
	}
	Rom RomManager::get(const json& config) const
	{
		uint16_t size = json_helpers::get<uint16_t>(config.at(0));
		uint32_t crc = json_helpers::get<uint32_t>(config.at(1));
		return find(size, crc);
	}
	Rom RomManager::find(const uint16_t size, const uint32_t crc) const
	{
		auto it1 = _roms.find(size);
		if (it1 == _roms.end())
			return Rom();
		auto tmp = it1->second;
		auto it2 = tmp.find(crc);
		if (it2 == tmp.end())
			return Rom();
		return it2->second;
	}
	void RomManager::loadRomDirectory(const fs::directory_entry& entry)
	{
		for (const auto& entry : fs::directory_iterator(entry.path())) {
			if (entry.is_regular_file())
				loadRomFile(entry);
			else if (entry.is_directory())
				loadRomDirectory(entry);
		}
	}
	void RomManager::loadRomFile(const fs::directory_entry& entry)
	{
		if (entry.path().extension() == ".zip")
		{
			loadRomZip(entry);
		}
		else
		{
			File file(entry.path().string());
			uint8_t* buffer = new uint8_t[file.getSize()];
			file.read(0,file.getSize(),buffer);
			Rom rom;
			rom.filename = entry.path().string();
			size_t size = entry.file_size();
			uint32_t crc32 = Crc32(buffer, file.getSize());
			auto it1 = _roms.find(size);
			if (it1 == _roms.end())
			{
				_roms.insert({ size, {{crc32, rom}} });
			}
			else
			{
				it1->second.insert({ crc32, rom });
			}
			delete[] buffer;
		}
	}
	void RomManager::loadRomZip(const fs::directory_entry& entry)
	{
		unzFile zipfile = unzOpen64(entry.path().string().c_str());
		if (!zipfile)
			throw std::runtime_error("Archive doesn't exist");
		if (unzGoToFirstFile(zipfile) == UNZ_OK) {
			do {
				if (unzOpenCurrentFile(zipfile) == UNZ_OK) {
					unz_file_info64 info;
					unzGetCurrentFileInfo64(zipfile, &info, nullptr, 0, nullptr, 0, nullptr, 0);
					char* filename = new char[info.size_filename + 1];
					unzGetCurrentFileInfo64(zipfile, &info, filename, info.size_filename+1, nullptr, 0, nullptr, 0);
					Rom rom;
					rom.archive = entry.path().string();
					rom.filename = filename;

					size_t size = info.uncompressed_size;
					uint32_t crc32 = info.crc;

					auto it1 = _roms.find(size);
					if (it1 == _roms.end())
					{
						_roms.insert({ size, {{crc32, rom}} });
					}
					else
					{
						it1->second.insert({ crc32, rom });
					}

					delete[] filename;
					unzCloseCurrentFile(zipfile);
				}
			} while (unzGoToNextFile(zipfile) == UNZ_OK);
		}
		unzClose(zipfile);
	}
}


aos::File::File(const string& filename) :
	_ifs(filename, std::ios::binary | std::ios::ate) {
	auto end = _ifs.tellg();
	_ifs.seekg(0, std::ios::beg);
	_size = std::size_t(end - _ifs.tellg());
}

aos::File::~File() {
	_ifs.close();
}

bool aos::File::read(const size_t offset, const size_t size, const uint8_t* data) {
	if (offset + size > _size)
		throw std::runtime_error("Error in file size");
	_ifs.seekg(offset, std::ios::beg);
	_ifs.read((char*)data, size);
	return true;
}


size_t aos::tools::Rom::read(uint8_t* destination, const size_t start, const size_t expected_size) const
{
	size_t effective_size = 0;
	if (archive.empty())
	{
		File file(filename);
		effective_size = (expected_size) ? expected_size : file.getSize() - start;
		file.read(0, effective_size, destination);
		return effective_size;
	}
	else
	{
		size_t offset = 0;
		unzFile zipfile = unzOpen64(archive.c_str());
		if (!zipfile)
			throw std::runtime_error("Archive doesn't exist");
		unz_file_info64 info;
		if (unzLocateFile(zipfile, filename.c_str(), 1) == UNZ_END_OF_LIST_OF_FILE)
			throw std::runtime_error("Bad archive");
		if (unzOpenCurrentFile(zipfile) != UNZ_OK)
			throw std::runtime_error("Bad archive 2");
		unzGetCurrentFileInfo64(zipfile, &info, nullptr, 0, nullptr, 0, nullptr, 0);
		effective_size = (expected_size) ? expected_size : info.uncompressed_size - start;
		uint8_t* buffer = destination;
		uint16_t loaded_size = effective_size + start;
		if (loaded_size > info.uncompressed_size)
			loaded_size = info.uncompressed_size;

		if (start > 0) {
			buffer = new uint8_t[loaded_size];
		}
		int error = UNZ_OK;
		do
		{
			error = unzReadCurrentFile(zipfile, buffer + offset, static_cast<uint32_t>(loaded_size));
			offset += error;
		} while ((error > 0) && (offset < loaded_size));
		unzClose(zipfile);

		if (start > 0) {
			for (size_t i = 0; i < effective_size; ++i)
				destination[i] = buffer[start + i];
			delete[] buffer;
		}
	}
	return effective_size;
}
