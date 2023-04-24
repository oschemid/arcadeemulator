#pragma once
#include "types.h"
#include "midway8080.h"
#include "../../controller/arcadecontroller.h"
#include "io.h"


namespace ae::midway8080
{
	class AmazingMaze : public GameBoard
	{
	public:
		AmazingMaze();
		virtual ~AmazingMaze() = default;
		void init(const emulator::Game&) override;
		std::vector<std::pair<uint16_t, string>> romFiles() const override;

		void out(const uint8_t, const uint8_t) override;
		uint8_t in(const uint8_t) override;

		void controllerTick() override { _controller->tick(); _port0.tick(*_controller); _port1.tick(*_controller); }
		rgb_t color(const uint8_t, const uint8_t, const bool) const override;

	protected:
		ae::controller::ArcadeController::Ptr _controller{ nullptr };
		ae::io::Port _port0{ 0 };
		ae::io::Port _port1{ 0 };
	};
}