#pragma once
#include "types.h"
#include "midway8080.h"
#include "mb14241.h"
#include "../../controller/arcadecontroller.h"
#include "io.h"


namespace aos::midway8080
{
	class SpaceChaser : public Midway8080
	{
	public:
		SpaceChaser(const std::vector<aos::emulator::RomConfiguration>&,
			const emulator::GameConfiguration&);
		virtual ~SpaceChaser();

		emulator::SystemInfo getSystemInfo() const override {
			return emulator::SystemInfo{
				.geometry = {.width = 224, .height = 256}
			};
		}

		void init(ae::display::RasterDisplay*) override;
		uint8_t tick() override;

	protected:
		xprocessors::MB14241::Ptr _shifter{ nullptr };
		std::vector<aos::emulator::RomConfiguration> _roms;
		ae::io::Port _port0{ 0 };
		ae::io::Port _port1{ 0 };
		ae::io::Port _port2{ 0 };
		ae::controller::ArcadeController::Ptr _controller{ nullptr };
		std::function<rgb_t(const uint8_t, const uint8_t)> _colorfn;

		uint8_t* _colorram;

		uint8_t in(const uint8_t);
		void out(const uint8_t, const uint8_t);
		void updateDisplay();
		void writeColorRam(const uint16_t, const uint8_t);
		uint8_t readColorRam(const uint16_t) const;
	};
}
