#pragma once
#include "types.h"
#include <vector>
#include <map>
#include <utility>
#include "emulator.h"


using aos::string;
using std::vector;
using std::map;
using std::pair;


namespace aos::library
{
	class Game
	{
	public:
		Game(const string n,
			aos::emulator::GameDriver e);

		string name() const { return _name; }
		aos::emulator::GameDriver& driver() { return _emulator; }
	protected:
		string _name;
		aos::emulator::GameDriver _emulator;
	};

	using Games = map<string, aos::emulator::GameDriver>;

	Games& getGames();

	struct Console
	{
		string name;
		string title;
		std::vector<Game> games;

		Console(const string, const string);
		void add(const Game&);
	};

	using Consoles = map<string, Console*>;

	Consoles& getConsoles();
	Console* getConsole(const string);
}
