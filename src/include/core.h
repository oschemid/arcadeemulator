#pragma once
#include <memory>
#include "types.h"
#include "device.h"
#include "memory.h"
#include "nlohmann/json.hpp"


using nlohmann::json;


namespace aos::ui { class Manager; }

namespace aos
{
	class Core
	{
	public:
		using Ptr = std::unique_ptr<Core>;

	public:
		virtual ~Core() = default;
		virtual DisplayDevice::DisplayRequirements getRequirements() const = 0;
		virtual void init(map<string, Device::SharedPtr>) = 0;
		virtual void run() = 0;

	protected:
		Core() = default;
	};

	class CoreFactory
	{
	public:
		using creator_fn = std::function<Core::Ptr(const json&)>;

		static Core::Ptr create(const string&, const json&);
		static bool registerCore(const string&, creator_fn);
	};
	//struct Driver
	//{
	//	using creator_fn = std::function<Machine::Ptr(const aos::mmu::RomMappings&)>;

	//	string name;
	//	string version;
	//	bool main_version{ false };
	//	string core;
	//	creator_fn creator;
	//	aos::mmu::RomMappings roms;

	//	bool is_unavailable() const { return std::any_of(roms.begin(), roms.end(), [](const aos::mmu::RomMapping& r) { return r.rom.filename.empty(); }); }
	//	Machine::Ptr create() { return creator(roms); }
	//};
}
