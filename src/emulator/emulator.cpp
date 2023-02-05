#include "emulator.h"
#include "registry.h"


using namespace ae::emulator;

Registry::RegistryItem::RegistryItem(const string& name, factory_fn factory) :
	_name(name),
	_factory(factory)
{}

Registry& Registry::instance()
{
	static Registry _registry;
	return _registry;
}

void Registry::add(const string& name, factory_fn factory)
{
	_entries.insert({ name, RegistryItem(name, factory) });
}

UEmulator Registry::create(const string& id)
{
	auto it = _entries.find(id);
	return (it != _entries.end()) ? it->second.factory()() : nullptr;
}

UEmulator Emulator::create(const string& id)
{
	return Registry::instance().create(id);
}