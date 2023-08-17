#pragma once
#include "display.h"
#include "gui.h"
#include "emulator.h"
#include "display.h"


using aos::emulator::Emulator;

namespace ae {
	class TileMapWidget : public gui::widgets::Widget {
	public:
		TileMapWidget(const string&, gui::GuiManager* manager);
		void reset(Emulator*);
		void draw(gui::GuiManager*) override;

	protected:
		string _name;
		gui::GuiManager* _manager;
		Emulator* _emulator{ nullptr };
		std::map<string, ImTextureID> _maps;
		string _current_map;
		std::vector<palette_t> _palettes;
		uint8_t _current_palette;
		bool _needRefresh{ false };

		ImTextureID createTexture(const aos::tilemap::Tiles);
		void refreshTextures();
	};
}
