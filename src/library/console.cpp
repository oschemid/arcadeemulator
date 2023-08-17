#include "library.h"
#include "registry.h"
#include <fstream>
#include "nlohmann/json.hpp"


using namespace aos::library;
using json = nlohmann::json;


namespace aos::library
{
	Consoles& getConsoles()
	{
		static Consoles _consoles;
		return _consoles;
	}

	Console* Consoles::get(const string& name)
	{
		auto it = find(name);
		if (it != end())
			return it->second;
		throw std::runtime_error("Unknown console");
	}

	void Consoles::init()
	{
		addGames();
		for (auto& console : *this)
			console.second->init();
	}

	void Consoles::addGames()
	{
		for (auto& [_, driver] : aos::Registry<aos::emulator::GameDriver>::instance().get())
		{
			try
			{
				auto console = get(driver.emulator);
				if (console)
				{
					console->add(driver);
				}
			}
			catch (std::exception&)
			{
			}
		}
	}

	Console::Console(const string& n, const string& t) :
		_name{ n },
		_title{ t },
		_roms{ "roms/" + _name }
	{
		getConsoles().insert({ n, this });
	}

	void Console::add(const aos::emulator::GameDriver& g)
	{
		string gamename = g.name;
		auto it = games.find(gamename);
		if (it == games.end())
			games[gamename] = new Game(gamename);
		games[gamename]->add(g);
	}

	void Console::init()
	{
		_roms.preload();
		for (auto& game : games) {
			for (auto& driver : game.second->emulators()) {
				for(auto& rom: driver.second.roms)
				{ 
					aos::tools::Rom romconfig = _roms.find(rom.rom.size, rom.rom.crc32);
					rom.rom.filename = romconfig.filename;
					rom.rom.archive = romconfig.archive;
				}
			}
		}
		load();
	}

	void Console::load()
	{
		std::ifstream f("data/" + _name + ".json");
		if (!f.fail())
		{
			json data = json::parse(f);
			for (auto& game : data)
			{
				string gamename = game["game"];
				string coverpath = game["cover"];
				auto driver = games[gamename];
				driver->setCover(coverpath);
			}
		}
	}
}