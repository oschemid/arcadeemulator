#pragma once
#include <cstdint>
#include <string>
#include "memory.h"
#include "cpu/i8080.h"
#include "SDL2/SDL.h"


namespace ae
{
	class spaceinvaders
	{
	protected:
		uint8_t ships;

	public:
		ae::memory memory;
		ae::cpu::i8080 cpu;

	public:
		spaceinvaders();

		bool init();
		void run();

	protected:
		void updateDisplay();
		uint16_t Pixels[224 * 256];

	};
}