#pragma once
#include "types.h"
#include "emulator.h"


#include "midway8080.h"
#include "mb14241.h"
#include "../../controller/arcadecontroller.h"
#include "io.h"


namespace aos::midway8080
{
	class SpaceInvaders : public Midway8080
	{
	public:
		SpaceInvaders(vector<pair<uint16_t, string>>,
			const emulator::GameConfiguration&,
			std::function<rgb_t(const uint8_t, const uint8_t)> = nullptr);
		virtual ~SpaceInvaders();

		emulator::SystemInfo getSystemInfo() const override {
			return emulator::SystemInfo{
				.geometry = {.width = 224, .height = 256}
			};
		}

		void init(ae::display::RasterDisplay*) override;
		uint8_t tick() override;

	protected:
		xprocessors::MB14241::Ptr _shifter{ nullptr };
		vector<pair<uint16_t, string>> _roms;
		ae::io::Port _port0{ 0 };
		ae::io::Port _port1{ 0 };
		ae::io::Port _port2{ 0 };
		ae::controller::ArcadeController::Ptr _controller{ nullptr };
		std::function<aos::rgb_t(const uint8_t, const uint8_t)> _colorfn;

		uint8_t in(const uint8_t);
		void out(const uint8_t, const uint8_t);
		void updateDisplay();
	};

	class AlienInvaders : public SpaceInvaders
	{
	public:
		AlienInvaders(vector<pair<uint16_t, string>>, const aos::emulator::GameConfiguration&);
		virtual ~AlienInvaders();
	};
}