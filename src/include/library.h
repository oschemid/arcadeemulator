#pragma once
#include "types.h"
#include "tools.h"
#include <vector>
#include <map>
#include <utility>
#include "emulator.h"
#include "tools.h"


using aos::string;
using std::vector;
using std::map;
using std::pair;


namespace aos::library
{
	class Game
	{
	public:
		Game(const string n);

		string name() const { return _name; }
		string cover() const { return _cover; }
		void add(const aos::emulator::GameDriver&);
		void setCover(const string path) { _cover = path; }
		std::map<std::string, aos::emulator::GameDriver>& emulators() { return _emulators; }
		aos::emulator::GameDriver& driver() { return _emulators[_selected]; }
		void select(const string version) { _selected = version; }
		string selected() const { return _selected; }

	protected:
		string _name;
		string _selected;
		string _cover;
		std::map<std::string, aos::emulator::GameDriver> _emulators;
	};

	class Console
	{
	public:
		Console(const string&, const string&);

		string name() const { return _name; }
		string title() const { return _title; }

		void init();
		void add(const aos::emulator::GameDriver&);

	protected:
		string _name;
		string _title;
		aos::tools::RomManager _roms;

	public:
		std::map<string, Game*> games;

		void load();
	};

	class Consoles : public map<string, Console*>
	{
	public:
		void init();
		Console* get(const string&);

	protected:
		void addGames();
	};
	Consoles& getConsoles();
}
