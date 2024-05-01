#pragma once
#include "device.h"


namespace aos::device
{
	class ArcadeController : public Device
	{
	public:
		using Ptr = std::shared_ptr<ArcadeController>;
		using joystick_control = enum { left = 0, right, up, down, fire };
		using button_control = enum { start1 = 0, start2 };
		using coin_control = enum { coin1 = 0 };
	public:
		static Ptr create();

		virtual ~ArcadeController() = default;

		virtual void init() override {}

		virtual bool joystick(const joystick_control) const { return false; }
		virtual bool button(const button_control) const { return false; }
		virtual bool coin(const coin_control) const { return false; }

	protected:
		ArcadeController() = default;
	};
}