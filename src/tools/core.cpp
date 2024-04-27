#include "core.h"


namespace aos
{
	std::map<string, CoreFactory::creator_fn>& mappings()
	{
		static std::map<string, CoreFactory::creator_fn> mapping;
		return mapping;
	}

	Core::Ptr CoreFactory::create(const string& core, const json& settings)
	{
		auto maps = mappings();
		auto map = maps.find(core);
		if (map != maps.end())
			return map->second(settings);
		return nullptr;
	}
	bool CoreFactory::registerCore(const string& core, creator_fn fn)
	{
		mappings().insert({ core, fn });
		return true;
	}
}
