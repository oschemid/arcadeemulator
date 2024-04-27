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

		virtual bool joystick1(const joystick_control) const { return false; }
		virtual bool button(const button_control) const { return false; }
		virtual bool coin(const coin_control) const { return false; }
		//bool coin() const { return _coin; }
		//bool coin2() const { return _coin2; }
		//bool coin3() const { return _coin3; }
		//bool cheat() const { return _cheat; }

	protected:
		ArcadeController() = default;
	};
}