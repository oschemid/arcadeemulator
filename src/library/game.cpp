#include "library.h"


namespace aos::library
{
	Game::Game(const aos::string n) :
		_name{ n },
		_selected{ "Original" }
	{
	}

	void Game::add(const aos::emulator::GameDriver& g)
	{
		string version = (g.version.empty())? "Original" : g.version;
		_emulators.insert({ version, g });
	}
}
