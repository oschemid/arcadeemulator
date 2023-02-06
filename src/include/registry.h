#pragma once
#include "types.h"
#include <map>


namespace ae
{
	template<class I> class Registry
	{
	public:
		using factory_fn = std::function<I()>;

	protected:
		std::map<const string, const factory_fn> _entries;
	protected:
		Registry() = default;
		~Registry() = default;

	public:
		static Registry& instance() { static Registry _registry; return _registry; }

		void add(const string& name, factory_fn factory) { _entries.insert({ name, factory }); }
		I create(const string& name) { auto it = _entries.find(name); return (it != _entries.end()) ? it->second() : nullptr; }
	};

	template<class I> class RegistryHandler
	{
	public:
		RegistryHandler(const string& id, Registry<I>::factory_fn fn)
		{
			Registry<I>::instance().add(id, fn);
		}
	};
}
