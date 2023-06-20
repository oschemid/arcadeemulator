#pragma once
#include "display.h"
#include "gui.h"
#include "library.h"


namespace aos {
	class ConsolesSidebar : public ae::gui::widgets::Sidebar {
	public:
		ConsolesSidebar(library::Consoles&);
		void drawContents() override;

		library::Console* getSelected() const { if (_selected != "") return aos::library::getConsoles().get(_selected); return nullptr; }

	protected:
		string _selected;
		library::Consoles _consoles;

	};

	class GameSelection : public ae::gui::widgets::Widget {
	public:
		GameSelection();
		void filterConsole(library::Console*);
		void draw(ae::gui::GuiManager*) override;
		library::Game* getSelected() const { return _selected; }
		void resetSelected() { _selected = nullptr; }

	protected:
		library::Game* _selected;
		library::Console* _console;

		void drawTile(ae::gui::GuiManager*, library::Game&);
		void drawSettings(library::Game&);
	};
}

namespace ae {
/*	class GameSelection : public gui::widgets::Widget {
	protected:
		ae::Game* _selected;
		string _action{ "" };
		string _filtered;
		Library _library;
		aos::library::Console* _console;

		void drawTile(Game&, VkDescriptorSet);
		bool drawSettings(Game&);
	public:
		GameSelection(Library);
		void setFiltered(Console);
		void draw(gui::GuiManager*) override;
		Game getSelected() const { return (_action=="run")?*_selected : Game(); }
		void resetSelected() { _selected = nullptr; _action = ""; }
	};
	*/
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