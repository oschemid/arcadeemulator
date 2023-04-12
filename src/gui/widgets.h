#pragma once
#include "display.h"
#include "gui.h"
#include "../settings/console.h"
#include "../settings/library.h"


namespace ae {
	class AppSidebar : public gui::widgets::Sidebar {
	protected:
		Console _selected;
		Consoles _consoles;

	public:
		AppSidebar(Consoles);
		void drawContents() override;

		Console getSelected() const { return _selected; }
	};

	class GameSelection : public gui::widgets::Widget {
	protected:
		ae::Game* _selected;
		string _action{ "" };
		string _filtered;
		Library _library;
		Console _console;

		void drawTile(Game&, VkDescriptorSet);
		bool drawSettings(Game&);
	public:
		GameSelection(Library);
		void setFiltered(Console);
		void draw(gui::GuiManager*) override;
		Game getSelected() const { return (_action=="run")?*_selected : Game(); }
		void resetSelected() { _selected = nullptr; _action = ""; }
	};

	class DisplayWidget : public gui::widgets::Widget {
	protected:
		string _name;
		display::RasterDisplay* _raster{ nullptr };
		ImTextureID _textureid{ nullptr };
		ImVec2 _imagesize;
		float _zoom;

	public:
		DisplayWidget(const string&, display::RasterDisplay*, const float);
		void draw(gui::GuiManager*) override;
	};
}