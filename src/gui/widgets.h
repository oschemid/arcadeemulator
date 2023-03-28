#pragma once
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

	class RasterDisplay : public gui::widgets::Widget {
	protected:
		string _name;
		ImTextureID _textureid;
		ImVec2 _texturesize;

	public:
		RasterDisplay(const string&, const ImTextureID, const uint16_t, const uint16_t);
		void draw(gui::GuiManager*) override;
	};
}