#include "gameboy.h"


namespace aos::gameboy
{
	Core::Ptr createGameboy(const json& settings)
	{
		string rom = settings["rom"].get<string>();
		auto emul = std::make_unique<Gameboy>(rom);
		return emul;
	}

	static bool registering = CoreFactory::registerCore("gameboy", createGameboy);
}