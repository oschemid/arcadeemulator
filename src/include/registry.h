#pragma once
#include "types.h"
#include <map>


namespace ae
{
	template<class I, typename creator_fn> class Registry
	{
	public:
		static Registry& instance() { static Registry _registry; return _registry; }

		void add(const string& name, creator_fn factory) { _entries.insert({ name, factory }); }
		creator_fn create(const string& name) { auto it = _entries.find(name); return (it != _entries.end()) ? it->second : nullptr; }

	protected:
		Registry() = default;
		~Registry() = default;

		std::map<const string, const creator_fn> _entries;
	};

	template<class I, typename creator_fn> class RegistryHandler
	{
	public:
		RegistryHandler(const string& id, creator_fn fn)
		{
			Registry<I, creator_fn>::instance().add(id, fn);
		}
	};
}
