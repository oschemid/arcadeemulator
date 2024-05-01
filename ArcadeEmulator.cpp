// ArcadeEmulator.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>

#include "database.h"
#include <thread>

#include "emulator.h"
#include "core.h"
#include "registry.h"
#include "ui.h"

#include "src/ui/widgets.h"
#include <fstream>
#include "imgui_impl_sdl2.h"

//#include "src/ui/debugger.h"
#include "tools.h"
#include "ui/rasterdisplaywidget.h"
#include "ui/amstradkeyboardwidget.h"
#include "ui/arcadecontrollerwidget.h"


int main(int argc, char** argv)
{
	aos::ui::init();
	aos::database::init();

	aos::ui::Manager gui({ "Arcad-OS", 0, 0 });
	gui.init();

	aos::database::load("./data");

	aos::database::getConsoles().init();
	
	aos::database::Driver* selected = nullptr;
	gui.addWidget("menu", std::make_unique<aos::MenuWidget>(&selected));

	std::thread* t = nullptr;
//	aos::gui::DebuggingWidget* r2 = nullptr;

	aos::Core::Ptr machine = nullptr;

	bool done = false;
	while (!done)
	{
		done = gui.processEvent();
		if (selected)
		{
			if (!machine)
			{
				gui.hideWidget("menu");
				machine = aos::CoreFactory::create(selected->emulator().core, selected->emulator().settings);

				auto requirements = machine->getRequirements();
				std::map<string, aos::Device::SharedPtr> devices;

				for (auto& [name, config] : requirements.items())
				{
					if (config["type"] == "display")
					{
						auto raster = std::make_shared<aos::ui::RasterDisplayWidget>(geometry_t{.width=config["width"],
							.height=config["height"],
							.rotation=(config["rotation"]=="ROT90")? geometry_t::rotation_t::ROT90 : geometry_t::rotation_t::NONE});
						raster->init();
						gui.addWidget(name, raster);
						devices.insert({ name, raster });
					}
					if (config["type"] == "joystick")
					{
						auto controller = std::make_shared<aos::ui::ArcadeControllerWidget>(config["fire"], config["secundary"]);
						controller->init();
						gui.addWidget(name, controller);
						devices.insert({ name, controller });
					}
					if (config["type"] == "amstradkbd")
					{
						auto keyboard = std::make_shared<aos::ui::AmstradKeyboardWidget>();
						keyboard->init();
						gui.addWidget(name, keyboard);
						devices.insert({ name, keyboard });
					}
				}

				machine->init(devices);

				t = new std::thread([&machine, &t]() { machine->run(); t->detach(); });
#ifdef _DEBUG
				//auto debugger = si->getDebugger();

				//if (debugger) {
				//	r2 = new aos::gui::DebuggingWidget("Z80 debugger", debugger);
				//	gui.addWidget("debugger", r2);
				//}
#endif
			}
			else {
				if ((t)&&(!(t->joinable()))) {
					delete t;
				}
			}
		}
		gui.renderFrame();
	}
	return 0;
}
