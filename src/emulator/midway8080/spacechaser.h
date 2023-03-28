#pragma once
#include "types.h"
#include "midway8080.h"
#include "mb14241.h"
#include "../../controller/arcadecontroller.h"


namespace ae::midway8080
{
	class SpaceChaser : public GameBoard
	{
	public:
		SpaceChaser();
		virtual ~SpaceChaser();
		void init(const emulator::Game&) override;
		rgb_t color(const uint8_t, const uint8_t, const bool) const override;
		void out(const uint8_t, const uint8_t) override;
		uint8_t in(const uint8_t) override;
		std::vector<std::pair<uint16_t, string>> romFiles() const override;
		void controllerTick() override { _controller->tick(); }
		bool romExtended() const { return true; }

	protected:
		xprocessors::MB14241::Ptr _shifter{ nullptr };
		ae::controller::ArcadeController::Ptr _controller{ nullptr };

		uint8_t _port2{ 0 };

		uint8_t* _colorram;
	};
}