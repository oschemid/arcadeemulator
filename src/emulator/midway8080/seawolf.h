#pragma once
#include "types.h"
#include "midway8080.h"
#include "mb14241.h"


namespace ae::midway8080
{
	class SeaWolf : public GameBoard
	{
	protected:
		xprocessors::MB14241::Ptr _shifter;

		uint8_t _periscopePosition;
		bool _fire;

	public:
		SeaWolf();
		virtual ~SeaWolf();
		void init(const emulator::Game&) override;
		void out(const uint8_t, const uint8_t) override;
		uint8_t in(const uint8_t) override;
		void controllerTick() override;
		std::vector<std::pair<uint16_t, string>> romFiles() const override;
		rgb_t color(const uint8_t, const uint8_t, const bool) const override;
	};
}