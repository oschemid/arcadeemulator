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

	class Emulator
	{
	public:
		using Ptr = std::unique_ptr<Emulator>;

	public:
		virtual void init(const json&) = 0;

		virtual SystemInfo getSystemInfo() const = 0;
		virtual void run(ae::gui::RasterDisplay*) = 0;

		// Factory
		static Emulator::Ptr create(const string&);
	};

	using RegistryHandler = ae::RegistryHandler<Emulator::Ptr>;
}