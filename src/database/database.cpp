#include "database.h"
#include <ranges>
#include <filesystem>

namespace fs = std::filesystem;


namespace aos::database
{
	bool init()
	{
		return true;
	}

	bool loadPlatforms(const json& content)
	{
		for (auto& [key, value] : content.items())
		{
			if ((key == "api") || (key == "metadata"))
				continue;
			auto platform = new Platform(key, value);
		}
		return true;
	}
	bool loadDrivers(const json& content)
	{
		for (auto& [key, value] : content.items())
		{
			if ((key == "api") || (key == "metadata"))
				continue;

			string name = value["name"].template get<string>();
			string platform = value["platform"].template get<string>();
			Driver* ndriver = new Driver(key, name, platform);

			string version = key;
			if (value.contains("version"))
				version = value["version"].template get<string>();
			ndriver->version(version);

			if (value.contains("year"))
				ndriver->year(value["year"].template get<uint16_t>());
			if (value.contains("overview"))
				ndriver->overview(value["overview"].template get<string>());

			json settings;
			if (value.contains("default"))
			{
				settings = value["default"];
			}
			if (value.contains("drivers"))
			{
				auto drivers = value["drivers"];
				for (auto& [id, driver] : drivers.items())
				{
					DriverVersion emul;
					json localsettings(settings);
					localsettings.merge_patch(driver);
					emul.core = localsettings["core"].template get<std::string>();
					emul.settings = localsettings;
					ndriver->addVersion(id, emul);
				}
			}
			else
			{
				DriverVersion emul;
				emul.core = settings["core"].template get<std::string>();
				emul.settings = settings;
				emul.settings["name"] = name;
				ndriver->addVersion(version, emul);
			}
		}
		return true;
	}
	bool loadFile(const fs::directory_entry& entry)
	{
		std::ifstream stream(entry.path());
		json content = json::parse(stream);
		if (!content.contains("api"))
			return false;
		string api = content["api"].template get<std::string>();
		if (api == "arcados.platform/v1")
			return loadPlatforms(content);
		if (api == "arcados.driver/v1")
			return loadDrivers(content);
		return true;
	}

	bool loadDirectory(const fs::directory_entry& entry)
	{
		for (const auto& entry : fs::directory_iterator(entry.path()))
		{
			if (entry.is_regular_file())
			{
				loadFile(entry);
			}
			else if (entry.is_directory())
			{
				loadDirectory(entry);
			}
		}
		return true;
	}
	bool load(const string& path)
	{
		loadDirectory(fs::directory_entry(path));
		return true;
	}

	Platform* Platforms::get(const string& name)
	{
		auto platform = find(name);
		if (platform != end())
			return platform->second;
		return nullptr;
	}

	Driver::Driver(const string& id, const string& name, const string& platform) :
		_id{ id },
		_name{ name },
		_platform{ platform }
	{
		Drivers::registering(id, this);
	}

	Drivers& Drivers::singleton()
	{
		static Drivers drivers;
		return drivers;
	}

	bool Drivers::registering(const string& name, Driver* driver)
	{
		singleton().insert({ name, driver });
		return true;
	}

	Driver* Drivers::get(const string& name)
	{
		auto driver = find(name);
		if (driver != end())
			return driver->second;
		return nullptr;
	}
}
