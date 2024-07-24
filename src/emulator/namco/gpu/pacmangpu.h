#pragma once
#include "tilemap.h"
#include "emulator.h"
#include "tilegpu.h"

#include <vector>

namespace aos::namco
{
	using decodingfn = std::function<void(uint8_t*, const size_t)>;

	class PacmanGpu : public TileGpu
	{
	public:
		struct Configuration {
			enum class RasterModel { PACMAN, RALLYX } rasterModel;
			std::function<void(uint8_t*, const size_t)> romDecoding;
			uint16_t spriteAddress;
		};

		~PacmanGpu();

		PacmanGpu& romDecodingFn(decodingfn decoder) {
			_romDecoder = decoder;
			return *this;
		}

		void init(aos::device::RasterDisplay*,
			      const vector<aos::mmu::RomMapping>&);

		using Ptr = std::unique_ptr<PacmanGpu>;

		static Ptr create(const PacmanGpu::Configuration& configuration) { return std::unique_ptr<PacmanGpu>(new PacmanGpu(configuration)); }

//		vector<palette_t> getPalettes() const { return _palettes; }
//		aos::tilemap::Tiles getTiles() const { return _tiles; }
//		aos::tilemap::Tiles getSprites() const { return _sprites; }

		void draw() override;

		void writeSpritePos(const uint16_t p, const uint8_t v) { _spritesxy[p] = v; }
		void flip(const bool flip) { _flip = flip; }
	protected:
		PacmanGpu(const PacmanGpu::Configuration&);

		Configuration _configuration;
		decodingfn _romDecoder;

		bool _flip{ false };

		uint8_t* _spritesxy{ nullptr };

		void initPalettes(const vector<aos::mmu::RomMapping>&);
		void initTilesSprites(const vector<aos::mmu::RomMapping>&);

		void drawBackground();
		void drawSprites();
	};
}