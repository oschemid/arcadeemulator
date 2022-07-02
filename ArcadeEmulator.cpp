// ArcadeEmulator.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>

#include "src/spaceinvaders.h"
#include "src/ui/ui.h"


int main(int argc, char** argv)
{
	// UI creation
	if (!ae::ui::init())
		return 1;

	// Machine selection
	ae::spaceinvaders si;
	si.init();

	ae::ui::menu my_menu;
	ae::ui::menu::response r = ae::ui::menu::response::NOTHING;

	while (r != ae::ui::menu::response::QUIT) {
		r = my_menu.run();
		switch (r) {
		case ae::ui::menu::response::LAUNCH:
			si.run();
			break;
		case ae::ui::menu::response::SETTINGS:
			std::cout << "Settings";
			break;
		}
	}
	ae::ui::destroy();
	return 0;
}
