#pragma once
#include "types.h"
#include "tools.h"
#include <list>
#include <map>
#include <utility>
#include "tools.h"
#include <ranges>
#include "nlohmann/json.hpp"

using aos::string;
using std::list;
using std::map;
using std::pair;
using nlohmann::json;

//
// This namespace contains all structures and functions to manage system lists and softwares 
//
namespace aos::database
{
	using PlatformCategory = enum { ARCADE, CONSOLE, COMPUTER };

	class Platform
	{
	public:
		Platform(const string&, const json&);

		const string& id() const { return _id; }
		const string& name() const { return _name; }
		const PlatformCategory category() const { return _category; }

	protected:
		string _id;
		string _name;
		PlatformCategory _category;
	};
	class Platforms : public map<string, Platform*>
	{
	public:
		static bool registering(const string&, Platform*);

		static auto list(const PlatformCategory category)
		{
			return singleton() | std::ranges::views::filter([category](std::pair<string, Platform*> p) { return p.second->category() == category; });
		}
		Platform* get(const string&);

	protected:
		Platforms() = default;
		static Platforms& singleton();
	};

	class DriverVersion
	{
	public:
		string core;
		json settings;
	};

	class Driver
	{
	public:
		Driver(const string&, const string&, const string&);

		const string& id() const { return _id; }
		const string& name() const { return _name; }
		const string& platform() const { return _platform; }

		uint16_t year() const { return _year; }
		void year(const uint16_t year) { _year = year; }
		const string& overview() const { return _overview; }
		void overview(const string& overview) { _overview = overview; }

		const string& version() const { return _version; }
		void version(const string& id) { _version = id; }

		std::vector<std::pair<string,string>> versions() const { return _versions; }
		DriverVersion& emulator() { auto it = _emulations.find(_version); return it->second; }

		void addVersion(const string& version, DriverVersion emulation)
		{
			_emulations.insert({ version, emulation });
			_versions.push_back({ version, emulation.settings["name"]}); sort(_versions.begin(), _versions.end(), [](const std::pair<string, string> a, const std::pair<string, string> b) { return a.second<b.second; });
		}

	protected:
		string _id;
		string _name;
		string _platform;
		uint16_t _year;
		string _overview;
		string _version;
		std::map<string, DriverVersion> _emulations;
		std::vector<std::pair<string, string>> _versions;
	};
	class Drivers : public map<string, Driver*>
	{
	public:
		static bool registering(const string&, Driver*);

		static auto list(const string& platform)
		{
			return singleton() | std::ranges::views::filter([platform](std::pair<string, Driver*> p) { return p.second->platform() == platform; });
		}
		Driver* get(const string&);

	protected:
		Drivers() = default;
		static Drivers& singleton();
	};

	// Database Initialisation
	bool init();

	bool load(const string&);

	// List functions
	inline vector<PlatformCategory> listPlatformCategories()
	{
		return { ARCADE, CONSOLE, COMPUTER };
	}
	inline auto listPlatforms(const PlatformCategory category)
	{
		return Platforms::list(category);
	}
	inline auto listDrivers(const Platform& platform)
	{
		return Drivers::list(platform.id());
	}

	// Get functions
	inline string get(const PlatformCategory category)
	{
		switch (category)
		{
		case ARCADE:
			return "Arcade";
		case CONSOLE:
			return "Console";
		case COMPUTER:
			return "Computer";
		}
	}

	class Game
	{
	public:
		Game(const string n);

		string name() const { return _name; }
		string cover() const { return _cover; }
//		void add(const aos::Driver&);
		void setCover(const string path) { _cover = path; }
//		std::map<std::string, aos::Driver>& emulators() { return _emulators; }
//		aos::Driver& driver() { return _emulators[_selected]; }
		void select(const string version) { _selected = version; }
		string selected() const { return _selected; }

	protected:
		string _name;
		string _selected;
		string _cover;
//		std::map<std::string, aos::Driver> _emulators;
	};

	class System
	{
	public:
		using systemtype = enum
		{
			ARCADE,
			CONSOLE,
			COMPUTER
		};
		System(const string&, const systemtype, const string&);

		string name() const { return _name; }
		systemtype type() const { return _type; }
		string title() const { return _title; }

		void init();
//		void add(const aos::Driver&);

	protected:
		string _name;
		systemtype _type;
		string _title;
		aos::tools::RomManager _roms;

	public:
		std::map<string, Game*> games;

		void load();
	};

	class Consoles : public map<string, System*>
	{
	public:
		void init();
		System* get(const string&);

	protected:
//		void addGames();
	};
	Consoles& getConsoles();
}
