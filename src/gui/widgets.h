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
		Game _selected;
		string _filtered;
		Library _library;
		Console _console;

		void drawTile(const Game&, VkDescriptorSet);
	public:
		GameSelection(Library);
		void setFiltered(Console);
		void draw(gui::GuiManager*) override;
		Game getSelected() const { return _selected; }
		void resetSelected() { _selected = Game(); }
	};

	class RasterDisplay : public gui::widgets::Widget {
	protected:
		string _name;
		ImTextureID _textureid;

	public:
		RasterDisplay(const string&, const ImTextureID);
		void draw(gui::GuiManager*) override;
	};
}