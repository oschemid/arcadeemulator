#pragma once
#include "types.h"
#include "midway8080.h"
#include "emulator.h"
#include "../../controller/arcadecontroller.h"
#include "io.h"


namespace aos::midway8080
{
	class AmazingMaze : public Midway8080
	{
	public:
		AmazingMaze(const vector<aos::emulator::RomConfiguration>&,
			const emulator::GameConfiguration&);
		virtual ~AmazingMaze();

		emulator::SystemInfo getSystemInfo() const override {
			return emulator::SystemInfo{
				.geometry = {.width = 256, .height = 224}
			};
		}

		void init(aos::display::RasterDisplay*) override;
		uint8_t tick() override;

	protected:
		vector<aos::emulator::RomConfiguration> _roms;
		aos::io::Port _port0{ 0 };
		aos::io::Port _port1{ 0 };
		ae::controller::ArcadeController::Ptr _controller{ nullptr };

		uint8_t in(const uint8_t);
		void out(const uint8_t, const uint8_t);
		void updateDisplay();
	};
}
