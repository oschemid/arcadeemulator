#pragma once
#include "types.h"
#include "registry.h"
#include "display.h"


namespace ae::emulator
{
	struct SystemInfo
	{
		geometry_t geometry;
	};

	class Game
	{
	public:
		const string& hardware() const { return _hardware; }
		const string& version() const { return _version; }
		const string& romsfile() const { return _romsfile; }
		uint8_t settings(const string& name) const { auto res = _settings.find(name); if (res == _settings.end()) throw std::out_of_range("Unknown"); return res->second; }

		Game(const string& h,
			 const string& v,
			 const string& r,
			 std::map<string, uint8_t> s) : 
			_hardware{ h },
			_version{ v }, _romsfile{ r }, _settings{ s } {}
	protected:
		string _hardware;
		string _version;
		string _romsfile;
		std::map<string, uint8_t> _settings;
	};

	class Emulator
	{
	public:
		using Ptr = std::unique_ptr<Emulator>;
		using creator_fn = std::function<Ptr(const Game&)>;
		using registry = ae::RegistryHandler<Ptr, creator_fn>;

	public:
		virtual ~Emulator() = default;
		virtual SystemInfo getSystemInfo() const = 0;
		virtual void init(ae::display::RasterDisplay*) = 0;
		virtual uint8_t tick() = 0;

		void run();

	protected:
		Emulator() = default;
		uint64_t _clockPerMs{ 0 };
	};

	Emulator::Ptr create(const string&, const Game&);
}