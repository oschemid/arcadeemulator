// ArcadeEmulator.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>

#include "emulator.h"
#include "src/gui/vulkan/engine.h"
#include "src/gui/gui.h"
#include "src/gui/widgets.h"
#include <fstream>
#include "src/settings/library.h"
#include "src/settings/console.h"
#include "imgui_impl_sdl.h"

#include "src/emulator/gameboy/debugger/tilemap.h"
#include "src/emulator/gameboy/gameboy.h"

int main(int argc, char** argv)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		std::cout << "SDL could not be initialized: " << SDL_GetError();
		return false;
	}
	ae::Consoles consoles;
	consoles.load("data/consoles.json");
	ae::Library library;
	library.load("data/library.json");

	ae::gui::Window window("Arcade Emulation", 1280, 800);
	ae::gui::Engine engine(&window);
	ae::gui::GuiManager gui(&engine);

	ae::AppSidebar sidebar(consoles);
	ae::GameSelection gameselection(library);
	gui.addWidget("sidebar", &sidebar);
	gui.addWidget("gameselection", &gameselection);

	window.init();
	engine.init();
	gui.init();
	std::thread* t = nullptr;
	std::thread* t2 = nullptr;
	bool done = false;
	ae::emulator::Emulator::Ptr si = nullptr;
	ae::gui::RasterDisplay* raster = nullptr;
	ae::gui::RasterDisplay* raster2 = nullptr;
	ae::gameboy::debug::Debugger* debugger = nullptr;
	std::string n1 = "TEST";
	std::string n2 = "TILEMAP";
	ae::RasterDisplay* r1;
	ae::RasterDisplay* r2;

	while (!done)
	{
		done = gui.processEvent();
		gameselection.setFiltered(sidebar.getSelected());
		if (gameselection.getSelected().title() != "") {
			if (!si) {
				si = ae::emulator::Emulator::create(sidebar.getSelected().id());
				si->init(gameselection.getSelected().settings());
				ae::emulator::SystemInfo requirements = si->getSystemInfo();
				raster = engine.getRasterDisplay();
				raster->init(requirements.geometry.width, requirements.geometry.height);
				r1 = new ae::RasterDisplay(n1, raster->getID());
				gui.addWidget("rasterdisplay", r1);
				t = new std::thread([&si, &t, &raster]() { si->run(raster); t->detach(); });

				//raster2 = engine.getRasterDisplay();
				//raster2->init(256,256);
				//r2 = new ae::RasterDisplay(n2, raster2->getID());
				//gui.addWidget("tilemap", r2);
				//debugger = new ae::gameboy::debug::Debugger(static_cast<ae::gameboy::Gameboy*>(&(*si)), raster2);
				//t2 = new std::thread([&debugger, &t2]() { debugger->run(); t2->detach(); });
			}
			else {
				if ((t)&&(!(t->joinable()))) {
//					delete si;
					si = nullptr;
					delete t;
					gameselection.resetSelected();
					delete raster;
					gui.removeWidget("rasterdisplay");
				}
			}
		}
		gui.renderFrame();
	}
	return 0;

	// UI creation
	//if (!ae::ui::init())
	//	return 1;

	//ae::IMachine* si = nullptr;

	//ae::ui::menu::response r = ae::ui::menu::response::NOTHING;

	//while (r != ae::ui::menu::response::QUIT) {
	//	ae::ui::menu my_menu;
	//	r = my_menu.run(si);
	//	switch (r) {
	//	case ae::ui::menu::response::LAUNCH:
	//		si->init();
	//		si->run();
	//		break;
	//	case ae::ui::menu::response::GAMESELECTION:
	//	{
	//		ae::ui::InterfaceGameSelection i((si == nullptr) ? "" : si->getID());
	//		i.run();
	//		if (i.getSelection() != "") {
	//			if (si)
	//				delete si;
	//			si = ae::IMachine::create(i.getSelection());
	//		}
	//	}
	//	break;
	//	case ae::ui::menu::response::GAMESETTINGS:
	//	{
	//		ae::ui::InterfaceGameSettings i(si);
	//		i.run();
	//	}
	//	break;
	//	}
	//}
	//ae::ui::destroy();
	//_CrtDumpMemoryLeaks();
	//return 0;
}

