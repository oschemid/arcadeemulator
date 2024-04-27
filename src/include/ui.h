#pragma once
#include "types.h"
#include "../ui/engine/engine.h"
#include "core.h"
#include <map>


namespace aos::ui
{
	// Initialisation of ui functionality
	// Must be called before using any ui methods
	void init();

	struct WindowInfo
	{
		string title;
		uint16_t width;
		uint16_t height;
	};
	class Manager;

	class Widget
	{
	public:
		typedef std::shared_ptr<Widget> Ptr;

	public:
		Widget() : _hidden(false) {}

		virtual void tickui() = 0;
		virtual void draw(aos::ui::Manager*) = 0;

		bool is_hidden() const { return _hidden == true; }
		void hide(const bool hidden) { _hidden = hidden; }

	protected:
		bool _hidden;
	};

	class Manager
	{
	public:
		Manager(WindowInfo);

		void init();
		void addWidget(const string&, Widget::Ptr);

		void hideWidget(const string&, const bool=true);
		std::map<string, Widget::Ptr>::iterator getWidget(const string&);
		void removeWidget(const string&);
		void toggleWidget(const string&);

		ImFont* fontTitle() { return _fontTitle; }
		ImTextureID loadTexture(const string&);
		ImTextureID createTexture(const uint16_t, const uint16_t);
		void refreshTexture(ImTextureID, const uint8_t*);
		void removeTexture(ImTextureID);
		bool processEvent();
		void renderFrame();

	protected:
		WindowInfo _window;

		ae::gui::Engine* _engine;
		std::map<string, Widget::Ptr> _widgets;
		std::map<string, ImTextureID> _textures;

		ImFont* _fontStandard;
		ImFont* _fontTitle;
	};
}