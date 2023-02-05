#pragma once
#include "types.h"
#include "vulkan/engine.h"
#include <map>


namespace ae::gui {
	class GuiManager;
}

namespace ae::gui::widgets {
	class Widget {
	protected:
		bool _hidden;
	public:
		Widget(): _hidden(false) {}

		virtual void draw(ae::gui::GuiManager*) = 0;
		bool is_hidden() const { return _hidden == true; }
		void hide() { _hidden = true; }
		void show() { _hidden = false; }
	};
	class Sidebar: public Widget {
	protected:
		uint16_t _width;

	public:
		Sidebar(const uint16_t);
		void draw(ae::gui::GuiManager*) override;

		virtual void drawContents() = 0;
	};
}

namespace ae::gui {
	class GuiManager {
	protected:
		Engine* _engine;
		std::map<string, widgets::Widget*> _widgets;

		ImFont* _fontStandard;
		ImFont* _fontTitle;

	public:
		GuiManager(Engine*);

		void init();
		void addWidget(const string&, widgets::Widget*);
		void removeWidget(const string&);

		ImFont* fontTitle() { return _fontTitle; }
		ImTextureID loadTexture(const string&);
		bool processEvent();
		void renderFrame();
	};
}