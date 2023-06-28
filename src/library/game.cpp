#include "library.h"


namespace aos::library
{
	Game::Game(const aos::string n) :
		_name{ n },
		_selected{ "###"}
	{
	}

	void Game::add(const aos::emulator::GameDriver& g)
	{
		string version = (g.version.empty())? "Original" : g.version;
		_emulators.insert({ version, g });
		if ((_selected == "###") || g.main_version)
			_selected = version;
	}
}
