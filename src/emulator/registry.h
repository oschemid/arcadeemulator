#pragma once
#include "types.h"
#include "emulator.h"
#include <map>
#include <memory>


namespace ae::emulator
{
	class Registry
	{
	public:
		using factory_fn = std::function<UEmulator()>;

	protected:
		class RegistryItem
		{
		protected:
			string _name;
			factory_fn _factory;

		public:
			RegistryItem(const string&, const factory_fn);
			factory_fn factory() const { return _factory; }
		};

		std::map<const string, const RegistryItem> _entries;
	protected:
		Registry() = default;
		~Registry() = default;

	public:
		static Registry& instance();

		void add(const string&, factory_fn);
		UEmulator create(const string&);
	};

	class RegistryHandler
	{
	public:
		RegistryHandler(const string& id, Registry::factory_fn fn)
		{
			Registry::instance().add(id, fn);
		}
	};
}
