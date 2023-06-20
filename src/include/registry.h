#pragma once
#include "types.h"
#include <map>


namespace ae
{
	template<class I> class Registry
	{
	public:
		static Registry& instance() { static Registry _registry; return _registry; }

		void add(const string& name, I object) { _entries.insert({ name, object }); }
		I& get(const string& name) { auto it = _entries.find(name); return (it != _entries.end()) ? it->second : nullptr; }
		std::map<const string, I> get() { return _entries; }
	protected:
		Registry() = default;
		~Registry() = default;

		std::map<const string, I> _entries;
	};

	template<class I> class RegistryHandler
	{
	public:
		RegistryHandler(const string& id, I object)
		{
			Registry<I>::instance().add(id, object);
		}
	};
}
