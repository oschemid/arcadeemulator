#pragma once
#include "display.h"
#include "gui.h"
#include "emulator.h"
#include "display.h"


using aos::emulator::Emulator;

namespace ae {
	class TileMapWidget : public gui::widgets::Widget {
	public:
		TileMapWidget(const string&, aos::ui::UiManager* manager);
		void reset(Emulator*);
		void draw(aos::ui::UiManager*) override;

	protected:
		string _name;
		aos::ui::UiManager* _manager;
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

namespace aos::gui
{
	class DebuggingWidget : public ae::gui::widgets::Widget
	{
	public:
		DebuggingWidget(const string&, aos::emulator::DebuggerTools* debug);
		void draw(aos::ui::UiManager*) override;

	protected:
		string _name;
		aos::emulator::DebuggerTools* _debug;
	};
}