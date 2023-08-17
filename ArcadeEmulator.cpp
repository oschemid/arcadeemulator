// ArcadeEmulator.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>

#include "library.h"

#include "emulator.h"
#include "registry.h"
#include "display.h"
#include "src/gui/vulkan/engine.h"
#include "src/gui/gui.h"
#include "src/gui/widgets.h"
#include <fstream>
#include "imgui_impl_sdl2.h"

#include "src/gui/debugger.h"
#include "tools.h"


int main(int argc, char** argv)
{
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		std::cout << "SDL could not be initialized: " << SDL_GetError();
		return false;
	}

	ae::gui::Window window("Arcade Emulation", 1280, 800);
	ae::gui::Engine engine(&window);
	ae::gui::GuiManager gui(&engine);

	aos::library::getConsoles().init();
	
	aos::ConsolesSidebar sidebar(aos::library::getConsoles());
	aos::GameSelection gameselection;
	gui.addWidget("sidebar", &sidebar);
	gui.addWidget("gameselection", &gameselection);

	window.init();
	engine.init();
	gui.init();
	std::string n1 = "TEST";
	std::string n2 = "TILES";

	std::thread* t = nullptr;
	std::thread* t2 = nullptr;
	bool done = false;
	aos::emulator::Emulator::Ptr si = nullptr;
	ae::DisplayWidget* r1 = nullptr;
	ae::TileMapWidget r2(n2, &gui);

	aos::display::RasterDisplay* raster = nullptr;
	while (!done)
	{
		done = gui.processEvent();
		gameselection.filterConsole(sidebar.getSelected());
		if (gameselection.getSelected()) {
			if (!si) {
				aos::library::Game* selected = gameselection.getSelected();
				si = selected->driver().creator(selected->driver().configuration, selected->driver().roms);
				aos::emulator::SystemInfo requirements = si->getSystemInfo();
				raster = new aos::display::RasterDisplay(requirements.geometry);
				raster->init();
				r1 = new ae::DisplayWidget(n1, raster, 2.);
				gui.addWidget("rasterdisplay", r1);
				si->init(raster);
				t = new std::thread([&si, &t]() { si->run(); t->detach(); });

				//r2.reset(si.get());
				//gui.addWidget("tilemap", &r2);
				//debugger = new ae::gameboy::debug::Debugger(static_cast<ae::gameboy::Gameboy*>(&(*si)), raster2);
				//t2 = new std::thread([&debugger, &t2]() { debugger->run(); t2->detach(); });
			}
			else {
				if ((t)&&(!(t->joinable()))) {
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
}

