#include "ui/arcadecontrollerwidget.h"


namespace aos::ui
{
	ArcadeControllerWidget::ArcadeControllerWidget()
		: ArcadeController{},
		Widget{}
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

		setJoystick1(joystick_control::left, ImGui::IsKeyDown(ImGuiKey_LeftArrow));
		setJoystick1(joystick_control::right, ImGui::IsKeyDown(ImGuiKey_RightArrow));
		setJoystick1(joystick_control::up, ImGui::IsKeyDown(ImGuiKey_UpArrow));
		setJoystick1(joystick_control::down, ImGui::IsKeyDown(ImGuiKey_DownArrow));
		setJoystick1(joystick_control::fire, ImGui::IsKeyDown(ImGuiKey_Space));

		setButton(button_control::start1, ImGui::IsKeyDown(ImGuiKey_1));
		setButton(button_control::start2, ImGui::IsKeyDown(ImGuiKey_2));

		setCoin(coin_control::coin1, ImGui::IsKeyDown(ImGuiKey_C));
	}
}