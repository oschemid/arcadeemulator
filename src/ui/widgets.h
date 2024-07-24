#pragma once
#include "device.h"
#include "device/rasterdisplay.h"
#include "ui.h"
#include "database.h"


namespace aos {
	class MenuWidget : public aos::ui::Widget
	{
	public:
		MenuWidget(aos::database::Driver**);
		virtual void draw(aos::ui::Manager*) override;
		virtual void tickui() override {}

	protected:
		aos::database::Platform* _platformselected{ nullptr };
		aos::database::Driver* _driverselected{ nullptr };
		aos::database::Driver** _driverlaunched{ nullptr };

		void drawBackground(aos::ui::Manager*);
		void drawSidebar(aos::ui::Manager*);
		void drawGames(aos::ui::Manager*);
		void drawTile(aos::ui::Manager*, database::Driver&);
		void drawGame(aos::ui::Manager*);
	};
}
