#pragma once
#include "ui.h"
#include "device/arcadecontroller.h"

#include <bitset>

namespace aos::ui
{
	class ArcadeControllerWidget : public ui::Widget, public device::ArcadeController
	{
	public:
		ArcadeControllerWidget(const bool, const bool);
		virtual void draw(ui::Manager*) override;
		virtual void tickui() override;

		virtual bool joystick(const joystick_control ctrl) const override { return _joystick[ctrl]; }
		virtual bool button(const button_control ctrl) const override { return _button[ctrl]; }
		virtual bool coin(const coin_control ctrl) const override { return _coin[ctrl]; }

	protected:
		bool _has_fire{ false };
		bool _secundary{ false };
		std::bitset<5> _joystick{ 0 };
		std::bitset<2> _button{ 0 };
		std::bitset<1> _coin{ 0 };

		void setJoystick(const joystick_control ctrl, const bool state) { if (state) _joystick.set(ctrl); else _joystick.reset(ctrl); }
		void setButton(const button_control ctrl, const bool state) { if (state) _button.set(ctrl); else _button.reset(ctrl); }
		void setCoin(const coin_control ctrl, const bool state) { if (state) _coin.set(ctrl); else _coin.reset(ctrl); }
	};
}