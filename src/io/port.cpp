#include "io.h"


using namespace aos::io;


Port::Port(const uint8_t v) :
	_port{ v } {}

void Port::set(uint8_t bit, const string& str, bool inverted)
{
	if (bit > 7)
		throw std::out_of_range("Port::set");
	_definition.push_back({ bit, str, inverted });
}

void Port::reset()
{
	_definition.clear();
}

void Port::init(const aos::emulator::GameConfiguration& game)
{
	const std::vector<aos::emulator::DipSwitch> dipswitch = game.switches;
	for (auto& [bit, str, inverted] : _definition)
	{
		try
		{
			for (auto& d : dipswitch) {
				if (d.name == str) {
					_port &= ~(1 << bit);
					uint8_t value = d.value << bit;
					_port |= (inverted) ? ~value : value;
				}
			}
		}
		catch (std::out_of_range)
		{
		}
	}
}

void Port::tick(const ae::controller::ArcadeController& controller)
{
	static std::map<string, std::function<bool(const ae::controller::ArcadeController&)>> _map =
	{
		{ "_CHEAT", [](const ae::controller::ArcadeController& c) { return c.cheat(); }},
		{ "_COIN", [](const ae::controller::ArcadeController& c) { return c.coin(); }},
		{ "_COIN2", [](const ae::controller::ArcadeController& c) { return c.coin2(); }},
		{ "_COIN3", [](const ae::controller::ArcadeController& c) { return c.coin3(); }},
		{ "_START1", [](const ae::controller::ArcadeController& c) { return c.button(ae::controller::ArcadeController::button_control::start1); }},
		{ "_START2", [](const ae::controller::ArcadeController& c) { return c.button(ae::controller::ArcadeController::button_control::start2); }},
		{ "_JOY1_FIRE", [](const ae::controller::ArcadeController& c) { return c.joystick1(ae::controller::ArcadeController::joystick_control::fire); }},
		{ "_JOY1_LEFT", [](const ae::controller::ArcadeController& c) { return c.joystick1(ae::controller::ArcadeController::joystick_control::left); }},
		{ "_JOY1_RIGHT", [](const ae::controller::ArcadeController& c) { return c.joystick1(ae::controller::ArcadeController::joystick_control::right); }},
		{ "_JOY1_DOWN", [](const ae::controller::ArcadeController& c) { return c.joystick1(ae::controller::ArcadeController::joystick_control::down); }},
		{ "_JOY1_UP", [](const ae::controller::ArcadeController& c) { return c.joystick1(ae::controller::ArcadeController::joystick_control::up); }},
		{ "_JOY2_LEFT", [](const ae::controller::ArcadeController& c) { return c.joystick2(ae::controller::ArcadeController::joystick_control::left); }},
		{ "_JOY2_RIGHT", [](const ae::controller::ArcadeController& c) { return c.joystick2(ae::controller::ArcadeController::joystick_control::right); }},
		{ "_JOY2_DOWN", [](const ae::controller::ArcadeController& c) { return c.joystick2(ae::controller::ArcadeController::joystick_control::down); }},
		{ "_JOY2_UP", [](const ae::controller::ArcadeController& c) { return c.joystick2(ae::controller::ArcadeController::joystick_control::up); }},
	};
	for (auto& [bit, str, inverted] : _definition)
	{
		if (_map.contains(str)) {
			if (_map[str](controller)^inverted)
				_port.set(bit);
			else
				_port.reset(bit);
		}
	}
}

uint8_t Port::get() const
{
	return static_cast<uint8_t>(_port.to_ulong());
}
