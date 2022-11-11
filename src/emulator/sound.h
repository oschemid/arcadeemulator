#pragma once
#include <SDL2/SDL.h>
#include "../include/types.h"


namespace ae {
	class Sound {
	public:
		uint8_t* buffer;
		uint32_t  length;
		SDL_AudioSpec audio;

	public:
		Sound(const string&);
	};

	class AudioDevice {
	protected:
		Sound* sound;
		uint8_t* buffer;
		uint32_t length;
		SDL_AudioDeviceID device;
		SDL_AudioSpec want;

		void callback(uint8_t*, int);

		static void forwardCallback(void* userdata, Uint8* stream, int len)
		{
			static_cast<AudioDevice*>(userdata)->callback(stream, len);
		}
	public:
		AudioDevice();

		void playSound(Sound*);
	};
}