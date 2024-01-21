#pragma once
#include "types.h"
#include <filesystem>


using aos::string;


namespace aos::tools
{
	class Rom
	{
	public:
		size_t size;
		uint32_t crc32;
		string filename;
		string archive;

		size_t read(uint8_t*, const size_t = 0, const size_t = 0) const;
	};

	class RomManager
	{
	public:
		RomManager(const string&);

		void preload();
		Rom find(const uint16_t, const uint32_t) const;

	protected:
		string _path;
		std::vector<Rom> _roms;

		void loadRomDirectory(const std::filesystem::directory_entry&);
		void loadRomFile(const std::filesystem::directory_entry&);
		void loadRomZip(const std::filesystem::directory_entry&);
	};
}

namespace ae::filemanager
{
	void readRoms(const string&, std::vector<std::pair<uint16_t,string>>, uint8_t*);

	uint32_t Crc32(const uint8_t*, size_t);
}
