#pragma once
#include "emulator.h"
#include "tilemap.h"
#include "xprocessors.h"
#include "SDL2/SDL.h"


namespace ae::namco
{
	class RallyX : public emulator::Emulator
	{
	public:
		RallyX(const emulator::Game&);
		virtual ~RallyX();

		emulator::SystemInfo getSystemInfo() const override;
		void init(ae::display::RasterDisplay*) override;
		uint8_t tick() override;

	protected:
		xprocessors::Cpu::Ptr _cpu{ nullptr };
		display::RasterDisplay* _display{ nullptr };

		void initVideoRom();

		uint8_t* _rom{ nullptr };
		uint8_t* _videoram{ nullptr };
		uint8_t* _ram{ nullptr };
		
		uint8_t _interrupt_vector{ 0 };
		std::vector<ae::tilemap::Tile> _tiles;
		std::vector<rgb_t> _palette;
		std::vector<rgb_t> _lookup;

		uint8_t* _radarattr{ nullptr };

		uint8_t read(const uint16_t) const;
		void write(const uint16_t, const uint8_t);

		void draw();
		void drawBackground();
	};
}
