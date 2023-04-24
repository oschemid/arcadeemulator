#pragma once
#include "emulator.h"
#include "tilemap.h"
#include "xprocessors.h"
#include "io.h"
#include "../../controller/arcadecontroller.h"


namespace ae::namco
{
	class Pacman : public emulator::Emulator
	{
	public:
		Pacman(const emulator::Game&);
		virtual ~Pacman();

		emulator::SystemInfo getSystemInfo() const override;
		void init(ae::display::RasterDisplay*) override;
		uint8_t tick() override;

		std::map<string, ae::tilemap::Tiles> getTiles() const override {
			return { {"tiles", _tiles}, {"sprites", _sprites} };
		}
		std::vector<palette_t> getPalettes() const override {
			return _lookup;
		}

	protected:
		xprocessors::Cpu::Ptr _cpu{ nullptr };
		display::RasterDisplay* _display{ nullptr };
		tilemap::TileMap* _tilemap{ nullptr };
		ae::controller::ArcadeController::Ptr _controller;
		ae::io::Port _port0{ 0 };
		ae::io::Port _port1{ 0 };
		ae::io::Port _port2{ 0 };

		void initVideoRom();

		uint8_t* _rom{ nullptr };
		uint8_t* _ram{ nullptr };

		bool _interrupt_enabled{ false };
		uint8_t _interrupt_vector{ 0 };
		ae::tilemap::Tiles _tiles;
		ae::tilemap::Tiles _sprites;
		palette_t _palette;
		std::vector<palette_t> _lookup;
		uint8_t* _spritesxy;

		uint8_t read(const uint16_t) const;
		void write(const uint16_t, const uint8_t);

		void draw();
		void drawBackground(const uint8_t);
		void drawSprites();
	};
}
