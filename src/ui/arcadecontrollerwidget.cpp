#include "ui/arcadecontrollerwidget.h"


namespace aos::ui
{
	ArcadeControllerWidget::ArcadeControllerWidget(const bool fire, const bool secundary)
		: ArcadeController{},
		Widget{},
		_has_fire{fire},
		_secundary{secundary}
	{
	}

	void ArcadeControllerWidget::draw(aos::ui::Manager*)
	{
	}

	void ArcadeControllerWidget::tickui()
	{
		bool shift = ImGui::IsKeyDown(ImGuiKey_ModShift);
		bool ctrl = ImGui::IsKeyDown(ImGuiKey_ModCtrl);
		bool alt = ImGui::IsKeyDown(ImGuiKey_ModAlt);

		auto io = ImGui::GetIO();

		if (_secundary)
		{
			setJoystick(joystick_control::left, ImGui::IsKeyDown(ImGuiKey_Q));
			setJoystick(joystick_control::right, ImGui::IsKeyDown(ImGuiKey_D));
			setJoystick(joystick_control::up, ImGui::IsKeyDown(ImGuiKey_Z));
			setJoystick(joystick_control::down, ImGui::IsKeyDown(ImGuiKey_S));
			if (_has_fire)
				setJoystick(joystick_control::fire, ImGui::IsKeyDown(ImGuiKey_A));
		}
		else
		{
			setJoystick(joystick_control::left, ImGui::IsKeyDown(ImGuiKey_LeftArrow));
			setJoystick(joystick_control::right, ImGui::IsKeyDown(ImGuiKey_RightArrow));
			setJoystick(joystick_control::up, ImGui::IsKeyDown(ImGuiKey_UpArrow));
			setJoystick(joystick_control::down, ImGui::IsKeyDown(ImGuiKey_DownArrow));
			if (_has_fire)
				setJoystick(joystick_control::fire, ImGui::IsKeyDown(ImGuiKey_Space));

			setButton(button_control::start1, ImGui::IsKeyDown(ImGuiKey_1));
			setButton(button_control::start2, ImGui::IsKeyDown(ImGuiKey_2));

			setCoin(coin_control::coin1, ImGui::IsKeyDown(ImGuiKey_C));
		}
	}
}