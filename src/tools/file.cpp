#include "types.h"
#include "tools.h"
#include "minizip-ng/unzip.h"
#include <filesystem>
#include <map>


using aos::string;
namespace fs = std::filesystem;


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
	Rom RomManager::find(const uint16_t size, const uint32_t crc) const
	{
		for (const auto& rom : _roms) {
			if ((rom.size == size) && (rom.crc32 == crc))
				return rom;
		}
		return Rom();
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
			rom.size = entry.file_size();
			rom.crc32 = ae::filemanager::Crc32(buffer, file.getSize());
			_roms.push_back(rom);
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
					rom.size = info.uncompressed_size;
					rom.crc32 = info.crc;
					_roms.push_back(rom);

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

bool aos::File::read(const uint16_t offset, const uint16_t size, const uint8_t* data) {
	if (offset + size > _size)
		throw std::runtime_error("Error in file size");
	_ifs.seekg(offset, std::ios::beg);
	_ifs.read((char*)data, size);
	return true;
}


size_t aos::tools::Rom::read(uint8_t* destination, const size_t start, const size_t expected_size) const
{
	const uint16_t effective_size = (expected_size) ? expected_size : size - start;
	if (archive.empty())
	{
		File file(filename);
		file.read(0, static_cast<uint16_t>(effective_size), destination);
		return effective_size;
	}
	else
	{
		size_t offset = 0;
		unzFile zipfile = unzOpen64(archive.c_str());
		if (!zipfile)
			throw std::runtime_error("Archive doesn't exist");
		unz_file_info64 info;
		if (unzLocateFile(zipfile, filename.c_str(), nullptr) == UNZ_END_OF_LIST_OF_FILE)
			throw std::runtime_error("Bad archive");
		if (unzOpenCurrentFile(zipfile) != UNZ_OK)
			throw std::runtime_error("Bad archive 2");
		unzGetCurrentFileInfo64(zipfile, &info, nullptr, 0, nullptr, 0, nullptr, 0);
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

void ae::filemanager::readRoms(const string&, std::vector<std::pair<uint16_t, string>>, uint8_t*)
{}