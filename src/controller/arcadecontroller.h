#pragma once
#include "types.h"
#include <bitset>


namespace ae::controller
{
	class ArcadeController
	{
	public:
		void tick();

		using joystick_control = enum { left = 0, right, up, down, fire };
		using button_control = enum { start1 = 0, start2 };

		bool joystick1(const joystick_control ctrl) const { return _joystick1[ctrl]; }
		bool joystick2(const joystick_control ctrl) const { return _joystick2[ctrl]; }
		bool button(const button_control ctrl) const { return _buttons[ctrl]; }
		bool coin() const { return _coin; }

		using Ptr = std::unique_ptr<ArcadeController>;

		static Ptr create() { return std::unique_ptr<ArcadeController>(new ArcadeController()); }

	protected:
		ArcadeController() = default;

		std::bitset<5> _joystick1{ 0 };
		std::bitset<5> _joystick2{ 0 };
		std::bitset<2> _buttons{ 0 };
		bool _coin{ false };
	};
}