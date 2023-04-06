#pragma once
#include "types.h"
#include "midway8080.h"
#include "mb14241.h"
#include "../../controller/arcadecontroller.h"


namespace ae::midway8080
{
	class SpaceInvaders : public GameBoard
	{
	public:
		SpaceInvaders();
		virtual ~SpaceInvaders();

		void init(const emulator::Game&) override;

	protected:
		xprocessors::MB14241::Ptr _shifter{ nullptr };
		ae::controller::ArcadeController::Ptr _controller{ nullptr };

		string _version;
		Port _port0{ 0 };
		Port _port1{ 0 };
		Port _port2{ 0 };

	public:
		void out(const uint8_t, const uint8_t) override;
		uint8_t in(const uint8_t) override;
		std::vector<std::pair<uint16_t, string>> romFiles() const override;
		void controllerTick() override { _controller->tick(); _port0.tick(*_controller); _port1.tick(*_controller); _port2.tick(*_controller); }
		rgb_t color(const uint8_t, const uint8_t, const bool) const override;
	};

	class AlienInvaders : public SpaceInvaders
	{
	public:
		AlienInvaders();
		virtual ~AlienInvaders();
	};
}