#pragma once
#include "tilemap.h"
#include "emulator.h"
#include "tilegpu.h"

#include <vector>

namespace aos::namco
{

	class PacmanGpu : public TileGpu
	{
	public:
		struct Configuration {
			geometry_t::rotation_t orientation;
			enum class RasterModel { PACMAN, RALLYX } rasterModel;
			enum class TileModel { PACMAN, PONPOKO } tileModel;
			enum class RomModel { PACMAN, WOODPECKER } romModel;
			uint16_t spriteAddress;
		};

		~PacmanGpu();

		void init(aos::display::RasterDisplay*,
			      const vector<aos::emulator::RomConfiguration>&);

		using Ptr = std::unique_ptr<PacmanGpu>;

		static Ptr create(const PacmanGpu::Configuration& configuration) { return std::unique_ptr<PacmanGpu>(new PacmanGpu(configuration)); }

		geometry_t getGeometry() const { return { .width = 288, .height = 224, .rotation = _configuration.orientation }; }

		vector<palette_t> getPalettes() const { return _palettes; }
		aos::tilemap::Tiles getTiles() const { return _tiles; }
		aos::tilemap::Tiles getSprites() const { return _sprites; }

		void draw() override;

		void writeSpritePos(const uint16_t p, const uint8_t v) { _spritesxy[p] = v; }
		void flip(const bool flip) { _flip = flip; }

	protected:
		PacmanGpu(const PacmanGpu::Configuration&);

		Configuration _configuration;

		bool _flip{ false };

		uint8_t* _spritesxy{ nullptr };

		void initPalettes(const vector<aos::emulator::RomConfiguration>&);
		void initTilesSprites(const vector<aos::emulator::RomConfiguration>&);

		void drawBackground();
		void drawSprites();
	};
}