#pragma once
#include "types.h"
#include "tools.h"
#include "display.h"
#include "tilemap.h"
#include <vector>
#include <map>
#include <algorithm>


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
	struct RomConfiguration
	{
		string region;
		uint16_t start;
		aos::tools::Rom rom;
	};
	using RomsConfiguration = vector<RomConfiguration>;

	class Emulator
	{
	public:
		using Ptr = std::unique_ptr<Emulator>;

	public:
		virtual ~Emulator() = default;
		virtual SystemInfo getSystemInfo() const = 0;
		virtual void init(aos::display::RasterDisplay*) = 0;
		virtual uint8_t tick() = 0;
		void run();

		// Debug methods
		virtual std::map<string, aos::tilemap::Tiles> getTiles() const { return std::map<string, aos::tilemap::Tiles>(); }
		virtual std::vector<palette_t> getPalettes() const { return std::vector<palette_t>(); }

	protected:
		Emulator() = default;
		uint64_t _clockPerMs{ 0 };
	};

	using creator_fn = std::function<Emulator::Ptr(const GameConfiguration&, const RomsConfiguration&)>;

	struct GameDriver
	{
		string name;
		string version;
		bool main_version{ false };
		string emulator;
		creator_fn creator;
		RomsConfiguration roms;
		GameConfiguration configuration;

		bool has_configuration() const { return configuration.switches.size() > 0; }
		bool is_unavailable() const { return std::any_of(roms.begin(), roms.end(), [](const RomConfiguration& r) { return r.rom.filename.empty(); }); }
	};
}