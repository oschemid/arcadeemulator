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
	public:
		ae::memory memory;
		ae::cpu::i8080 cpu;

	public:
		spaceinvaders();

		bool init();
		void run();

	protected:
		void updateDisplay();
		SDL_Window* MainWindow;
		SDL_Renderer* MainRenderer;
		SDL_Texture* MainTexture;
		uint16_t Pixels[224 * 256];

	};
}