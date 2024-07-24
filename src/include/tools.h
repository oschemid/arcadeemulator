#pragma once
#include "types.h"
#include <filesystem>
#include <chrono>
#include <nlohmann/json.hpp>


using nlohmann::json;
using aos::string;


namespace aos::tools
{
	class Clock
	{
	public:
		Clock(const uint64_t);

		void reset();
		bool tickable();

	protected:
		uint64_t _clockPerMs{ 0 };
		uint64_t _clock{ 0 };
		uint64_t _clockCompensation{ 0 };
		uint64_t _lastThrottle{ 0 };
		std::chrono::steady_clock::time_point _startTime;
	};

	class Rom
	{
	public:
		string filename;
		string archive;

		size_t read(uint8_t*, const size_t = 0, const size_t = 0) const;
	};

	class RomManager
	{
	public:
		RomManager(const string&);

		void preload();
		Rom get(const json&) const;

	protected:
		string _path;
		std::map < size_t, std::map<uint32_t, Rom>> _roms;
		Rom find(const uint16_t, const uint32_t) const;

		void loadRomDirectory(const std::filesystem::directory_entry&);
		void loadRomFile(const std::filesystem::directory_entry&);
		void loadRomZip(const std::filesystem::directory_entry&);
	};
}

namespace aos::tools::json_helpers
{
	template<typename T> T get(const json& data)
	{
		if (data.is_number_integer())
			return data.template get<T>();
		if (data.is_string())
		{
			string d = data.template get<std::string>();
			return static_cast<T>(std::stoul(d, nullptr, 0));
		}
		return 0;
	}
}

