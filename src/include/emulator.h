#pragma once
#include "types.h"
#include "display.h"
#include "tilemap.h"
#include <vector>
#include <map>

using aos::string;
using aos::geometry_t;
using std::vector;
using std::pair;


namespace aos::emulator
{
	struct SystemInfo
	{
		geometry_t geometry;
	};
	struct DipSwitch
	{
		string name;
		uint8_t value;
		string description;
		vector<string> values;
	};
	struct GameConfiguration
	{
		vector<DipSwitch> switches;
	};

	class Emulator
	{
	public:
		using Ptr = std::unique_ptr<Emulator>;

	public:
		virtual ~Emulator() = default;
		virtual SystemInfo getSystemInfo() const = 0;
		virtual void init(ae::display::RasterDisplay*) = 0;
		virtual uint8_t tick() = 0;
		void run();

		// Debug methods
		virtual std::map<string, ae::tilemap::Tiles> getTiles() const { return std::map<string, ae::tilemap::Tiles>(); }
		virtual std::vector<palette_t> getPalettes() const { return std::vector<palette_t>(); }

	protected:
		Emulator() = default;
		uint64_t _clockPerMs{ 0 };
	};

	using creator_fn = std::function<Emulator::Ptr(const GameConfiguration&)>;

	struct GameDriver
	{
		string name;
		string emulator;
		creator_fn creator;
		vector<pair<uint16_t, string>> roms;
		GameConfiguration configuration;
	};
}