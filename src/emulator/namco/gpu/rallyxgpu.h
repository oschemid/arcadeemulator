#pragma once

#include "tilegpu.h"

namespace aos::namco
{
	class RallyXGpu : public TileGpu
	{
	public:
		struct Configuration {
		};

		~RallyXGpu();

		void init(aos::display::RasterDisplay* , const vector<aos::emulator::RomConfiguration>&);

		geometry_t getGeometry() const { return { .width = 288, .height = 224, .rotation = geometry_t::rotation_t::NONE }; }

		using Ptr = std::unique_ptr<RallyXGpu>;

		static Ptr create() { return std::unique_ptr<RallyXGpu>(new RallyXGpu()); }

		void draw() override;

		void setScrollX(const uint8_t v) { _scrollX = v; }
		void setScrollY(const uint8_t v) { _scrollY = v; }

	protected:
		RallyXGpu();

		void initPalettes(const vector<aos::emulator::RomConfiguration>&);
		void initTilesSprites(const vector<aos::emulator::RomConfiguration>&);

		void drawBackground(const uint8_t);
		void drawSprites();
		void drawSidebar();

		uint8_t _scrollX{ 0 };
		uint8_t _scrollY{ 0 };
	};
}