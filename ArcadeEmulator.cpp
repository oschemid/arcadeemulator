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

				auto raster = std::make_shared<aos::ui::RasterDisplayWidget>(requirements.geometry);
				raster->init();
				gui.addWidget("display", raster);
				devices.insert({ "display", raster });

				auto keyboard = std::make_shared<aos::ui::AmstradKeyboardWidget>();
				keyboard->init();
				gui.addWidget("keyboard", keyboard);
				devices.insert({ "keyboard", keyboard });

				auto controller = std::make_shared<aos::ui::ArcadeControllerWidget>();
				controller->init();
				gui.addWidget("controller", controller);
				devices.insert({ "controller", controller });

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
