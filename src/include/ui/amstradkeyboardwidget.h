#pragma once
#include "ui.h"
#include "device/amstradkeyboard.h"


namespace aos::ui
{
	class AmstradKeyboardWidget : public ui::Widget, public device::AmstradKeyboard
	{
	public:
		AmstradKeyboardWidget();
		virtual void draw(ui::Manager*) override;
		virtual void tickui() override;
	};
}