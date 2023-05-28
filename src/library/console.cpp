#include "library.h"


using namespace aos::library;


Consoles& aos::library::getConsoles()
{
	static Consoles _consoles;
	return _consoles;
}

Console* aos::library::getConsole(const string name)
{
	auto consoles = getConsoles();
	auto it = consoles.find(name);
	if (it != consoles.end())
		return it->second;
	throw std::runtime_error("Unknown console");
}

Console::Console(const string n, const string t) :
	name{ n },
	title{ t }
{
	getConsoles().insert({ n, this });
}

void Console::add(const Game& g)
{
	games.push_back(g);
}
