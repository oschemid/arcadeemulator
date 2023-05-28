#include "library.h"


namespace aos::library
{
	Games& getGames()
	{
		static Games _games;
		return _games;
	}

	Game::Game(const aos::string n,
		aos::emulator::GameDriver e) :
		_name{ n },
		_emulator{ e }
	{
		getGames().insert({ n, e });
	}
}
