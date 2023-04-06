#include "arcadecontroller.h"
#include "SDL2/SDL.h"


using namespace ae::controller;


void ArcadeController::tick()
{
	const uint8_t* Keyboard = SDL_GetKeyboardState(NULL);

	if (Keyboard[SDL_SCANCODE_LEFT])
		_joystick1.set(joystick_control::left);
	else
		_joystick1.reset(joystick_control::left);
	if (Keyboard[SDL_SCANCODE_RIGHT])
		_joystick1.set(joystick_control::right);
	else
		_joystick1.reset(joystick_control::right);
	if (Keyboard[SDL_SCANCODE_UP])
		_joystick1.set(joystick_control::up);
	else
		_joystick1.reset(joystick_control::up);
	if (Keyboard[SDL_SCANCODE_DOWN])
		_joystick1.set(joystick_control::down);
	else
		_joystick1.reset(joystick_control::down);
	if (Keyboard[SDL_SCANCODE_SPACE])
		_joystick1.set(joystick_control::fire);
	else
		_joystick1.reset(joystick_control::fire);

	if (Keyboard[SDL_SCANCODE_Q])
		_joystick2.set(joystick_control::left);
	else
		_joystick2.reset(joystick_control::left);
	if (Keyboard[SDL_SCANCODE_E])
		_joystick2.set(joystick_control::right);
	else
		_joystick2.reset(joystick_control::right);
	if (Keyboard[SDL_SCANCODE_W])
		_joystick2.set(joystick_control::up);
	else
		_joystick2.reset(joystick_control::up);
	if (Keyboard[SDL_SCANCODE_S])
		_joystick2.set(joystick_control::down);
	else
		_joystick2.reset(joystick_control::down);
	if (Keyboard[SDL_SCANCODE_LCTRL])
		_joystick2.set(joystick_control::fire);
	else
		_joystick2.reset(joystick_control::fire);


	if (Keyboard[SDL_SCANCODE_1])
		_buttons.set(button_control::start1);
	else
		_buttons.reset(button_control::start1);
	if (Keyboard[SDL_SCANCODE_2])
		_buttons.set(button_control::start2);
	else
		_buttons.reset(button_control::start2);

	if (Keyboard[SDL_SCANCODE_C])
		_coin = true;
	else
		_coin = false;
	if (Keyboard[SDL_SCANCODE_V])
		_coin2 = true;
	else
		_coin2 = false;
}
