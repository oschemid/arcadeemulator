// ArcadeEmulator.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>

#include "emulator.h"
#include "display.h"
#include "src/gui/vulkan/engine.h"
#include "src/gui/gui.h"
#include "src/gui/widgets.h"
#include <fstream>
#include "src/settings/library.h"
#include "src/settings/console.h"
#include "imgui_impl_sdl2.h"

#include "src/gui/debugger.h"


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
	std::string n1 = "TEST";
	std::string n2 = "TILES";

	std::thread* t = nullptr;
	std::thread* t2 = nullptr;
	bool done = false;
	ae::emulator::Emulator::Ptr si = nullptr;
	ae::DisplayWidget* r1 = nullptr;
	ae::TileMapWidget r2(n2, &gui);

	ae::display::RasterDisplay* raster = nullptr;
	while (!done)
	{
		done = gui.processEvent();
		gameselection.setFiltered(sidebar.getSelected());
		if (gameselection.getSelected().title() != "") {
			if (!si) {
				auto version = gameselection.getSelected().currentVersion();
				std::map<std::string, uint8_t> settings;
				for (auto s : version->settings) {
					std::string id = s.at("id");
					uint8_t value = s.at("value");
					settings.insert({ id,value });
				}
				ae::emulator::Game game(gameselection.getSelected().hardware(),
					                    version->_version,
					                    version->_romspath,
					                    settings);
				si = ae::emulator::create(sidebar.getSelected().id(),game);
				ae::emulator::SystemInfo requirements = si->getSystemInfo();
				raster = new ae::display::RasterDisplay(requirements.geometry);
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

