#include "database.h"
#include <map>


namespace aos::database
{
	static std::map<string, PlatformCategory> categoryNameToEnum = {
		{ "Arcade", PlatformCategory::ARCADE },
		{ "Computer", PlatformCategory::COMPUTER },
		{ "Console", PlatformCategory::CONSOLE }
	};
	static std::map<PlatformCategory, string> categoryEnumToName = {
		{ PlatformCategory::ARCADE, "Arcade" },
		{ PlatformCategory::COMPUTER, "Computer"},
		{ PlatformCategory::CONSOLE, "Console"}
	};

	PlatformCategory decodeCategory(const string& category)
	{
		auto it = categoryNameToEnum.find(category);
		if (it != categoryNameToEnum.end())
			return it->second;
		throw std::runtime_error("Unknown category " + category);
	}

	Platform::Platform(const string& id, const json& settings) :
		_id{ id }
	{
		if (!settings.is_object())
			throw std::runtime_error("Bad settings for platform " + id);
		if (!settings.contains("name"))
			throw std::runtime_error("Missing name for platform " + id);
		if (!settings.contains("category"))
			throw std::runtime_error("Missing category for platform " + id);
		_name = settings["name"].template get<std::string>();
		_category = decodeCategory(settings["category"].template get<std::string>());
		Platforms::registering(id, this);
	}

	Platforms& Platforms::singleton()
	{
		static Platforms platforms;
		return platforms;
	}

	bool Platforms::registering(const string& name, Platform* platform)
	{
		singleton().insert({ name, platform });
		return true;
	}
}