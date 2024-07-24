#include "midway8080.h"


namespace aos::midway8080
{
	static tools::RomManager& romcache()
	{
		static tools::RomManager instance("roms/midway8080");
		return instance;
	}
	static bool cache_loaded = false;

	Core::Ptr createMidway(const json& settings)
	{
		if (!cache_loaded)
		{
			romcache().preload();
			cache_loaded = true;
		}
		Midway8080::Configuration configuration;

		mmu::RomMappings mappings;
		for (auto& [zone, roms] : settings["roms"].items())
		{
			if (zone == "cpu2")
			{
				configuration.roms.extendedRoms = true;
			}
			for (auto& rom : roms)
			{
				if (rom.contains("file"))
				{
					mmu::RomMapping mapping{ zone, {"",""},{0,0} };
					if (rom.contains("size"))
					{
						mapping.mapping.size = tools::json_helpers::get<uint16_t>(rom["size"]);
					}
					if (rom.contains("start"))
					{
						mapping.mapping.start = tools::json_helpers::get<uint16_t>(rom["start"]);
					}
					mapping.rom = romcache().get(rom["file"]);
					mappings.push_back(mapping);
				}
			}
		}
		configuration.roms.files = mappings;
		uint8_t port1 = 0;
		uint8_t port2 = 0;
		if (settings.contains("settings"))
		{
			for (auto& [parameter, setting] : settings["settings"].items())
			{
				if (parameter == "display")
				{
					configuration.display.colormap = configuration.display.WHITE;
					if (setting.contains("colormap"))
					{
						if (setting["colormap"] == "GREEN")
							configuration.display.colormap = configuration.display.GREEN;
						if (setting["colormap"] == "SPACEINVADERS")
							configuration.display.colormap = configuration.display.SPACEINVADERS;
						if (setting["colormap"] == "COSMICMONSTERS")
							configuration.display.colormap = configuration.display.COSMICMONSTERS;
						if (setting["colormap"] == "SPACECHASER")
							configuration.display.colormap = configuration.display.SPACECHASER;
					}
					if (setting.contains("rotated"))
					{
						configuration.display.rotated = true;
					}
				}
				if (parameter == "controllers")
				{
					if (setting.contains("joystick"))
					{
						auto joystick = setting["joystick"];
						if (joystick["type"] == "4WAY")
							configuration.controllers.joysticks.type = configuration.controllers.joysticks.JOYSTICK4WAY;
						if (joystick["type"] == "2WAYFIRE")
							configuration.controllers.joysticks.type = configuration.controllers.joysticks.JOYSTICK2WAYFIRE;
						if (joystick["type"] == "4WAYFIRE")
							configuration.controllers.joysticks.type = configuration.controllers.joysticks.JOYSTICK4WAYFIRE;
						configuration.controllers.joysticks.mapping = joystick["mapping"];
					}
					if (setting.contains("buttons"))
					{
						auto buttons = setting["buttons"];
						configuration.controllers.buttons = buttons["mapping"].template get<string>();
					}
				}
				if (setting.contains("parameter"))
				{
					if (setting["parameter"] == "port1")
					{
						port1 += setting["value"].template get<uint8_t>();
					}
					if (setting["parameter"] == "port2")
					{
						port2 += setting["value"].template get<uint8_t>();
					}
				}
			}
		}
		auto emul = std::make_unique<Midway8080>(configuration);
		emul->setPorts(port1, port2);
		return emul;
	}

	static bool registering = CoreFactory::registerCore("midway8080", createMidway);
}