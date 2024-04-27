#include "pacmansystem.h"


namespace aos::namco
{
	static tools::RomManager& romcache()
	{
		static tools::RomManager instance("roms/namco");
		return instance;
	}
	static bool cache_loaded = false;

	Core::Ptr createPacman(const json& settings)
	{
		if (!cache_loaded) {
			romcache().preload();
			cache_loaded = true;
		}

		PacmanSystem2::Configuration configuration;

		mmu::RomMappings mappings;
		if (settings["roms"].contains("cpu2b"))
			configuration.roms.extendedMemoryType = PacmanSystem2::Configuration::Roms::MemoryType::BANK2;
		else if (settings["roms"].contains("cpu3"))
			configuration.roms.extendedMemoryType = PacmanSystem2::Configuration::Roms::MemoryType::RAMROM;
		else if (settings["roms"].contains("cpu2"))
			configuration.roms.extendedMemoryType = PacmanSystem2::Configuration::Roms::MemoryType::ROM;

		for (auto& [zone, roms] : settings["roms"].items())
		{
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
		uint8_t dsw1 = 0;

		configuration.roms.files = mappings;
		if (settings.contains("settings"))
		{
			for (auto& [parameter, setting] : settings["settings"].items())
			{
				if (parameter == "alibaba")
				{
					configuration.hardware.alibaba = setting.template get<bool>();
				}
				if (setting.contains("parameter"))
				{
					if (setting["parameter"] == "dsw1")
					{
						dsw1 += setting["value"].template get<uint8_t>();
					}
				}
				if (parameter == "controller")
				{
					if (setting.contains("coin"))
					{
						configuration.invertedcoin = true;
					}
					if (setting.contains("fire"))
					{
						configuration.controller.fire = true;
					}
				}
				if (parameter == "decoders")
				{
					if (setting.contains("cpu"))
						configuration.roms.cpudecoder = setting["cpu"].template get<string>();
					if (setting.contains("video"))
						configuration.roms.videodecoder = setting["video"].template get<string>();
					if (setting.contains("interrupt"))
						configuration.interruptdecoder = setting["interrupt"].template get<string>();
				}
			}
		}
		auto emul = std::make_unique<aos::namco::PacmanSystem2>(configuration);
		emul->settings("dsw1", dsw1);
		return emul;
	}

	static bool registering = CoreFactory::registerCore("pacman", createPacman);
}