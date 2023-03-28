#pragma once
#include "types.h"
#include "registry.h"
#include "../gui/widgets.h"


namespace ae::emulator
{
	struct SystemInfo
	{
		struct Geometry {
			uint16_t width;
			uint16_t height;
		} geometry;
	};

	class Game
	{
	public:
		const string& hardware() const { return _hardware; }
		const string& version() const { return _version; }
		const string& romsfile() const { return _romsfile; }
		uint8_t settings(const string& name) const { return _settings.find(name)->second; }

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
		virtual void init() = 0;

		virtual SystemInfo getSystemInfo() const = 0;
		virtual void run(ae::gui::RasterDisplay*) = 0;

	protected:
		Emulator() = default;
	};

	Emulator::Ptr create(const string&, const Game&);
}