#pragma once

namespace aos::namco
{
	class PacmanPort : public aos::io::Port
	{
	public:
		PacmanPort() : aos::io::Port(0xff) {}

		PacmanPort& joystick1(const bool active=true) {
			set(0, "_JOY1_UP", active);
			set(1, "_JOY1_LEFT", active);
			set(2, "_JOY1_RIGHT", active);
			set(3, "_JOY1_DOWN", active);
			return *this;
		}
		PacmanPort& joystick2(const bool active = true) {
			set(0, "_JOY2_UP", active);
			set(1, "_JOY2_LEFT", active);
			set(2, "_JOY2_RIGHT", active);
			set(3, "_JOY2_DOWN", active);
			return *this;
		}
		PacmanPort& fire2(const uint8_t bit = 4, const bool active = true) {
			set(bit, "_JOY2_FIRE", active);
			return *this;
		}
		PacmanPort& fire1(const uint8_t bit = 4, const bool active = true) {
			set(bit, "_JOY1_FIRE", active);
			return *this;
		}
		PacmanPort& coin(const uint8_t bit = 5, const bool active=true) {
			set(bit, "_COIN", active);
			return *this;
		}
		PacmanPort& starts(const bool active = true) {
			set(5, "_START1", active);
			set(6, "_START2", active);
			return *this;
		}
	};
}
