// ArcadeEmulator.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>

#include "machine.h"
#include "src/machine/spaceinvaders.h"
#include "src/ui/ui.h"


int main(int argc, char** argv)
{
	// UI creation
	if (!ae::ui::init())
		return 1;

	ae::IMachine* si = nullptr;

	ae::ui::menu::response r = ae::ui::menu::response::NOTHING;

	while (r != ae::ui::menu::response::QUIT) {
		ae::ui::menu my_menu;
		r = my_menu.run(si);
		switch (r) {
		case ae::ui::menu::response::LAUNCH:
			si->init();
			si->run();
			break;
		case ae::ui::menu::response::GAMESELECTION:
		{
			ae::ui::InterfaceGameSelection i((si == nullptr) ? "" : si->getID());
			i.run();
			if (i.getSelection() != "")
				si = ae::newMachine(i.getSelection());
		}
		break;
		}
	}
	ae::ui::destroy();
	return 0;
}

