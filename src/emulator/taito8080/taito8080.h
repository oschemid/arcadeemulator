#pragma once
#include "types.h"
#include "emulator.h"
#include "xprocessors.h"


namespace ae::taito8080
{
	class Taito8080 : public emulator::Emulator
	{
	protected:
		xprocessors::UCpu _cpu;

	public:
		Taito8080();
		virtual ~Taito8080() = default;

		void init() override;
		void run(ae::gui::RasterDisplay*) override;
	};
}